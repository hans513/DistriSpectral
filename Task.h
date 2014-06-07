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
#include <cstring>
#endif /* defined(__DistriSpectral__Task__) */


using namespace std;

class Task {

    
public:
    
    static const int DBG = 1;
    
    static const int TERMINATE = -1;
    static const int INITIAL = 1;
    static const int MULTIPLY = 10;
    static const int TEST = 99;
    
    /*
    Task(int cmd) {
        Task(cmd, NULL);
    }*/
     
    
    Task(int cmd, int* size=NULL): mCmd(cmd) {
        if (size!=NULL) memcpy( mSize, size, sizeof(mSize));
        
        if (DBG) cout << "Task cmd: " << mCmd << "  Generated" << endl;
        
    }
    
    int cmd() {
        return mCmd;
    }
    
    int* size() {
        return mSize;
    }

private:
    int mCmd;
    int mSize[2];
    
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
