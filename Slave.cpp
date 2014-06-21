//
//  Slave.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#include "Slave.h"

using namespace std;
using namespace Eigen;

void Slave::run() {

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
                cout << endl << "Remote >> mId:" << mId << " Task::INITIAL" <<endl<< "  [" <<task->id()<<"]";
                int dataSize;
                MPI_Probe(MASTER_ID, 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
                
                vector<double> buffer(dataSize);
                MPI_Request request;
                
                cout << endl << "Remote >> mId:" << mId << " waiting for data "<< "  [" <<task->id()<<"]" << endl;
                MPI_Irecv(&buffer[0], dataSize, MPI_DOUBLE, MASTER_ID, 1, MPI_COMM_WORLD, &request);
                
                cout << endl << "Remote >> mId:" << mId << " data received"<< "  [" <<task->id()<<"]";
                
                MatrixXd matrix = Map<MatrixXd>(&buffer[0], task->size()[0], task->size()[1]);
                initialWork(matrix, task->info());
                break;
            }
                
            {case Task::BASIS_MUL:
                cout << endl <<"Remote >> mId:" << mId << " Task::BASIS_MUL" << "  [" <<task->id()<<"]"<<endl;

                long dataSize = task->size()[0] * task->size()[1];
                vector<double> buffer(dataSize);
                
                //MPI_Barrier(MPI_COMM_WORLD);
                MPI_Bcast(&buffer[0], dataSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MatrixXd matrix = Map<MatrixXd>(&buffer[0], task->size()[0], task->size()[1]);
                basisMul(matrix);
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