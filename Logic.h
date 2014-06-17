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
#include <mutex>
#include <condition_variable>

#endif /* defined(__DistriSpectral__Logic__) */


#ifndef __DistriSpectral__Master__
#include "Master.h"
#endif 

#ifndef __DistriSpectral__Task__
#include "Task.h"
#endif

#ifndef __SpecGmm__D3Matrix__
#include "D3Matrix.h"
#endif

#ifndef __SpecGmm__TesnsorPower__
#include "TensorPower.h"
#endif

#ifndef DistriSpectral_Misc_h
#include "Misc.h"
#endif

typedef long IndexType;

class Logic {
    
public:
    
    static const bool DBG = true;
    static const bool PRINT_MATRIX = false;
    static const bool TIME_MEASURE = true;
    
    // State for mWait
    static const int STATE_ACTIVE = 0;
    static const int STATE_WAIT = 1;
    
    Logic(Master& master): mDispatcher(&master) {}
    ~Logic() { }
    
    void start(MatrixXd X, int K, double noise);
    void initialize_cb();
    void computeZ_cb();

    // Return Estimate centers after computation
    Eigen::MatrixXd centers() {return mCenters;};


    void finish();
    
private:

    Eigen::MatrixXd initialize(MatrixXd X, int nTarget);
    Eigen::MatrixXd calculateBasis(Eigen::MatrixXd rpj, int nTarget);
    Eigen::MatrixXd computeZ(Eigen::MatrixXd basis);
    Eigen::MatrixXd calculateWhiten(Eigen::MatrixXd bpj, Eigen::MatrixXd basis
                                    , int K, IndexType nData) ;
    

    

    // hold lock and change mWait
    void changeWaitState(int state);
    
    // The info of the split matrices
    vector<ChunkInfo> mChunkVec;

    // For dispatching tasks
    Master* mDispatcher;
    
    std::mutex              mState_mutex;
    std::condition_variable mState_condition;
    int mWait;

    Eigen::MatrixXd mCenters;

    // Temporary data (Should be removed in the future)
    //DataGenerator* mData;
    template <typename Derived>
    void tensorDecompose(D3Matrix<Derived> T, MatrixXd W);
    void afterWhiten(Eigen::MatrixXd X, Eigen::MatrixXd W, int K, double sigma);
};