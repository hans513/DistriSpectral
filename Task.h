//
//  Task.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/7.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#ifndef __DistriSpectral__Task__
#define __DistriSpectral__Task__

#include <iostream>
#include <Eigen/Dense>
#include <cstring>

#endif /* defined(__DistriSpectral__Task__) */


#ifndef DistriSpectral_Misc_h
#include "Misc.h"
#endif

using namespace std;

class Logic;

class Callback {
    
public:
    Callback (long size[2], int target, Logic* logic, void (Logic::*cb) (void)): mLogic(logic), mCb(cb) {
        if (size!=NULL) {
            memcpy( mSize, size, sizeof(mSize));
            mResult = Eigen::MatrixXd::Zero(mSize[0], mSize[1]);
        }
        mTargetResult = target;
        mCurrentResult = 0;
    }
    
    virtual void notify(void* data) {
        
        Eigen::MatrixXd matrix = Eigen::Map<Eigen::MatrixXd>((double*)data, mSize[0], mSize[1]);
        
        cout << endl <<"Callback function " << mCurrentResult+1  << "/"  << mTargetResult<< endl;
        
        mResult += matrix;
        
        if (++mCurrentResult == mTargetResult) {
            //mLogic->initialize_cb();
            (mLogic->*mCb)();
        }
        
        cout << endl <<"Finish Callback";
    }
    
    Eigen::MatrixXd result() {return mResult;}
    void setTargetResult(int target) {mTargetResult = target;}

    
protected:
    // Pointer to main logic class
    Logic* mLogic;
    
    // The dimension of the result matrix
    long mSize[2];
    
    // Function pointer for the callback function
    void (Logic::*mCb) (void);

    Eigen::MatrixXd mResult;
    
    // Number of result we are supposed to receive
    int mTargetResult;
    
    // Number of result we received
    int mCurrentResult;
    

    Callback (Logic* logic, void (Logic::*cb) (void)): mLogic(logic), mCb(cb) {
    }
};

class EdoLibertyCallback : public Callback {
    
public:
    
    EdoLibertyCallback (long size[2], int target, Logic* logic, void (Logic::*cb) (void)): Callback (logic, cb) {
            
        if (size!=NULL) {
            memcpy( mSize, size, sizeof(mSize));
            mResult = Eigen::MatrixXd::Zero(mSize[0], 2*mSize[1]);
        }

        mTargetResult = target;
        mCurrentResult = 0;
    }

    void notify(void* data) {
  
        Eigen::MatrixXd matrix = Eigen::Map<Eigen::MatrixXd>((double*)data, mSize[0], mSize[1]);
        cout << endl <<"Edo Callback function " << mCurrentResult+1  << "/"  << mTargetResult<< endl;

        if (mResult.sum()==0) {
            mResult.middleCols(0, mSize[1]) = matrix;
        } else {
            mResult.middleCols(mSize[1], mSize[1]) = matrix;
            mResult = Shrinking(mResult.transpose(), mSize[1]);
            mResult.transposeInPlace();
        }

        if (++mCurrentResult == mTargetResult) {
            (mLogic->*mCb)();
        }

        cout << endl <<"Finish Callback";
    }
};


class Task {

public:
    
    static const int DBG = 1;
    
    static const int TERMINATE = -1;
    static const int INITIAL = 1;
    static const int BASIS_MUL = 2;
    static const int CAL_TENSOR = 3;
    static const int RESET = 10;
    static const int TEST = 99;
    
    static const int DATA_TAG = 1;
    static const int RETURN_TAG = 9;
    static const int TREESUM_TAG = 9;
    
    Task(int cmd, long size[2]=NULL, int info=0, int id=0): mCmd(cmd), mInfo(info), mId(id) {
        if (size!=NULL) memcpy( mSize, size, sizeof(mSize));
    }
    
    int cmd() {
        return mCmd;
    }
    
    long* size() {
        return mSize;
    }
    
    int info() {
        return mInfo;
    }
    
    int id() {
        return mId;
    }

    static std::string cmdToString(int cmd) {
        switch (cmd) {
            case Task::INITIAL:
                return std::string("Task::INITIAL");
                break;
            case Task::BASIS_MUL:
                return std::string("Task::BASIS_MUL");
                break;
            case Task::CAL_TENSOR:
                return std::string("Task::CAL_TENSOR");
                break;
            case Task::RESET:
                return std::string("Task::RESET");
                break;
            case Task::TERMINATE:
                return std::string("Task::TERMINATE");
                break;
            default:
                return std::string("Task:: default??");
                break;
        }
    }
    
private:
    int mCmd;
    long mSize[2];
    int mInfo;
    
    // For debugging
    int mId;
    
};


class TaskParcel {
  
public:
    TaskParcel(Task task, Eigen::MatrixXd data, Callback* callback): mTask(task){
        mData = data;
        mCallback = callback;
    };
    
    TaskParcel(Task task): mTask(task){};

    Task task()     {return mTask;}
    void* data()    {return mData.data();}
    int dataSize()  {return mData.size();}
    Callback* callback() {return mCallback;}
    
    Eigen::MatrixXd matrix() {return mData;}
    
private:
    Task mTask;
    Eigen::MatrixXd mData;
    Callback* mCallback;
};


class ChunkInfo {

public:
    ChunkInfo(int start, int end): mStart(start), mEnd(end){};
    
    void assign(int slave) {
        mSlave = slave;
    }
    
    int start() { return mStart;}
    int end() { return mEnd;}
    int slave() { return mSlave;}
    
private:
    int mStart;
    int mEnd;
    int mSlave;
};