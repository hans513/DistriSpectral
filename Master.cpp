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
    cout << "\tRemote >> mId:on duty" << endl;
}

void Master::sender() {
    
    pid_t pid = getpid();
    cout << endl <<"\tMaster SENDER ==>> pid:"  << pid;
    
    while (!mExit) {
        
        if (DBG) cout << endl <<"\tMaster SENDER ==>> pop next task" << endl;
        
        // We don't have task to assign or we don't have slave available now.
        // Start to receive result from slaves.
        if (mTaskQueue.size()==0 || mAvailSlave.size()==0) {
            if(isWaitingCallback()) receiver(0);
        }

        // May Block here: get next task to assign
        TaskParcel current = mTaskQueue.pop();
        if (DBG) cout << endl << "\tMaster SENDER ==>> After pop task";
        Task task = current.task();
        
        if (task.cmd()==Task::TERMINATE) break;
        
        // I should make it more general here
        switch (task.cmd()) {

            {case Task::INITIAL:
                // May Block here: get a available slave to assign
                if (DBG) cout << endl <<"\tMaster SENDER ==>> before pop slave" << endl;
                int slave = mAvailSlave.pop();
                if (DBG) cout << endl <<"\tMaster SENDER ==>> after pop slave";
                
                cout << endl <<"\tMaster SENDER ==>> send task " << Task::cmdToString(task.cmd())  << " to slave " << slave << "  [" <<task.id()<<"]";
                
                // register callback function
                setCallback(slave, current.callback());
                
                MPI_Send(&task, sizeof(Task), MPI_BYTE, slave, 0, MPI_COMM_WORLD);
                
                if (current.data()==NULL) {
                    cout << endl <<"ERROR!!!!!! \tMaster SENDER ==>> !!!!!!No data to be sent ???? " << slave << endl;
                    continue;
                }
                if (DBG) cout << endl <<"\tMaster SENDER ==>> send data to slave " << slave << "  [" <<task.id()<<"]"<< endl;
                if (DBG) cout << endl << "\tMaster SENDER ==>> Matrix size:"  << current.dataSize();
                MPI_Send(current.data(), current.dataSize(), MPI_DOUBLE, slave, 1, MPI_COMM_WORLD);
                if (DBG) cout << endl <<"\tMaster SENDER ==>> finish sending data to slave " << slave << endl;
                break;
                
            }
                
            {case Task::BASIS_MUL:
             case Task::CAL_TENSOR:
                cout << endl << "\tMaster SENDER ==>>"<< Task::cmdToString(task.cmd()) << "  [" <<task.id()<<"]";
                
                // Assume all the slaves are idle now. We just send the task to every slaves with using the queue.
                // WARNING: the queue will be messy after these tasks
                for (int slave=1; slave<mNumProc; slave++) {
                    MPI_Send(&task, sizeof(Task), MPI_BYTE, slave, 0, MPI_COMM_WORLD);
                }
                
                // Set callback function here, since there are only few nodes are responsible for /
                // sending result back to master
                setTreeSumCallback(current.callback());

                if (DBG_CALLBACK) printCallback ();

                MPI_Bcast(current.data(), current.dataSize(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
                break;
            }

            {default:
                break;
            }
        }
    }

    cout << endl <<"\tMaster SENDER ==>> EXIT";
    
}

bool Master::isWaitingCallback() {
    
    for (int i=0; i<mCallbackVec.size(); i++) {
        if (mCallbackVec.at(i)!=NULL) return true;
    }
    
    return false;
}

void Master::receiveResult() {
    
    int dataSize;
    MPI_Status status;
    
    cout << endl <<"\tMaster RECEIVER <<== wait to receive next msg" << endl;
    
    // May Block here
    MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, mComm, &status);
    
    
    MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
    if (DBG) cout << endl <<"\tMaster RECEIVER <<== about to receive dataSize:"<< dataSize << endl;
    vector<double> buffer(dataSize);
    MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, mComm, &status);
    if (mExit) return;
    
    cout << endl <<"\tMaster RECEIVER <<== receive from slave " << status.MPI_SOURCE;
    
    printCallback();
    
    {
        std::unique_lock<std::mutex> lock(mCallback_mutex);
        // Callback function knows how to handle data
        if (mCallbackVec.at(status.MPI_SOURCE) != NULL) {
            if (DBG_CALLBACK) cout << endl << "\tMaster RECEIVER <<== Calling callback slave:"  << status.MPI_SOURCE;
            mCallbackVec.at(status.MPI_SOURCE)->notify(&buffer[0]);
            mCallbackVec.at(status.MPI_SOURCE) = NULL;
        }
        else cout << endl << "\tMaster RECEIVER <<== No callback";
    }
    
    printCallback ();
    
    // Put the slave back to available pool
    if (DBG) cout << endl << "\tMaster RECEIVER <<== Before push slave back";
    mAvailSlave.push(status.MPI_SOURCE);
    if (DBG) cout << endl << "\tMaster RECEIVER <<== After push slave back";
}

void Master::waitingCallback() {
    
    int dataSize;
    MPI_Status status;
    
    while (isWaitingCallback()) {
        receiveResult();
        /*
        cout << endl <<"\tMaster RECEIVER <<== wait to receive next msg" << endl;
        
        // May Block here
        MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, mComm, &status);
        
        
        MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
        if (DBG) cout << endl <<"\tMaster RECEIVER <<== about to receive dataSize:"<< dataSize << endl;
        vector<double> buffer(dataSize);
        MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, mComm, &status);
        if (mExit) break;
        
        cout << endl <<"\tMaster RECEIVER <<== receive from slave " << status.MPI_SOURCE;
        
        printCallback();
        
        {
            std::unique_lock<std::mutex> lock(mCallback_mutex);
            // Callback function knows how to handle data
            if (mCallbackVec.at(status.MPI_SOURCE) != NULL) {
                if (DBG_CALLBACK) cout << endl << "\tMaster RECEIVER <<== Calling callback slave:"  << status.MPI_SOURCE;
                mCallbackVec.at(status.MPI_SOURCE)->notify(&buffer[0]);
                mCallbackVec.at(status.MPI_SOURCE) = NULL;
            }
            else cout << endl << "\tMaster RECEIVER <<== No callback";
        }
        
        printCallback ();
        
        // Put the slave back to available pool
        if (DBG) cout << endl << "\tMaster RECEIVER <<== Before push slave back";
        mAvailSlave.push(status.MPI_SOURCE);
        if (DBG) cout << endl << "\tMaster RECEIVER <<== After push slave back";
         */
        
    }
    
}

void Master::receiver(int runningWhenIdle) {
    
    if (runningWhenIdle) {
        pid_t pid = getpid();
        cout << endl <<"\tMaster RECEIVER ==>> pid:"  << pid << endl;
    }

    int dataSize;
    MPI_Status status;
    
    while (!mExit && (runningWhenIdle||isWaitingCallback()) ) {
  
        cout << endl <<"\tMaster RECEIVER <<== wait to receive next msg";
        
        // May Block here
        MPI_Probe(MPI_ANY_SOURCE, Task::RETURN_TAG, mComm, &status);
        
        
        MPI_Get_count(&status, MPI_DOUBLE, &dataSize);
        if (DBG) cout << endl <<"\tMaster RECEIVER <<== about to receive dataSize:"<< dataSize << endl;
        vector<double> buffer(dataSize);
        MPI_Recv(&buffer[0], dataSize, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, mComm, &status);
        if (mExit) break;
        
        cout << endl <<"\tMaster RECEIVER <<== receive from slave " << status.MPI_SOURCE;
        
        printCallback();
        
        {
            std::unique_lock<std::mutex> lock(mCallback_mutex);
            // Callback function knows how to handle data
            if (mCallbackVec.at(status.MPI_SOURCE) != NULL) {
                if (DBG_CALLBACK) cout << endl << "\tMaster RECEIVER <<== Calling callback slave:"  << status.MPI_SOURCE;
                mCallbackVec.at(status.MPI_SOURCE)->notify(&buffer[0]);
                mCallbackVec.at(status.MPI_SOURCE) = NULL;
            }
            else cout << endl << "\tMaster RECEIVER <<== No callback";
        }
        
        printCallback ();

        // Put the slave back to available pool
        if (DBG) cout << endl << "\tMaster RECEIVER <<== Before push slave back";
        mAvailSlave.push(status.MPI_SOURCE);
        if (DBG) cout << endl << "\tMaster RECEIVER <<== After push slave back";
        

    }

    cout << endl <<"\tMaster RECEIVER <<== EXIT" << endl;
}

void Master::submit(TaskParcel parcel) {
    mTaskQueue.push(parcel);
}

void Master::submit(vector<TaskParcel> vec) {
    mTaskQueue.push(vec);
}


void Master::terminate() {
    
    cout << endl <<"\tMaster ===== Terminate!! =====";
    
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

