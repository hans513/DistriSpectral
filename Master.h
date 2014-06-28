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
    
    static const int DBG = 1;
    static const int DBG_CALLBACK = 1;
    
    Master(int numproc, MPI_Comm newComm): mNumProc(numproc), mComm(newComm)  {
        mExit = 0;
        for (int i=1; i<numproc; i++) {
            mAvailSlave.push(i);
        }
        mCallbackVec = vector<Callback*>(numproc);
    }
    
    ~Master() { }
    
    void run();
    void receiver();
    void sender();
    void terminate();
    void submit(TaskParcel parcel);
    
    void reset();
    
    int nProc() {
        return mNumProc;
    }
    
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
    
    // For debug
    void printCallback () {
        if (!DBG_CALLBACK) return;
        
        for (int i=0; i<mCallbackVec.size(); i++) {
            cout << endl << "mCallbackVec " << i << " " << mCallbackVec.at(i);
        }
    }
    
};
