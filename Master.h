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

#include <Eigen/Dense>
#include "mpi.h"

#endif /* defined(__DistriSpectral__Master__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif


#ifndef SpecGmm_DataGenerator_h
#include "DataGenerator.h"
#endif

#ifndef __DistriSpectral__BlockingQueue__
#include "BlockingQueue.h"
#endif


class Master {
    
public:
    
    static const int DBG = 1;
    
    Master(int numproc): mNumProc(numproc) {
        mExit = 0;
        for (int i=1; i<numproc; i++) {
            mAvailSlave.push(i);
        }
        mCallbackVec = vector<Callback>(numproc-1);
    }
    
    ~Master() {
        //if (data) delete data;
    }
    
    void run();
    void receiver();
    void sender();
    //void initialize();
    //void test();
    //void test_initial();
    void terminate();
    void submit(TaskParcel parcel);
    
private:
    int mNumProc;
    int mExit;
    
    BlockingQueue<TaskParcel> mTaskQueue;
    
    BlockingQueue<int> mAvailSlave;
    
    vector<Callback> mCallbackVec;
    
    // The state of slvae: 0 = free, 1 = busy
    //vector<int> mSlaveState;
    
    
    
    //vector<ChunkInfo> mChunkVec;
    
    
    // Temporary data (Should be removed in the future)
    //MatrixXd mDataset;
    //DataGenerator *data;
    
};
