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
    
    
    int p = 4;
    
    MatrixXd rpj = initialize(p);
    
    MatrixXd basis = calculateBasis(rpj, p);
    
    cout << endl << endl << "BASIS="  << basis << endl << endl << endl;
    
    MatrixXd Z = computeZ(basis);
    
    cout << endl << "Z=" << Z;
    
    
    
    
    
    finish();

}


// Split data or generate data info here
MatrixXd Logic::initialize(int nTarget) {
    
    int nDimension = 4;
    int nGaussian = 2;
    int nDataPerGaussian = 10;
    double noise = 1; //variance
    double unitRadius =10;
    
    int nChunk = 4;

    
    //For a large data input we want to split it here;
    data = new DataGenerator(nDimension, nGaussian, nDataPerGaussian, pow(noise,0.5), unitRadius);
    int blk = data->X().cols() / nChunk;
    for (int i=0; i<nChunk-1; i++) {
        mChunkVec.push_back(ChunkInfo(i*blk, (i+1)*blk));
    }
    mChunkVec.push_back(ChunkInfo((nChunk-1)*blk, data->X().cols()));

    
    long retSize[2] = {nDimension, nTarget};
    Callback_S1* callback = new Callback_S1(retSize, nChunk, this, &Logic::initialize_cb);

    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mWait = 1;
    }
    
    for (int i=0; i<nChunk; i++) {
        long nCol = mChunkVec.at(i).end()-mChunkVec.at(i).start();
        long size[2] = {nDimension, nCol};
        Task task(Task::INITIAL, size, nTarget);
        TaskParcel tp(task, data->X().middleCols(mChunkVec.at(i).start(), nCol), callback);
        mDispatcher->submit(tp);
    }
    
    cout << endl << "Logic ==============> start to wait";
    std::unique_lock<std::mutex> lock(mState_mutex);
    mState_condition.wait(lock);
    cout << endl << "Logic ==============> finish waiting";
    
    MatrixXd result = callback->result();
    delete callback;
    
    return result;
}

void Logic::initialize_cb() {
 
    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mWait = 0;
    }
    mState_condition.notify_one();

}

// TODO: shoud I use svd instead?
MatrixXd Logic::calculateBasis(MatrixXd rpj, int nTarget) {
    
    //const JacobiSVD<MatrixXd> svd(rpj, ComputeThinU | ComputeThinV);
    //MatrixXd Ub = svd.matrixU();
    
    HouseholderQR<MatrixXd> qr(rpj);
    MatrixXd Q = MatrixXd::Identity(rpj.rows(),nTarget);
    Q = qr.householderQ() * Q;

    return Q;
}

MatrixXd Logic::computeZ(MatrixXd basis) {


    long retSize[2] = {basis.cols(), basis.cols()};
    
    // TODO 2=nprocess
    Callback_S1* callback = new Callback_S1(retSize, 2, this, &Logic::computeZ_cb);
    
    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mWait = 1;
    }
    

    long size[2] = {basis.cols(), basis.cols()};
    Task task(Task::BASIS_MUL, size);
    TaskParcel tp(task, basis, callback);
    mDispatcher->submit(tp);

    cout << endl << "Logic ==============> start to wait";
    std::unique_lock<std::mutex> lock(mState_mutex);
    mState_condition.wait(lock);
    cout << endl << "Logic ==============> finish waiting";
    
    MatrixXd result = callback->result();
    delete callback;
    
    return result;
    
    
}

void Logic::computeZ_cb() {

    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mWait = 0;
    }
    mState_condition.notify_one();
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
    
    long size[]={3,3};
    
    Task task(Task::INITIAL, size);
    
    MPI_Send(&task, sizeof(task), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    MPI_Send(a.data(), a.size(), MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
    
}
