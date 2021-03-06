//
//  Master.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#ifndef __DistriSpectral__Master__
#define __DistriSpectral__Master__

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <Eigen/Dense>
#include "mpi.h"

#include <sys/types.h>
#include <unistd.h>

#endif /* defined(__DistriSpectral__Master__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif

#ifndef SpecGmm_DataGenerator_h
#include "DataGenerator.h"
#endif

#ifndef DistriSpectral_BlockingQueue2_h
#include "BlockingQueue2.h"
#endif

class Master {
    
public:
    
    static const int DBG = 0;
    static const int DBG_CALLBACK = 0;
    
    Master(int numproc, int enableFf, int enableSvd, MPI_Comm newComm)
    : mNumProc(numproc), mComm(newComm), mWithFastfood(enableFf), mWithDistSvd(enableSvd)  {
        mExit = 0;
        for (int i=1; i<numproc; i++) {
            mAvailSlave.push(i);
        }
        mCallbackVec = vector<Callback*>(numproc);
    }
    
    ~Master() { }
    
    void run();
    void receiver(int runningWhenIdle);
    void sender();
    void terminate();
    void submit(TaskParcel parcel);
    void submit(vector<TaskParcel> vec);
    
    void reset();
    
    int nProc() {
        return mNumProc;
    }
    
    int withFastfood() {return mWithFastfood;}
    int withDistSvd() {return mWithDistSvd;}
    
private:

    int mNumProc;
    int mExit;
    MPI_Comm mComm;
    
    BlockingQueue<TaskParcel> mTaskQueue;
    BlockingQueue<int> mAvailSlave;
    
    vector<Callback*> mCallbackVec;
    std::mutex  mCallback_mutex;
    // Manipulate mCallbackVec
    void setCallback(int slave, Callback* callback);
    
    void setTreeSumCallback(Callback* callback);
    
    // Settings
    int mWithFastfood;
    int mWithDistSvd;
    

    bool isWaitingCallback();
    void waitingCallback();
    void receiveResult();
    
    // For debug
    void printCallback () {
        if (!DBG_CALLBACK) return;
        
        for (int i=0; i<mCallbackVec.size(); i++) {
            cout << endl << "mCallbackVec " << i << " " << mCallbackVec.at(i);
        }
    }
    
};
