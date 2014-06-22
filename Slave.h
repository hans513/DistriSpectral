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

class Slave {
    
public:
    
    static const int MASTER_ID = 0;
    static const int DBG = 0;
    
    Slave(int id): mId(id) {}
    
    void run();
    
    // Task functions
    void initialWork(Eigen::MatrixXd input, int target);
    void basisMul(Eigen::MatrixXd);
    void calTensor(Eigen:: MatrixXd basis);
    
private:

    int mId;
    void resetDataCache() {
        dataVec.clear();
    }
    vector<Eigen::MatrixXd> dataVec;
};