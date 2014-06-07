//
//  Master.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#ifndef __DistriSpectral__Master__
#define __DistriSpectral__Master__

#include <iostream>
#include <Eigen/Dense>
#include "mpi.h"

#endif /* defined(__DistriSpectral__Master__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif

using namespace std;
using namespace Eigen;

class Master {
    
public:
    
    Master() {  
    }
    
    void run();
    void test();
    
private:
    
    
};
