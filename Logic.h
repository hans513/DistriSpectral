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
    //void terminate();
    
private:
    //int mNumProc;
    vector<ChunkInfo> mChunkVec;
    
    Master* mDispatcher;
    
    Eigen::MatrixXd mBufMatrix;
    
    // Temporary data (Should be removed in the future)
    DataGenerator *data;
    
};


class Callback_S1 : public Callback {
    
public:
    Callback_S1(int size[2], Logic* logic): mLogic(logic) {
        if (size!=NULL) memcpy( mSize, size, sizeof(mSize));
    }
    
    void notify(void* data) {
        cout << endl <<"Notify!!" << endl;
        cout << endl <<"Notify!!" << endl;
        Eigen::MatrixXd matrix = Eigen::Map<Eigen::MatrixXd>((double*)data, mSize[0], mSize[1]);
        mLogic->mBufMatrix += matrix;
    }
private:
    int mSize[2];
    Logic* mLogic;
};