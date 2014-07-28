//
//  Logic.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/9.
//  Copyright (c) 2014 Tse-Han Huang. All rights reserved.
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
typedef double DataType;

class Logic {
    
public:
    
    static const bool DBG = true;
    static const bool PRINT_MATRIX = false;
    static const bool TIME_MEASURE = true;
    
//    static const int MAX_CHUNK = 240 * 1000 * 1000;
    static const int MAX_CHUNK = 60 * 1000 * 1000; //60M
    
    // State for mWait
    static const int STATE_ACTIVE = 0;
    static const int STATE_WAIT = 1;
    
    Logic(Master& master): mDispatcher(&master) {}
    ~Logic() { }
    
    void start(MatrixXd X, int K, double noise);
    void initialize_cb();
    void computeZ_cb();
    void buildTensor_cb();

    // Return Estimate centers after computation
    Eigen::MatrixXd centers() {return mCenters;};

    void finish();
    std::string getTimeDetail();
    
private:

    Eigen::MatrixXd initialize(MatrixXd X, int nTarget);
    Eigen::MatrixXd calculateBasis(Eigen::MatrixXd rpj, int nTarget);
    Eigen::MatrixXd computeZ(Eigen::MatrixXd basis);
    Eigen::MatrixXd calculateWhiten(Eigen::MatrixXd bpj, Eigen::MatrixXd basis
                                    , int K, IndexType nData) ;

    D3Matrix<MatrixXd> buildTensor(Eigen::MatrixXd X, Eigen::MatrixXd W, int K, double sigma);
    


    // hold lock and change mWait
    void changeWaitState(int state);
    
    // return the number of chunks we should split
    int decideChunks(MatrixXd X);
    
    // 
    IndexType decideP(int K, int dimension);
    
    // The info of the split matrices
    vector<ChunkInfo> mChunkVec;

    // For dispatching tasks
    Master* mDispatcher;
    
    std::mutex              mState_mutex;
    std::condition_variable mState_condition;
    int mWait;

    Eigen::MatrixXd mCenters;
    
    std::vector<int64> mTime;

    // Temporary data (Should be removed in the future)
    //DataGenerator* mData;
    template <typename Derived>
    void tensorDecompose(D3Matrix<Derived> T, MatrixXd W);
    void afterWhiten(Eigen::MatrixXd X, Eigen::MatrixXd W, int K, double sigma);
};
