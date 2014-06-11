//
//  Master.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014 Tse-Han Huang. All rights reserved.
//

#include "Master.h"


using namespace std;
using namespace Eigen;

void Master::run() {
    cout << "Master on duty" << endl;

    //thread sender(&Master::sender, this);
    //thread receiver(&Master::receiver, this);

}

void Master::sender() {
    
    while (!mExit) {
        
         cout << endl <<"Master: pop next task";
        
        // May Block here: get next task to assign
        TaskParcel current = mTaskQueue.pop();
        // May Block here: get a available slave to assign
        int slave = mAvailSlave.pop();
        
        if (DBG) cout << endl <<"Master: send task " << current.task().cmd() << " to slave " << slave << endl;
        
        // register callback function when the task return
        mCallbackVec.at(slave) = current.callback();
        
        cout << endl <<"Master: sender mCallbackVec size:" << mCallbackVec.size() << endl;
        
        Task ttt = current.task();
        MPI_Send(&ttt, sizeof(Task), MPI_CHAR, slave, 0, MPI_COMM_WORLD);
        
         cout << endl <<"Master: after send task to slave " << slave << endl;
        if (current.data()==NULL) continue;
         cout << endl <<"Master: send data to slave " << slave << endl;
        MPI_Send(current.data(), current.dataSize(), MPI_DOUBLE, slave, 1, MPI_COMM_WORLD);
    }
    
}

void Master::receiver() {
    
    int dataSize;
    MPI_Status status;
    
    while (!mExit) {
        
        cout << endl <<"Master: receiver prepare to receive next msg size:" << endl;
        
        // May Block here
        MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
        vector<double> buffer(dataSize);
        MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        
        if (DBG) cout << endl <<"Master: receive from to slave " << status.MPI_SOURCE << endl;
        
        cout << endl <<"Master: receiver mCallbackVec size:" << mCallbackVec.size() << endl;

        
        //MatrixXd matrix = Map<MatrixXd>(&buffer[0], task->size()[0], task->size()[1]);
    
        cout << endl <<"Master: !!!!"<< mCallbackVec.at(status.MPI_SOURCE).name();
        
        // Callback function knows how to handle data
        mCallbackVec.at(status.MPI_SOURCE).notify(&buffer[0]);
        
        cout << endl <<"Master: Check"<< endl;
        cout << endl <<"Master: Check"<< endl;cout << endl <<"Master: Check"<< endl;
        cout << endl <<"Master: Check"<< endl;cout << endl <<"Master: Check"<< endl;
        
        
//      mCallbackVec.at(status.MPI_SOURCE) ;

        // Put the slave back to available pool
        mAvailSlave.push(status.MPI_SOURCE);
        
        cout << endl <<"Master: Check1"<< endl;
        cout << endl <<"Master: Check1"<< endl;cout << endl <<"Master: Check1"<< endl;
        cout << endl <<"Master: Check1"<< endl;cout << endl <<"Master: Check1"<< endl;
    }
    
}

void Master::submit(TaskParcel parcel) {
    mTaskQueue.push(parcel);
}


void Master::terminate() {
    Task task(Task::TERMINATE);
    for (int id=1; id<mNumProc; id++) {
        MPI_Send(&task, sizeof(task), MPI_CHAR, id, 0, MPI_COMM_WORLD);
    }
}

/*
// Split data or generate data info here
void Master::initialize() {
    
    int nDimension = 4;
    int nGaussian = 2;
    int nDataPerGaussian = 10;
    double noise = 1; //variance
    double unitRadius =10;
    
    int nChunk = 4;
    
    //For a large data input we want to split it here;
    data = new DataGenerator(nDimension, nGaussian, nDataPerGaussian, pow(noise,0.5), unitRadius);
    int blk = data->X().cols() / nChunk;
    for (int i=0; i<nChunk-1; i++) {
        mChunkVec.push_back(ChunkInfo(i*blk, (i+1)*blk));
    }
    mChunkVec.push_back(ChunkInfo((nChunk-1)*blk, data->X().cols()+1));
    
    

    
    //mDataset = data.X
    
}

void Master::test() {
    
    for (int i=0; i<mChunkVec.size(); i++) {
        cout << "start:" << mChunkVec.at(i).start()<< "  end:" << mChunkVec.at(i).end() << endl;
    }
    
}

void Master::test_initial() {
    
    MatrixXd a(3,3);
    a << 1,2,3,4,5,6,7,8,9;
    
    int size[]={3,3};
    
    Task task(Task::INITIAL, size);
    
    MPI_Send(&task, sizeof(task), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    MPI_Send(a.data(), a.size(), MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
    
}
 */

