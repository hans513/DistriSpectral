//
//  main.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

//#include "main.h"

/*Do not forget to include the MPI header file*/
#include "mpi.h"
#include "Master.h"
#include "Slave.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <Eigen/Dense>

using namespace Eigen;
using namespace std;

//#define BUF_SIZE 10*10
#define AROW 1000
#define ACOL 1000
#define BROW 1000
#define BCOL 1000
#define BUF_SIZE AROW*BCOL

static const int DBG=0;

MatrixXd distri_mul(MatrixXd X1, MatrixXd X2, int numprocs) {
    
    MPI_Status Stat;
    
    if (X1.cols()!=X2.rows()) {
        cout << "Error: distri_mul dimension mismatch";
        return MatrixXd::Identity(1,1);
    }
    
    MatrixXd ret(X1.rows(), X2.cols());
    
    int blk = X1.cols() / numprocs;
    int *index = new int[numprocs+1];
    for (int i=0; i<numprocs; i++) {
        index[i] = i*blk;
    }
    index[numprocs] = X1.cols();
    
    for(int id=1; id<numprocs; id++) {
        
        cout << endl <<"Master: Start sending matrix to node:" << id << endl;
        
        MatrixXd temp1 = X1.middleCols(index[id], index[id+1]-index[id]);
        int *size1 = new int[2];
        size1[0] = X1.rows();
        size1[1] = index[id+1]-index[id];
        MPI_Send(size1, 2, MPI_INT, id, 1, MPI_COMM_WORLD);
        MPI_Send(temp1.data(), temp1.size(), MPI_DOUBLE, id, 1, MPI_COMM_WORLD);
        
        MatrixXd temp2 = X2.middleRows(index[id], index[id+1]-index[id]);
        int *size2 = new int[2];
        size2[0] = index[id+1]-index[id];
        size2[1] = X2.cols();
        MPI_Send(size2, 2, MPI_INT, id, 1, MPI_COMM_WORLD);
        MPI_Send(temp2.data(), temp2.size(), MPI_DOUBLE, id, 1, MPI_COMM_WORLD);
        
        cout << endl <<"Master: Finish sending matrix to node:" << id << endl;
    }
    
    ret = X1.leftCols(index[1]) * X2.topRows(index[1]);
    
    //double buffer[ARRAY_SIZE];
    vector<double> buffer(BUF_SIZE);
    
    cout << endl <<endl<<"=== Master: Start receiving result ===" << endl<<endl <<endl;
    for (int id=1; id<numprocs; id++) {
        
        //cout << endl<<"Master: Receiving result from node:" << id << endl;
        MPI_Recv(&buffer[0], BUF_SIZE, MPI_DOUBLE, id, 2, MPI_COMM_WORLD, &Stat);
        cout << endl<<"Master: Result Received from node:"<< id << endl;
        MatrixXd matrix = Map<MatrixXd>(&buffer[0], X1.rows(), X2.cols());
        ret += matrix;
    }
    
    return ret;
}

int main( int argc, char *argv[] )
{
    int myid, numprocs;
    int istart,iend,sum=0,psum=0,i;
    vector<double> buffer(BUF_SIZE);
    
    MPI_Status Stat;
    
    
    /* Initialize the MPI execution environment */
    MPI_Init(&argc,&argv);
    
    /* Get the number of processes associated with communicator*/
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    
    /*Determines the rank of the calling process in the communicator */
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    
    MatrixXd C;
    /* Send the data to only one in cluster*/
    if(myid==0) {
        
        Master* master = new Master();
        master->run();
        /*

        cout << endl<<"Master: numprocs:" << numprocs << endl;

        MatrixXd A = MatrixXd::Random(AROW,ACOL);
        MatrixXd B = MatrixXd::Random(BROW,BCOL);
        
        cout << endl<<"Master: Calculating the correct answer" << endl;
        MatrixXd correct = A*B;

        cout << endl <<"Master: Start Distributed calculation" << endl;
        MatrixXd result = distri_mul(A,B, numprocs);
        
    	MatrixXd error = correct - result;
        cout << endl << "Error =" << error.sum() << endl;
        cout << "VECTOR VERSION"; */
    }
    
    /*In process1: block until you get data from root process*/
    
    else {
        Slave* slave = new Slave();
        slave->run();
        /*
        int size[3];
        cout << "Remote >> myid:" << myid << " Ready" << endl;
        MPI_Recv(size, 3, MPI_INT, 0, 1, MPI_COMM_WORLD, &Stat);
        //cout << "myid:" << myid << " check2" << endl;
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
    
    /* Terminate MPI execution environment */
    MPI_Finalize();
    return 0;
}
