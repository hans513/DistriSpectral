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
#include <vector>
#include "mpi.h"

#endif /* defined(__DistriSpectral__Master__) */

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif


#ifndef SpecGmm_DataGenerator_h
#include "DataGenerator.h"
#endif


class Master {
    
public:
    
    static const int DBG = 1;
    
    Master(int numproc): mNumProc(numproc) {
    }
    
    ~Master() {
        if (data) delete data;
    }
    
    void run();
    
    void initialize();
    void test();
    void test_initial();
    void terminate();
    
private:
    int mNumProc;
    vector<ChunkInfo> mChunkVec;
    
    
    // Temporary data (Should be removed in the future)
    //MatrixXd mDataset;
    DataGenerator *data;
    
};
