//
//  Logic.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/9.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#include "Logic.h"


using namespace std;
using namespace Eigen;

void Logic::start() {
    
    mWait = 0;
    initialize();
    
    cout << endl << "Logic ==============> start to wait";

    std::unique_lock<std::mutex> lock(mState_mutex);
    mState_condition.wait(lock);
    cout << endl << "Logic ==============> finish waiting";
    
    finish();

}


// Split data or generate data info here
void Logic::initialize() {
    
    int nDimension = 4;
    int nGaussian = 2;
    int nDataPerGaussian = 10;
    double noise = 1; //variance
    double unitRadius =10;
    
    int nChunk = 4;
    int nTarget = 5;
    
    //For a large data input we want to split it here;
    data = new DataGenerator(nDimension, nGaussian, nDataPerGaussian, pow(noise,0.5), unitRadius);
    int blk = data->X().cols() / nChunk;
    for (int i=0; i<nChunk-1; i++) {
        mChunkVec.push_back(ChunkInfo(i*blk, (i+1)*blk));
    }
    mChunkVec.push_back(ChunkInfo((nChunk-1)*blk, data->X().cols()));

    
    int retSize[2] = {nDimension, nTarget};
    Callback_S1* callback = new Callback_S1(retSize, nChunk, this);

    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mWait = 1;
    }
    
    for (int i=0; i<nChunk; i++) {
        int nCol = mChunkVec.at(i).end()-mChunkVec.at(i).start();
        int size[2] = {nDimension, nCol};
        Task task(Task::INITIAL, size, nTarget);
        TaskParcel tp(task, data->X().middleCols(mChunkVec.at(i).start(), nCol), callback);
        mDispatcher->submit(tp);
    }
}

void Logic::finish() {
    
    mDispatcher->terminate();
    cout << endl << "Logic ==============> finish ";
}

void Logic::test() {
    
    for (int i=0; i<mChunkVec.size(); i++) {
        cout << "start:" << mChunkVec.at(i).start()<< "  end:" << mChunkVec.at(i).end() << endl;
    }
    
}

void Logic::test_initial() {
    
    MatrixXd a(3,3);
    a << 1,2,3,4,5,6,7,8,9;
    
    int size[]={3,3};
    
    Task task(Task::INITIAL, size);
    
    MPI_Send(&task, sizeof(task), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    MPI_Send(a.data(), a.size(), MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
    
}
