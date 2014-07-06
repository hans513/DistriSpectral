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
    
    // Buffer for receiving the remote task
    char taskBuf[sizeof(Task)];

    MPI_Status status;
    int exit = 0;

    while (!exit) {

        cout << endl << "Remote >> mId:" << mId << " Wait for next task";
        cout << endl;
        
        // Receive remote task here
        MPI_Recv(taskBuf, sizeof(Task), MPI_BYTE, MASTER_ID, 0, MPI_COMM_WORLD, &status);
        Task* task = (Task*) taskBuf;
        
        cout << endl << "Remote >> mId:" << mId << " Got task:" <<task->cmd()<< "  [" <<task->id()<<"]";

        
        // Decide what task to do
        switch (task->cmd()) {
            
            {case Task::TERMINATE:
                cout << endl << "Remote >> mId:" << mId << " TERMINATE!"<< endl;
                exit=1;
                break;
            }
             
            // Receiving the data matrix chunk
            {case Task::INITIAL:
                cout << endl << "Remote >> mId:" << mId << " Task::INITIAL"<< "  [" <<task->id()<<"]" << endl;
                MatrixXd matrix = receiveMatrixFrom(MASTER_ID, Task::DATA_TAG, task->size());
                initialWork(matrix, task->info());
                break;
            }
            
            // Receving the broadcast
            {case Task::BASIS_MUL:
             case Task::CAL_TENSOR:
                cout << endl <<"Remote >> mId:" << mId << " Task::"<<  Task::cmdToString(task->cmd()) <<"  dataSize:"<< task->size()[0] << " * " << task->size()[1] << "  [" <<task->id()<<"]"<<endl;
                long dataSize = task->size()[0] * task->size()[1];
                vector<double> buffer(dataSize);
                
                MPI_Bcast(&buffer[0], dataSize, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MatrixXd matrix = Map<MatrixXd>(&buffer[0], task->size()[0], task->size()[1]);
                
                if      (task->cmd() == Task::BASIS_MUL) basisMul(matrix);
                else if (task->cmd() == Task::CAL_TENSOR) calTensor(matrix);

                break;
            }
            
            {case Task::RESET:
                resetDataCache();
                // For reset task, the size contains settings.
                mWithFastfood = task->size()[0];
                mWithDistSvd = task->size()[1];
                static int first = 0;
                if (!first++) {
                    char name[MAXHOSTNAMELEN];
                    size_t namelen = MAXHOSTNAMELEN;
                    pid_t pid = getpid();
                    
                    if (gethostname(name, namelen) != -1) {
                        cout << endl << "Remote >> mId:" << mId << " Map to machine:" << name << "  Pid=" << pid;
                    }
                }
                
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
    
    MatrixXd result;
    // Fastfood random projection
    if (mWithFastfood) {
        Fastfood ff(input.cols(), target);
        result = ff.multiply(input);
    }
    // General random Gaussian projection
    else {
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
        result = input * gausssian;
    }

    
    if (mWithDistSvd) {
        result = edoSketching(result.transpose(), target);
        result = result.transpose();
    }
    
    cout << endl << "Remote >> mId:" << mId << " InitialWork Sending result back";
    if (DBG) cout << endl << "RESULT:" << endl << result << endl;
    MPI_Send(result.data(), result.size(), MPI_DOUBLE, MASTER_ID, Task::RETURN_TAG, mComm);

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
    Global_sum(result, mId, mTotalProc, mComm);
    cout << endl << "Remote >> mId:" << mId << " Finish Sending result back";
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
    Global_sum(result, mId, mTotalProc, MPI_COMM_WORLD);
    cout << endl << "Remote >> mId:" << mId << " Finish Sending result back";
}

int isPowerOfTwo (unsigned int x)
{
    return ((x != 0) && ((x & (~x + 1)) == x));
}

MatrixXd Slave::receiveMatrixFrom(int sender, int tag, long size[2]) {
    
    MPI_Status status;
    int dataSize;
          cout << endl << "Remote >> mId:" << mId << " about to probe" << endl;
    MPI_Probe(sender, tag, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
    
    //cout << endl << "Remote >> mId:" << my_rank << " 1st round Global_sum:"<<dataSize << endl;
    
    vector<double> buffer(dataSize);
    //cout << endl << "Remote >> mId:" << my_rank << " 1st round Global_sum Waiting "<<partner << endl;
    
    //MPI_Irecv(&buffer[0], dataSize, MPI_DOUBLE, MASTER_ID, 1, MPI_COMM_WORLD, &request);
        cout << endl << "Remote >> mId:" << mId << " prepare to receive    dataSize:" <<dataSize<< endl;
    MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, sender, tag, MPI_COMM_WORLD, &status);
    cout << endl << "Remote >> mId:" << mId << " data received" << endl;
    
    MatrixXd matrix = Map<MatrixXd>(&buffer[0], size[0], size[1]);
    
    return matrix;
}

/*-----------------------------------------------------------------*/
/* Function:    Global_sum
 * Purpose:     Compute the global sum of ints stored on the processes
 *
 * Input args:  my_contrib = process's contribution to the global sum
 *              my_rank = process's rank
 *              p = number of processes
 *              comm = communicator
 * Return val:  Sum of each process's my_contrib:  valid only
 *              on process 0.
 *
 * Notes:
 *    1.  Uses tree structured communication.
 *    2.  p, the number of processes must be a power of 2.
 *    3.  The return value is valid only on process 0.
 *    4.  The pairing of the processes is done using bitwise
 *        exclusive or.  Here's a table showing the rule for
 *        for bitwise exclusive or
 *           X Y X^Y
 *           0 0  0
 *           0 1  1
 *           1 0  1
 *           1 1  0
 *        Here's a table showing the process pairing with 8
 *        processes (r = my_rank, other column heads are bitmask)
 *           r     001 010 100
 *           -     --- --- ---
 *           0 000 001 010 100
 *           1 001 000  x   x
 *           2 010 011 000  x
 *           3 011 010  x   x
 *           4 100 101 110 000
 *           5 101 100  x   x
 *           6 110 111 100  x
 *           7 111 110  x   x
 */
MatrixXd Slave::Global_sum(MatrixXd myData, int my_rank, int nProc, MPI_Comm comm) {
    
    cout << endl << "Remote >> mId:" << my_rank << " Global_sum!!"<< endl;
    
    MatrixXd   sum = myData;
    int        partner;
    int        done = 0;
    unsigned   bitmask = 1;
    
    if (!isPowerOfTwo(nProc)) {
        int originProc = nProc;
        nProc = pow2roundup(nProc) / 2;
        
        // receiver
        if (my_rank < nProc && my_rank < (originProc-nProc)) {
            partner = my_rank + nProc;
            long size[2] = {sum.rows(), sum.cols()};
            MatrixXd matrix = receiveMatrixFrom(partner, Task::TREESUM_TAG, size);
            sum += matrix;
        }
        // sender
        else if (my_rank >= nProc) {
            partner = my_rank - nProc;
            
            MPI_Comm comm = MPI_COMM_WORLD;
            if (partner == MASTER_ID) comm = mComm;
            
            cout << endl << "Remote >> mId:" << my_rank << "1st Global_sum Sending "<<partner << endl;
            MPI_Send(sum.data(), sum.size(), MPI_DOUBLE, partner, Task::TREESUM_TAG, comm);
            done = 1;
        }
        
    }
    
    while (!done && bitmask < nProc) {
        partner = my_rank ^ bitmask;
        
         cout << endl << "Remote >> mId:" << my_rank << " Global_sum PARTNER is="<< partner << endl;
        
        if (my_rank < partner) {
            long size[2] = {sum.rows(), sum.cols()};
            MatrixXd matrix = receiveMatrixFrom(partner, Task::TREESUM_TAG, size);
            sum += matrix;
            bitmask <<= 1;
        } else {
            
            MPI_Comm comm = MPI_COMM_WORLD;
            if (partner == MASTER_ID) comm = mComm;
                
            
            
            cout << endl << "Remote >> mId:" << my_rank << " Global_sum Sending "<<partner << endl;
            MPI_Send(sum.data(), sum.size(), MPI_DOUBLE, partner, Task::TREESUM_TAG, comm);
            done = 1;
        }
    }
    
    /* Valid only on 0 */
        cout << endl << "Remote >> mId:" << my_rank << " end Global_sum!!"<< endl;
    return sum;
}  /* Global_sum */