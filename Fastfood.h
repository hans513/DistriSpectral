//
//  Fastfood.h
//  SpecGmm
//
//  Created by Huang, Tse-Han on 2014/5/18.
//  Copyright (c) 2014 Huang, Tse-Han. All rights reserved.
//

#ifndef __SpecGmm__Fastfood__
#define __SpecGmm__Fastfood__

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>
#include <Eigen/Dense>

#endif /* defined(__SpecGmm__Fastfood__) */


#ifndef DistriSpectral_Misc_h
#include "Misc.h"
#endif

void testHadamardGen();
void test_FastfoodRangeFinder();

class Fastfood {
public:
    
    // Usually m >> n
    Fastfood(unsigned long m, unsigned long n);
    
    Fastfood(){};
    
    void initialize();
    Eigen::MatrixXd multiply(Eigen::MatrixXd input);
    Eigen::MatrixXd multiply2(Eigen::MatrixXd input);
    
private:
    // The target dimension
    unsigned long mTargetRows;
    unsigned long mTargetCols;

    // The real dimension after making the length to be the power of 2
    unsigned long mRealRows;
    unsigned long mRealCols;
    
    // The residual matrix dimension
    unsigned mResidualRows;
    unsigned mResidualCols;

    // The length of each fastfood block block
    unsigned long mBlkLength;
    unsigned long mNumBlk;
    
    std::vector<Eigen::VectorXi> mB;
    std::vector<Eigen::VectorXi> mPI;
    std::vector<Eigen::VectorXd> mG;
    Eigen::MatrixXd mResidual;
};


class FastfoodRangeFinder {
    
public:
    
    FastfoodRangeFinder(Eigen::MatrixXd X, long target) {

        
        mFf =  Fastfood(X.cols(), target);
        Eigen::MatrixXd Y = mFf.multiply(X);
        
        //Eigen::MatrixXd Z = mFf.multiply2(X);
        //Eigen::MatrixXd diff = Y-Z;
        //cout << endl << "multiply dif" << endl<<diff.sum()<<endl;
        
        //cout << endl << "X after random ff" << endl<<Y<<endl;
        //Eigen::MatrixXd Y;
        //mFf.multiply(X, Y);
        
        Eigen::HouseholderQR<Eigen::MatrixXd> qr(Y);
        mQ = Eigen::MatrixXd::Identity(X.rows(),target);
        mQ = qr.householderQ() * mQ;
        
        //cout << endl << "Q:" << endl<< mQ <<endl;
    }

    Eigen::MatrixXd Q() {
        return mQ;
    }
    
private:
    Fastfood mFf;
    Eigen::MatrixXd mQ;
    //Eigen::MatrixXd mR;
};



