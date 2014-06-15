//
//  DataGenerator.h
//  SpecGmm
//
//  Created by Huang, Tse-Han on 2014/5/17.
//  Copyright (c) 2014年 Huang, Tse-Han. All rights reserved.
//

#ifndef SpecGmm_DataGenerator_h
#define SpecGmm_DataGenerator_h

#include <random>
#include <vector>
#include <time.h>
#include <Eigen/Dense>
#include <iostream>

#endif

typedef long IndexType;


// A container for data characteristic settings
class DataSettings {
    
public:
    DataSettings(IndexType nDimension, IndexType nCluster
                 , IndexType nDataPerGaussian, double noise, double unitRadius)
    :   mDimension(nDimension),
        mCluster(nCluster),
        mDataPerGaussian(nDataPerGaussian),
        mNoise(noise),
        mUnitRadius(unitRadius){ };
    
    DataSettings(){};

    IndexType dimension()       {return mDimension;}
    IndexType cluster()         {return mCluster;}
    IndexType dataPerCluster()  {return mDataPerGaussian;}
    double noise()              {return mNoise;}
    double unitRadius()         {return mUnitRadius;}
    
private:
    IndexType mDimension;
    IndexType mCluster;
    IndexType mDataPerGaussian;
    double mNoise;
    double mUnitRadius;
};

class DataGenerator {
    
public:
    
    DataGenerator(IndexType nDimension, IndexType nGaussian, IndexType nDataPerGaussian, double noise, double unitRadius) {
                
                mPara = DataSettings(nDimension, nGaussian
                                         , nDataPerGaussian, noise, unitRadius);
                initialize();
            };
    
    DataGenerator(DataSettings settings) {
        mPara = settings;
        initialize();
    }
    

    void initialize();

    Eigen::MatrixXd X() {return mX;}
    Eigen::MatrixXd center() {return mCenters;}
    double evaluate(Eigen::MatrixXd estimate);
    
    
private:
    DataSettings mPara;
    Eigen::MatrixXd mCenters;
    Eigen::MatrixXd mX;
};
