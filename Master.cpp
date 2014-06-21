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
        
        cout << endl <<"Master SENDER ==>> pop next task" << endl;
        
        // May Block here: get next task to assign
        TaskParcel current = mTaskQueue.pop();
        cout << endl << "Master SENDER ==>> After pop task";
        Task task = current.task();
        
        if (task.cmd()==Task::TERMINATE) break;
        
        // I should make it more general here
        switch (task.cmd()) {

            {case Task::INITIAL:
                // May Block here: get a available slave to assign
                cout << endl <<"Master SENDER ==>> before pop slave" << endl;
                int slave = mAvailSlave.pop();
                cout << endl <<"Master SENDER ==>> after pop slave";
                
                cout << endl <<"Master SENDER ==>> send task " << current.task().cmd() << " to slave " << slave << "  [" <<task.id()<<"]";
                
                // register callback function
                setCallback(slave, current.callback());
                
                MPI_Send(&task, sizeof(Task), MPI_CHAR, slave, 0, MPI_COMM_WORLD);
                
                if (current.data()==NULL) {
                    cout << endl <<"Master SENDER ==>> !!!!!!No data to be sent ???? " << slave << endl;
                    continue;
                }
                cout << endl <<"Master SENDER ==>> send data to slave " << slave << "  [" <<task.id()<<"]"<< endl;
                      MPI_Request request;
                MPI_Isend(current.data(), current.dataSize(), MPI_DOUBLE, slave, 1, MPI_COMM_WORLD, &request);
                cout << endl <<"Master SENDER ==>> finish sending data to slave " << slave << endl;
                break;
                
            }
                
            {case Task::BASIS_MUL:
                cout << endl << "Master SENDER ==>> BASIS_MUL task "<< "  [" <<task.id()<<"]"<<endl;
                for (int slave_id=1; slave_id<mNumProc; slave_id++) {
                    int slave = mAvailSlave.pop();
                    
                    MPI_Send(&task, sizeof(Task), MPI_CHAR, slave_id, 0, MPI_COMM_WORLD);
                    if (DBG) cout << endl << "Master SENDER ==>> setCallback for " <<slave;
                    Callback* cb = current.callback();
                    // TODO
                    cb->setTargetResult(mNumProc-1);
                    setCallback(slave, current.callback());
                }
                
                printCallback ();
                if (DBG) cout << endl << "Master SENDER ==>> BASIS_MUL MPI_Barrier ";
                //MPI_Barrier(MPI_COMM_WORLD);
                MPI_Bcast(current.data(), current.dataSize(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
                break;
            }

                
            {default:
                break;
            }
        }
    }

    cout << endl <<"Master SENDER ==>> EXIT";
    
}

void Master::receiver() {
    
    int dataSize;
    MPI_Status status;
    
    while (!mExit) {
        
        cout << endl <<"Master RECEIVER <<== wait to receive next msg" << endl;
        
        // May Block here
        MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, MPI_COMM_WORLD, &status);
        if (mExit) break;
        
        MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
        vector<double> buffer(dataSize);
        MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        
        cout << endl <<"Master RECEIVER <<== receive from slave " << status.MPI_SOURCE;
        
        printCallback();
        
        {
            std::unique_lock<std::mutex> lock(mCallback_mutex);
            // Callback function knows how to handle data
            if (mCallbackVec.at(status.MPI_SOURCE) != NULL) {
                cout << endl << "Master RECEIVER <<== Calling callback slave:"  << status.MPI_SOURCE;
                mCallbackVec.at(status.MPI_SOURCE)->notify(&buffer[0]);
                mCallbackVec.at(status.MPI_SOURCE) = NULL;
            }
            else cout << endl << "Master RECEIVER <<== No callback";
        }
        
        printCallback ();

        // Put the slave back to available pool
        cout << endl << "Master RECEIVER <<== Before push slave back";
        mAvailSlave.push(status.MPI_SOURCE);
        cout << endl << "Master RECEIVER <<== After push slave back";

    }

    cout << endl <<"Master RECEIVER <<== EXIT" << endl;
}

void Master::submit(TaskParcel parcel) {
    mTaskQueue.push(parcel);
}


void Master::terminate() {
    
    cout << endl <<"Master ===== Terminate!! =====";
    
    Task task(Task::TERMINATE);
    for (int id=1; id<mNumProc; id++) {
        MPI_Send(&task, sizeof(task), MPI_CHAR, id, 0, MPI_COMM_WORLD);
    }

    mExit = 1;

    // Send fake message to SENDER for termination
    TaskParcel tp(task);
    mTaskQueue.push(tp);
    
    // Send fake message to RECEIVER for termination
    MatrixXd nullMatrix(1,1);
    nullMatrix << 0;
    MPI_Send(nullMatrix.data(), nullMatrix.size(), MPI_DOUBLE, 0, Task::RETURN_TAG, MPI_COMM_WORLD);

}

void Master::setCallback(int slave, Callback* callback) {
    std::unique_lock<std::mutex> lock(mCallback_mutex);
    mCallbackVec.at(slave) = callback;
}

void Master::reset() {
    Task task(Task::RESET);
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

