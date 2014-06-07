//
//  Slave.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#include "Slave.h"


void Slave::run() {
    cout << "Remote >> myid:" << mId << " Ready" << endl;
    
    //vector<double> buffer(BUF_SIZE);
    
    //while (true) {
        
        char* taskBuf = new char[sizeof(Task)];
        MPI_Status Stat;
        
        MPI_Recv(taskBuf, sizeof(Task), MPI_CHAR, 0, 1, MPI_COMM_WORLD, &Stat);
        
        Task* task = (Task*) taskBuf;
    
        cout << endl <<"cmd:"<<task->cmd() <<"  size1:" << task->size()[1] << endl;
    
       
        
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
        
    //}
}