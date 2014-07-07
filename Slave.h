//
//  Slave.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#ifndef __DistriSpectral__Slave__
#define __DistriSpectral__Slave__

#include <iostream>
#include <vector>
#include <random>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <unistd.h>

#include <Eigen/Dense>

#include "mpi.h"

#endif /* defined(__DistriSpectral__Slave__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif

#ifndef __SpecGmm__D3Matrix__
#include "D3Matrix.h"
#endif

#ifndef DistriSpectral_Misc_h
#include "Misc.h"
#endif

#ifndef __SpecGmm__Fastfood__
#include "Fastfood.h"
#endif /* defined(__SpecGmm__Fastfood__) */

//static Eigen::MatrixXd Global_sum(MatrixXd myData, int my_rank, int p, MPI_Comm comm);
//static Eigen::MatrixXd receiveMatrixFrom(int sender, long size[2]);

class Slave {
    
public:
    
    static const int MASTER_ID = 0;
    static const int DBG = 0;
    
    Slave(int id, int nProc, MPI_Comm newComm):
        mId(id), mTotalProc(nProc),mComm(newComm) {
    
        sprintf(mMsgHeader,"Remote >> mId: %d", mId);
    }
    
    void run();
    
private:
    
    int mId;
    int mTotalProc;
    void resetDataCache() {
        dataVec.clear();
    }
    vector<Eigen::MatrixXd> dataVec;
    
    MPI_Comm mComm;
    
    // Task functions
    void initialWork(Eigen::MatrixXd input, int target);
    void basisMul(Eigen::MatrixXd);
    void calTensor(Eigen:: MatrixXd basis);
    
    Eigen::MatrixXd Global_sum(MatrixXd myData, int my_rank, int p, MPI_Comm comm);
    Eigen::MatrixXd receiveMatrixFrom(int sender, int tag, long size[2]);
    
    // Settings
    int mWithFastfood;
    int mWithDistSvd;
    
    char mMsgHeader[64];
};