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

#endif /* defined(__DistriSpectral__Task__) */


class Task {

    
public:
    
    static const int MULTIPLY = 1;
    static const int TEST = 99;
    
    Task(int cmd, int* size): mCmd(cmd){
        memcpy( mSize, size, sizeof(mSize));
    }
    
    /*
    long size() {
        return mDAta.size()+ sizeof(int);
    }
     */
    
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