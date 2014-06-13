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

class DataGenerator {
    
public:
    
    DataGenerator(unsigned long nDimension, unsigned long nGaussian, unsigned long nDataPerGaussian, double noise, double unitRadius)
        : mDimension(nDimension),
            mGaussian(nGaussian),
            mDataPerGaussian(nDataPerGaussian),
            mNoise(noise),
            mUnitRadius(unitRadius){
                initialize();
            };

    void initialize();

    
    Eigen::MatrixXd X() {return mX;}
    Eigen::MatrixXd center() {return mCenters;}
    double evaluate(Eigen::MatrixXd estimate);
    
    
private:
    unsigned long mDimension;
    unsigned long mGaussian;
    unsigned long mDataPerGaussian;
    double mNoise;
    double mUnitRadius;
    Eigen::MatrixXd mCenters;
    Eigen::MatrixXd mX;
    
};
