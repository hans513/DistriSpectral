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

#include <stdio.h>
#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include <thread>

#ifndef __DistriSpectral__Logic__
#include "Logic.h"
#endif /* defined(__DistriSpectral__Logic__) */


#ifndef __DistriSpectral__Slave__
#include "Slave.h"
#endif /* defined(__DistriSpectral__Slave__) */

#ifndef __DistriSpectral__Master__
#include "Master.h"
#endif /* defined(__DistriSpectral__Master__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif

using namespace Eigen;
using namespace std;

static const int DBG=0;

int main( int argc, char *argv[] ) {
    int myid, numprocs;

    /* Initialize the MPI execution environment */
    MPI_Init(&argc,&argv);
    
    /* Get the number of processes associated with communicator*/
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    
    /*Determines the rank of the calling process in the communicator */
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    
    if(myid==0) {
    
        // It's master node
        Master master(numprocs);

        // The thread that sends task to slaves
        std::thread sender(&Master::sender, &master);
        
        // The thread thst receives result from slave
        std::thread receiver(&Master::receiver, &master);
        
        int nDimension = 50;
        int nCluster = 10;
        int nDataPerCluster = 1000;
        double noise = 1; //variance
        double unitRadius =10;
        
        DataSettings para(nDimension, nCluster, nDataPerCluster, pow(noise,0.5), unitRadius);
        DataGenerator data(para);
        
        // Main Algorithm
        Logic logic(master);
        logic.start(data.X(), nCluster, noise);
        
        sender.join();
        receiver.join();
        
        cout << endl << "RESULT " << endl << logic.centers();
        
        data.evaluate(logic.centers());

    }
    else {
        // It's slave node
        Slave slave(myid);
        slave.run();
    }
  
    /* Terminate MPI execution environment */
    MPI_Finalize();
    
    return 0;
}
