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
#include "mpi.h"

#endif /* defined(__DistriSpectral__Slave__) */


#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif

#define AROW 1000
#define ACOL 1000
#define BROW 1000
#define BCOL 1000
#define BUF_SIZE AROW*BCOL

using namespace std;
using namespace Eigen;

class Slave {
    
public:
    
    Slave(int id): mId(id) {
        
    }
    
    void run();
    
private:
    
    int mId;
    
};