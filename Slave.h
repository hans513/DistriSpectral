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
#include <Eigen/Dense>
#include <stdlib.h>
#include <vector>
#include <random>
#include "mpi.h"

#endif /* defined(__DistriSpectral__Slave__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif


class Slave {
    
public:
    
    static const int MASTER_ID = 0;
    static const int DBG = 1;
    
    Slave(int id): mId(id) {}
    
    void run();
    void initialWork(Eigen::MatrixXd input, int target);
    void basisMul(Eigen::MatrixXd);
    
private:
    
    int mId;
    vector<Eigen::MatrixXd> dataVec;
    
};