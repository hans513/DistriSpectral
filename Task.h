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


using namespace std;


class Callback {
public:
    Callback(){};
    virtual void notify(void* data) {cout<<endl<<"virtual function!!";};
    void setname(char *input) {
        strncpy(mName, input, sizeof(mName));
    }
    char* name() {return mName;};
private:
    char  mName[32];
};


class Task {

public:
    
    static const int DBG = 1;
    
    static const int TERMINATE = -1;
    static const int INITIAL = 1;
    static const int MULTIPLY = 10;
    static const int TEST = 99;
    
    static const int RETURN_TAG = 9;
    
    /*
     Task(int cmd) {
     Task(cmd, NULL);
     }*/
    
    
    Task(int cmd, int size[2]=NULL, int info=0): mCmd(cmd), mInfo(info) {
        if (size!=NULL) memcpy( mSize, size, sizeof(mSize));
    }
    
    int cmd() {
        return mCmd;
    }
    
    int* size() {
        return mSize;
    }
    
    int info() {
        return mInfo;
    }
    
private:
    int mCmd;
    int mSize[2];
    int mInfo;
    
};


class TaskParcel {
  
public:
    TaskParcel(Task task, Eigen::MatrixXd data, Callback &callback): mTask(task){
        mData = data;
        mCallback = &callback;
    };
    
    Task task() {return mTask;}
    void* data() {return mData.data();}
    int dataSize(){return mData.size();}
    Callback callback() {return *mCallback;}
    
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
