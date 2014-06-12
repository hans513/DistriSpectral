//
//  Logic.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/9.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#ifndef __DistriSpectral__Logic__
#define __DistriSpectral__Logic__

#include <iostream>
#include <mutex>
#include <condition_variable>

#endif /* defined(__DistriSpectral__Logic__) */


#ifndef __DistriSpectral__Master__
#include "Master.h"
#endif /* defined(__DistriSpectral__Master__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif


class Logic {
    
    friend class Callback_S1;
    
public:
    
    static const int DBG = 1;
    
    Logic(Master& master): mDispatcher(&master) {
    }
    
    ~Logic() {
        if (data) delete data;
    }
    
    void start();
    void initialize();
    void test();
    void test_initial();
    void finish();
    //void terminate();
    
    
        Eigen::MatrixXd check;
private:
    //int mNumProc;
    vector<ChunkInfo> mChunkVec;

    Master* mDispatcher;


    std::mutex              mState_mutex;
    std::condition_variable mState_condition;
    int mWait;
    
    // Temporary data (Should be removed in the future)
    DataGenerator *data;
    

  
};




class Callback_S1 : public Callback {
    
public:
    Callback_S1(int size[2], int target, Logic* logic): mLogic(logic) {
        if (size!=NULL) {
            memcpy( mSize, size, sizeof(mSize));
            mResult = Eigen::MatrixXd::Zero(mSize[0], mSize[1]);
        }
        mTargetResult = target;
        mCurrentResult = 0;
    }
    
    void notify(void* data) {
        cout << endl <<"Notify!!  " << mCurrentResult  << endl;
        Eigen::MatrixXd matrix = Eigen::Map<Eigen::MatrixXd>((double*)data, mSize[0], mSize[1]);
        
        mResult += matrix;
        
        if (++mCurrentResult == mTargetResult) {
            cout << endl <<"Final Result!!" << mResult << endl;

            // Pass the result to mLogig
            // mLogic->mBufMatrix += matrix;
            
            {
                std::unique_lock<std::mutex> lock(mLogic->mState_mutex);
                mLogic->mWait = 0;
            }
            mLogic->mState_condition.notify_one();
            
            delete this;
        }
    }

private:
    int mSize[2];
    Logic* mLogic;
    Eigen::MatrixXd mResult;
    int mTargetResult;
    int mCurrentResult;
};