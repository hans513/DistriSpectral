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
    
    pid_t pid = getpid();
    cout << endl <<"Master SENDER ==>> pid:"  << pid << endl;
    
    while (!mExit) {
        
        cout << endl <<"Master SENDER ==>> pop next task" << endl;
        
        if (mTaskQueue.size() == 0) {
            waitingCallback();
        }
        
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
                
                MPI_Send(&task, sizeof(Task), MPI_BYTE, slave, 0, MPI_COMM_WORLD);
                
                if (current.data()==NULL) {
                    cout << endl <<"Master SENDER ==>> !!!!!!No data to be sent ???? " << slave << endl;
                    continue;
                }
                cout << endl <<"Master SENDER ==>> send data to slave " << slave << "  [" <<task.id()<<"]"<< endl;
                cout << endl << "Master SENDER ==>> Matrix size:"  << current.dataSize();
                MPI_Send(current.data(), current.dataSize(), MPI_DOUBLE, slave, 1, MPI_COMM_WORLD);
                cout << endl <<"Master SENDER ==>> finish sending data to slave " << slave << endl;
                break;
                
            }
                
            {case Task::BASIS_MUL:
             case Task::CAL_TENSOR:
                cout << endl << "Master SENDER ==>>"<< Task::cmdToString(task.cmd()) << "  [" <<task.id()<<"]"<<endl;
                
                // Assume all the slaves are idle now. We just send the task to every slaves with using the queue.
                // WARNING: the queue will be messy after these tasks
                for (int slave=1; slave<mNumProc; slave++) {
                    MPI_Send(&task, sizeof(Task), MPI_BYTE, slave, 0, MPI_COMM_WORLD);
                }
                
                // Set callback function here, since there are only few nodes are responsible for /
                // sending result back to master
                setTreeSumCallback(current.callback());

                printCallback ();

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

bool Master::isWaitingCallback() {
    
    for (int i=0; i<mCallbackVec.size(); i++) {
        if (mCallbackVec.at(i)!=NULL) return true;
    }
    
    return false;
}

void Master::waitingCallback() {
    
    int dataSize;
    MPI_Status status;
    
    while (isWaitingCallback()) {
        
        cout << endl <<"Master RECEIVER <<== wait to receive next msg" << endl;
        
        // May Block here
        MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, mComm, &status);
        
        
        MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
        cout << endl <<"Master RECEIVER <<== about to receive dataSize:"<< dataSize << endl;
        vector<double> buffer(dataSize);
        MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, mComm, &status);
        if (mExit) break;
        
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
        if (DBG) cout << endl << "Master RECEIVER <<== Before push slave back";
        mAvailSlave.push(status.MPI_SOURCE);
        if (DBG) cout << endl << "Master RECEIVER <<== After push slave back";
        
    }
    
}

void Master::receiver() {
    
    pid_t pid = getpid();
    cout << endl <<"Master RECEIVER ==>> pid:"  << pid << endl;

    
    int dataSize;
    MPI_Status status;
    
    while (!mExit) {
        
        cout << endl <<"Master RECEIVER <<== wait to receive next msg" << endl;
        
        // May Block here
        MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, mComm, &status);
        
        
        MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
        cout << endl <<"Master RECEIVER <<== about to receive dataSize:"<< dataSize << endl;
        vector<double> buffer(dataSize);
        MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, mComm, &status);
        if (mExit) break;
        
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
        if (DBG) cout << endl << "Master RECEIVER <<== Before push slave back";
        mAvailSlave.push(status.MPI_SOURCE);
        if (DBG) cout << endl << "Master RECEIVER <<== After push slave back";

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
        MPI_Send(&task, sizeof(task), MPI_BYTE, id, 0, MPI_COMM_WORLD);
    }

    mExit = 1;

    // Send fake message to SENDER for termination
    TaskParcel tp(task);
    mTaskQueue.push(tp);
    
    // Send fake message to RECEIVER for termination
    /*
    MatrixXd nullMatrix(1,1);
    nullMatrix << 0;
    MPI_Send(nullMatrix.data(), nullMatrix.size(), MPI_DOUBLE, 0, Task::RETURN_TAG, mComm);
    */
}

void Master::setCallback(int slave, Callback* callback) {
    std::unique_lock<std::mutex> lock(mCallback_mutex);
    mCallbackVec.at(slave) = callback;
}

void Master::reset() {

    mAvailSlave.clear();
    
    // Send enable Fastfood and enable distributed SVD through size column
    long setting[] = {mWithFastfood, mWithDistSvd};
    Task task(Task::RESET, setting);
    
    for (int id=1; id<mNumProc; id++) {
        MPI_Send(&task, sizeof(task), MPI_BYTE, id, 0, MPI_COMM_WORLD);
             mAvailSlave.push(id);
    }
}

void Master::setTreeSumCallback(Callback* callback) {
    
    int nFeedback = 0;
    int partner = 1;
 
    while (partner < mNumProc) {
        setCallback(partner, callback);
        nFeedback++;
        partner <<= 1;
    }

    callback->setTargetResult(nFeedback);
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
    
    MPI_Send(&task, sizeof(task), MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    MPI_Send(a.data(), a.size(), MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
    
}
 */

