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

#ifndef DistriSpectral_Misc_h
#include "Misc.h"
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
    
    MPI_Group  orig_group, new_group;
    MPI_Comm   new_comm;
    MPI_Comm_group(MPI_COMM_WORLD, &orig_group);
    MPI_Comm_create(MPI_COMM_WORLD, orig_group, &new_comm);
    int new_rank;
    if(myid==0) {
        
        MPI_Group_rank (orig_group, &new_rank);
        //cout << endl << "MASTER: newRank:" <<new_rank;
        
        // Default settings
        int nDimension = 50;
        int nCluster = 10;
        int nDataPerCluster = 1000;
        double noise = 1; //variance
        double unitRadius =10;
        
        int enableFF = 0;
        int enableDistSvd = 1;
        
        string strDefault;
        cout << endl << "Use default settings? [y/n] (default y):";
        getline(cin, strDefault);
        
        // User want to input new Settings
        if (!strDefault.compare("n")) {
            cout << endl << "Dimension:";
            cin >> nDimension;
            cout << "Number of clusters:";
            cin >> nCluster;
            cout << "Number of data per cluster:";
            cin >> nDataPerCluster;
            cout << "Enable Fastfood? [1/0] (default " << enableFF <<"):";
            cin >> enableFF;
            cout << "Enable Distributed SVD? [1/0] (default " << enableDistSvd<< "):";
            cin >> enableDistSvd;
        }
        
        // It's master node
        Master master(numprocs, enableFF, enableDistSvd, new_comm);

        // The thread that sends task to slaves
        std::thread sender(&Master::sender, &master);
        
        // The thread thst receives result from slave
        // std::thread receiver(&Master::receiver, &master);
        
        DataSettings para(nDimension, nCluster, nDataPerCluster, pow(noise,0.5), unitRadius);
        
        int nRepeat = 1;
        
        vector<int64> timeHistory;
        
        for (int repInd =0; repInd<nRepeat; repInd++) {
        
            master.reset();
            DataGenerator data(para);
        
            // Main Algorithm
            Logic logic(master);
            
            int64 t0 = GetTimeMs64();
            logic.start(data.X(), nCluster, noise);
            int64 t1 = GetTimeMs64();
            
            cout << "[MAIN] : Total time=" << (t1-t0) << endl;
            timeHistory.push_back(t1-t0);
            
            if(DBG) cout << endl << "RESULT " << endl << logic.centers();
            data.evaluate(logic.centers());
            
        }
      
        master.terminate();
         cout << endl << "BEFORE JOIN" << endl;
        sender.join();
        cout << endl << "AFTER JOIN" << endl;
        //receiver.join();

        float average=0;
        for (int i=0; i<timeHistory.size(); i++) {
            cout << endl << "Trial " << i << "\tTime:\t" << timeHistory.at(i);
            average += timeHistory.at(i);
        }
        
        cout << endl << "Average :\t" << average/timeHistory.size() << endl;
    
    }
    else {
        
        MPI_Group_rank (orig_group, &new_rank);
        //cout << endl << "SLAVE: id:"<< myid <<" newRank:" <<new_rank;
        
        // It's slave node
        Slave slave(myid, numprocs, new_comm);
        slave.run();
    }
  
    /* Terminate MPI execution environment */
    MPI_Finalize();
    
    return 0;
}
