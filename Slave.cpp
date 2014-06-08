//
//  Slave.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#include "Slave.h"


void Slave::run() {
    cout << "Remote >> mId:" << mId << " Ready" << endl;
    
    //vector<double> buffer(BUF_SIZE);
    char taskBuf[sizeof(Task)];
    MPI_Status status;
    
    
    while (true) {

        if (DBG) {
            cout << "Remote >> mId:" << mId << " Wait for next task" <<endl;
        }
        
        MPI_Recv(taskBuf, sizeof(Task), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
        Task* task = (Task*) taskBuf;
    
        
        switch (task->cmd()) {
            
            {case Task::TERMINATE:
                cout << "Remote >> mId:" << mId << " TERMINATE!"<<endl;
                exit (EXIT_SUCCESS);
                break;
            }
                
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
                
                break;
            }
            
            {case Task::TEST:
                break;
            }
                
            {default:
                break;
            }
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