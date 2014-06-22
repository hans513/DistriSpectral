//
//  Slave.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014 Tse-Han Huang. All rights reserved.
//

#include "Slave.h"

using namespace std;
using namespace Eigen;

void Slave::run() {
    
    char name[MAXHOSTNAMELEN];
    size_t namelen = MAXHOSTNAMELEN;
    
    if (gethostname(name, namelen) != -1) {
        cout << endl << "Remote >> mId:" << mId << " Map to machine:" << name;
    }
    

    // Buffer for receiving the remote task
    char taskBuf[sizeof(Task)];

    MPI_Status status;
    int exit = 0;

    while (!exit) {

        cout << endl << "Remote >> mId:" << mId << " Wait for next task";
        cout << endl;
        
        // Receive remote task here
        MPI_Recv(taskBuf, sizeof(Task), MPI_CHAR, MASTER_ID, 0, MPI_COMM_WORLD, &status);
        Task* task = (Task*) taskBuf;
        
        cout << endl << "Remote >> mId:" << mId << " Got task:" <<task->cmd()<< "  [" <<task->id()<<"]";

        
        // Decide what task to do
        switch (task->cmd()) {
            
            {case Task::TERMINATE:
                cout << endl << "Remote >> mId:" << mId << " TERMINATE!"<< endl;
                exit=1;
                break;
            }
             
            // Receive the data matrix chunk
            {case Task::INITIAL:
                cout << endl << "Remote >> mId:" << mId << " Task::INITIAL"<< "  [" <<task->id()<<"]" << endl;
                int dataSize;
                MPI_Probe(MASTER_ID, 1, MPI_COMM_WORLD, &status);
                
                cout << endl << "Remote >> mId:" << mId << " Task::INITIAL Probe"<< "  [" <<task->id()<<"]" << endl;
                
                MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
                
                cout << endl << "Remote >> mId:" << mId << " Task::INITIAL GetCount:"<<dataSize << " going to allocate vector  [" <<task->id()<<"]" << endl;
                
                vector<double> buffer(dataSize);
                cout << endl << "Remote >> mId:" << mId << " Task::INITIAL After bufer vector allocation dataSize "<<dataSize << "  [" <<task->id()<<"]" << endl;
//                MPI_Request request;
                
                cout << endl << "Remote >> mId:" << mId << " waiting for data "<< "  [" <<task->id()<<"]" << endl;
                //MPI_Irecv(&buffer[0], dataSize, MPI_DOUBLE, MASTER_ID, 1, MPI_COMM_WORLD, &request);
                MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, MASTER_ID, 1, MPI_COMM_WORLD, &status);
                cout << endl << "Remote >> mId:" << mId << " data received"<< "  [" <<task->id()<<"]";
                
                MatrixXd matrix = Map<MatrixXd>(&buffer[0], task->size()[0], task->size()[1]);
                initialWork(matrix, task->info());
                break;
            }
                
            {case Task::BASIS_MUL:
             case Task::CAL_TENSOR:
                cout << endl <<"Remote >> mId:" << mId << " Task::"<<  Task::cmdToString(task->cmd()) <<"  dataSize:"<< task->size()[0] << " * " << task->size()[1] << "  [" <<task->id()<<"]"<<endl;
                long dataSize = task->size()[0] * task->size()[1];
                vector<double> buffer(dataSize);
                
                //MPI_Barrier(MPI_COMM_WORLD);
                MPI_Bcast(&buffer[0], dataSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MatrixXd matrix = Map<MatrixXd>(&buffer[0], task->size()[0], task->size()[1]);
                
                if      (task->cmd() == Task::BASIS_MUL) basisMul(matrix);
                else if (task->cmd() == Task::CAL_TENSOR) calTensor(matrix);

                break;
            }
            
            {case Task::RESET:
                resetDataCache();
                break;
            }
                
            {default:
                break;
            }
        }
    
        cout << endl << "Remote >> mId:" << mId << " Finish task" << "  [" <<task->id()<<"]"<<endl;

    }
}

/**
 State 1: Random projection
 */
void Slave::initialWork(MatrixXd input, int target) {
    
    if (DBG) cout << endl <<"Remote >> mId:" << mId << " Task::initialWork" <<endl;
    
    dataVec.push_back(input);
    
    // Fastfood random projection
    Fastfood ff(input.cols(), target);
    MatrixXd result = ff.multiply(input);

    // General random Gaussian projection
    /*
    srand (time(NULL));
    random_device rd;
    default_random_engine generator(rd());
    normal_distribution<double> normal_distri(0, 1);
    
    MatrixXd gausssian(input.cols(), target);
    
    for (int i=0; i<gausssian.rows(); i++) {
        for(int j=0; j<gausssian.cols(); j++) {
            gausssian(i, j) = normal_distri(generator);
        }
    }
    
    MatrixXd result = input * gausssian;
    */

    
    cout << endl << "Remote >> mId:" << mId << " InitialWork Sending result back";
    if (DBG) cout << endl << "RESULT:" << endl << result << endl;
    MPI_Send(result.data(), result.size(), MPI_DOUBLE, MASTER_ID, Task::RETURN_TAG, MPI_COMM_WORLD);
}


/**
 State 3: Basis multiplication
 */
void Slave::basisMul(Eigen::MatrixXd basis) {
    
    if (DBG) cout << endl <<"Remote >> mId:" << mId << " Task::basisMul" <<endl;
    
    MatrixXd result = MatrixXd::Zero(basis.cols(),basis.cols());

    for (int i=0; i<dataVec.size(); i++) {
        MatrixXd temp = basis.transpose()*dataVec.at(i);
        result += temp * temp.transpose();
    }

    cout << endl << "Remote >> mId:" << mId << " basisMul Sending result back";
    if (DBG) cout << endl << "RESULT:" << endl << result << endl;
    MPI_Send(result.data(), result.size(), MPI_DOUBLE, MASTER_ID, Task::RETURN_TAG, MPI_COMM_WORLD);
}

/**
 State 5: Calculate tensor
 */

void Slave::calTensor(Eigen:: MatrixXd whiten) {
    
    int k = whiten.cols();
    
    if (DBG) cout << endl <<"Remote >> mId:" << mId << " Task::calTensor" <<endl;

    MatrixXd ewx = MatrixXd::Zero(k, 1);
    D3Matrix<MatrixXd> wxTensor(k, k, k);
    
    // For all the matrices in the cache
    for (int i=0; i<dataVec.size(); i++) {
        MatrixXd currentLayer = dataVec.at(i);
        
        for (int j=0; j<currentLayer.cols(); j++) {
            MatrixXd wx = whiten.transpose() * currentLayer.col(j);
            
            // Calculate E[W'X]
            ewx += wx;

            // Calculate E[W'X (x)^3]
            MatrixXd temp = outer(wx,wx).getLayer(0);
            wxTensor += outer(temp, wx);
        }
    }

    // Flatten the result
    MatrixXd result(k, k*k+1);
    result.col(0) = ewx;
    for (int layer=0; layer<k; layer++) {
        result.middleCols(layer*k+1, k) = wxTensor.getLayer(layer);
    }

    cout << endl << "Remote >> mId:" << mId << " calTensor Sending result back";
    MPI_Send(result.data(), result.size(), MPI_DOUBLE, MASTER_ID, Task::RETURN_TAG, MPI_COMM_WORLD);
}