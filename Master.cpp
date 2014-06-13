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
}

void Master::sender() {
    
    while (!mExit) {
        
        cout << endl <<"Master sender ==>>: pop next task";
        
        // May Block here: get next task to assign
        TaskParcel current = mTaskQueue.pop();
        Task task = current.task();
        
        if (task.cmd()==Task::TERMINATE) break;
        
        
        // I should make it more general here
        
        
        switch (task.cmd()) {

            {case Task::INITIAL:
                // May Block here: get a available slave to assign
                int slave = mAvailSlave.pop();
                
                if (DBG) cout << endl <<"Master sender ==>>: send task " << current.task().cmd() << " to slave " << slave;
                
                // register callback function when the task return
                mCallbackVec.at(slave) = current.callback();
                //Callback* cb = mCallbackVec.at(slave);
                
                
                MPI_Send(&task, sizeof(Task), MPI_CHAR, slave, 0, MPI_COMM_WORLD);
                
                if (current.data()==NULL) continue;
                cout << endl <<"Master sender ==>>: send data to slave " << slave << endl << "DDDDDAAAATTTTTAA" << endl << current.matrix();
                MPI_Send(current.data(), current.dataSize(), MPI_DOUBLE, slave, 1, MPI_COMM_WORLD);
                break;
                
            }
                
            {case Task::BASIS_MUL:
                if (DBG) cout << endl << "Master sender ==>> BASIS_MUL task "<<endl;
                for (int slave_id=1; slave_id<mNumProc; slave_id++) {
                    MPI_Send(&task, sizeof(Task), MPI_CHAR, slave_id, 0, MPI_COMM_WORLD);
                   
                    Callback_S1* cb = current.callback();
                    // TODO
                    //cb->setTargetResult(mNumProc);
                    mCallbackVec.at(slave_id) = current.callback();
                }
                
                cout << "Master sender ==>> BASIS_MUL MPI_Barrier "<<endl;
                //MPI_Barrier(MPI_COMM_WORLD);
                cout << "Master sender ==>> BASIS_MUL pass MPI_Barrier "<<endl;
                MPI_Bcast(current.data(), current.dataSize(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
                break;
            }

                
            {default:
                break;
            }
        }
    }

    cout << endl <<"Master sender EXIT";
    
}

void Master::receiver() {
    
    int dataSize;
    MPI_Status status;
    
    while (!mExit) {
        
        cout << endl <<"Master receiver <<==: Receiver wait to receive next msg" << endl;
        
        // May Block here
        MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, MPI_COMM_WORLD, &status);
        if (mExit) break;
        
        MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
        vector<double> buffer(dataSize);
        MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        
        if (DBG) cout << endl <<"Master receiver <<==: receive from slave " << status.MPI_SOURCE << endl;
        
        for (int i=0; i<mCallbackVec.size(); i++) cout << endl << "mCallbackVec " << i << " " << mCallbackVec.at(i);
        
        // Callback function knows how to handle data
        if (mCallbackVec.at(status.MPI_SOURCE) != NULL) {
            cout << endl << "Master receiver <<==: Calling callback   slave:"  << status.MPI_SOURCE;

            mCallbackVec.at(status.MPI_SOURCE)->notify(&buffer[0]);
            mCallbackVec.at(status.MPI_SOURCE) = NULL;
        }
        
        else cout << endl << "Master receiver <<==: No callback";

        // Put the slave back to available pool
        mAvailSlave.push(status.MPI_SOURCE);

    }

    cout << endl <<"Master receiver EXIT" << endl;
}

void Master::submit(TaskParcel parcel) {
    mTaskQueue.push(parcel);
}


void Master::terminate() {
    
    cout << endl <<"Master Terminate!! ";
    
    Task task(Task::TERMINATE);
    for (int id=1; id<mNumProc; id++) {
        MPI_Send(&task, sizeof(task), MPI_CHAR, id, 0, MPI_COMM_WORLD);
    }

    mExit = 1;

    // Send fake message to sender for termination
    TaskParcel tp(task);
    mTaskQueue.push(tp);
    
    // Send fake message to receiver for termination
    MatrixXd nullMatrix(1,1);
    nullMatrix << 0;
    MPI_Send(nullMatrix.data(), nullMatrix.size(), MPI_DOUBLE, 0, Task::RETURN_TAG, MPI_COMM_WORLD);

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

