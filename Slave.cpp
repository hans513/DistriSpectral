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

        if (DBG) {
            cout << "Remote >> mId:" << mId << " Wait for next task" <<endl;
        }
        
        // Receive remote task here
        MPI_Recv(taskBuf, sizeof(Task), MPI_CHAR, MASTER_ID, 0, MPI_COMM_WORLD, &status);
        Task* task = (Task*) taskBuf;
    
        
        if (DBG) {
            cout << "Remote >> mId:" << mId << " Got task:" <<task->cmd()<< endl;
        }
        
        // Decide what task to do
        switch (task->cmd()) {
            
            {case Task::TERMINATE:
                cout << "Remote >> mId:" << mId << " TERMINATE!"<<endl;
                exit=1;
                break;
            }
                
                
            // Send the data matrix chunk
            {case Task::INITIAL:
                
                int dataSize;
                MPI_Probe(MASTER_ID, 1, MPI_COMM_WORLD, &status);
                MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
                vector<double> buffer(dataSize);
                MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, MASTER_ID, 1, MPI_COMM_WORLD, &status);
                MatrixXd matrix = Map<MatrixXd>(&buffer[0], task->size()[0], task->size()[1]);
                
                if (DBG) {
                    cout << "Remote >> mId:" << mId << " Initial got:" << matrix <<endl;
                }
                
                initialWork(matrix, task->info());
                
                break;
            }
            
            {case Task::TEST:
                break;
            }
                
            {default:
                break;
            }
        }
    
        if (DBG) {
            cout << "Remote >> mId:" << mId << " Finish task" <<endl;
        }
       
        
        /*
         int size[3];
        
        
         MPI_Recv(size, 3, MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);
         MPI_Recv(&buffer[0], BUF_SIZE, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &Stat);
         cout << "Remote >> myid:" << myid << " Matrix received size:" << size[0]<< " , " << size[1] << endl;
         MatrixXd matrix1 = Map<MatrixXd>(&buffer[0], size[0], size[1]);
         
         if(DBG) cout << endl <<"id: " << myid << endl << "receiveA" << endl << matrix1 << endl;
         
         MPI_Recv(size, 3, MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);
         MPI_Recv(&buffer[0], BUF_SIZE, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &Stat);
         MatrixXd matrix2 = Map<MatrixXd>(&buffer[0], size[0], size[1]);
         
         C = matrix1 * matrix2;
         
         cout << "Remote >> myid:" << myid << " Computation complete. Sending back result" << endl;
         MPI_Send(C.data(), C.size(), MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
        */
        
    }
}

void Slave::initialWork(MatrixXd input, int target) {
    
    srand (time(NULL));
    random_device rd;
    default_random_engine generator(rd());
    normal_distribution<double> normal_distri(0, 1);
    
    MatrixXd mGausssian(input.rows(), target);
    
    for (int i=0; i<mGausssian.rows(); i++) {
        for(int j=0; j<mGausssian.cols(); j++) {
            mGausssian(i, j) = normal_distri(generator);
        }
    }
    
}