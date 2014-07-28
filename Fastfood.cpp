//
//  Fastfood.cpp
//  SpecGmm
//
//  Created by Huang, Tse-Han on 2014/5/18.
//  Copyright (c) 2014 Huang, Tse-Han. All rights reserved.
//


#include "Fastfood.h"

#ifndef __DistriSpectral__Hadamard__
#include "hadamardc.h"
#endif

using namespace std;
using namespace Eigen;

MatrixXi hadamardGenerator(long length) {
    MatrixXi next(2,2);
    next << 1,1,1,-1;
    long current = 2;
    
    MatrixXi previous;
    
    while(current < length) {
        
        previous = next;
        current*=2;
        next = MatrixXi(current, current);
        next << previous, previous, previous, -previous;
        
    }
    
    return next;
}

void testHadamardGen() {
    cout << endl << "hadamard 2" << endl << hadamardGenerator(2) << endl;
    cout << endl << "hadamard 4" << endl << hadamardGenerator(4) << endl;
    cout << endl << "hadamard 8" << endl << hadamardGenerator(8) << endl;
}

//  Test case for FastfoodRangeFinder
void test_FastfoodRangeFinder() {
    unsigned ndimension = 18;
    unsigned nbasis = 6;
    unsigned ndataPoints = 40;
    
    // Random basis
    MatrixXd basis = MatrixXd::Random(ndimension, nbasis);
    
    // nDimension * nDataPoints (rank=nBasis)
    MatrixXd span = basis * MatrixXd::Random(nbasis, ndataPoints);
    
    FastfoodRangeFinder ffFinder(span, nbasis);
    
    unsigned rankDiff = 0;
    
    // Check if all the column vectors in the matrix q we got are linear dependent
    // with the Basis we have above.
    for (int i=0; i<nbasis; i++) {
        MatrixXd test(ndimension, nbasis+1);
        test << ffFinder.Q(), basis.col(i);
        
        FullPivLU<MatrixXd> lu(test);
        lu.setThreshold(pow(10,-10));
        rankDiff += (lu.rank()-nbasis);
        cout << endl << "threshold:" << lu.threshold() << endl;
    }
    
    cout << endl << "Rank diff:" << rankDiff << endl;
}

// Usually m >> n
Fastfood::Fastfood(unsigned long m, unsigned long n): mTargetRows(m), mTargetCols(n) {
    assert (m>n);
    mRealCols = pow2roundup(n);
    mRealRows = mRealCols > m? mRealCols: m;
    mBlkLength = mRealCols;
    
    mNumBlk = mRealRows / mRealCols;
    
    mResidualRows = (unsigned)(mRealRows % mRealCols);
    mResidualCols = (unsigned)mBlkLength;
    
    initialize();
}

void Fastfood::initialize() {
    
    srand (time(NULL));
 
    random_device rd;
    default_random_engine generator(rd());
    normal_distribution<double> normal_distri(0, 1);
    
    for (int i=0; i<mNumBlk; i++) {

        VectorXi b(mBlkLength,1);
        VectorXd g(mBlkLength,1);
        VectorXi pi(mBlkLength,1);

        for (long j=0; j<mBlkLength; j++) pi(j)= (int)j;
        
        for (long j=0; j<mBlkLength; j++) {
            
            // Binary random number (+1, -1)
            int rnd = rand()%2;
            if (rnd==0) rnd = -1;
            b(j)=rnd;
            
            // Permutation: Random swap to make permutation
            int rnd2 = rand() % mBlkLength;
            int temp = pi(j);
            pi(j) = pi(rnd2);
            pi(rnd2) = temp;
            
            // Gaussian
            g(j) = normal_distri(generator);
        }
        
        mB.push_back(b);
        mPI.push_back(pi);
        mG.push_back(g);
    }
    
    if (mResidualRows == 0) return;

    mResidual = MatrixXd(mResidualRows, mResidualCols);
    
    for (int i=0; i<mResidualRows; i++) {
        for(int j=0; j<mResidualCols; j++) {
            mResidual(i, j) = normal_distri(generator);
        }
    }
}


MatrixXd Fastfood::multiply(MatrixXd input) {
    
    MatrixXd Ret = MatrixXd::Zero(input.rows(), mRealCols);
    
    for (long i=0; i<mNumBlk; i++) {
        MatrixXd B = mB.at(i).cast<double>().asDiagonal();
        MatrixXd G = mG.at(i).asDiagonal();
        VectorXi pi = mPI.at(i);
        
        vector<double> buffer(B.rows()*B.cols());
        hadamard_apply_matrix(&buffer[0], B.data(), B.rows(), B.cols());
        MatrixXd HB = Map<MatrixXd>(&buffer[0], B.rows(), B.cols());
        
        // Permutation
        MatrixXd PHB =  MatrixXd::Zero(mBlkLength,mBlkLength);
        for (long j=0; j<pi.size(); j++) {
            PHB.row(j) = HB.row(pi(j));
        }
        
        MatrixXd GPHB = G * PHB;
        hadamard_apply_matrix(&buffer[0], GPHB.data(), GPHB.rows(), GPHB.cols());
        MatrixXd F = Map<MatrixXd>(&buffer[0], GPHB.rows(), GPHB.cols());
        
        // Extract the correspondent block in the X
        MatrixXd inputBlock = input.middleCols(i * mBlkLength, mBlkLength);
        
        Ret += inputBlock * F;
    }
    
    if (mResidualRows != 0) {
        // The # of redisual col in input is eqaul to the # of redisual row in FF
        MatrixXd inputBlock = input.rightCols(mResidualRows);
        Ret += inputBlock* mResidual;
    }
    
    return Ret;
}


/**
 Use for verifying multiply
 */
MatrixXd Fastfood::multiply2(MatrixXd input) {
    
    MatrixXd realF(mRealRows, mRealCols);
    
    MatrixXd Ret = MatrixXd::Zero(input.rows(), mRealCols);
    
    for (long i=0; i<mNumBlk; i++) {
        MatrixXi B = mB.at(i).asDiagonal();
        MatrixXd G = mG.at(i).asDiagonal();
        VectorXi pi = mPI.at(i);
        //MatrixXi PI = MatrixXi::Zero(mBlkLength,mBlkLength);
        MatrixXd H = hadamardGenerator(mBlkLength).cast<double>();
        
        // Permutation
        // for (long j=0; j<pi.size(); j++) PI(j, pi(j)) = 1;
        
        MatrixXd HB = H * B.cast<double>();
        
        // Permutation
        MatrixXd PHB =  MatrixXd::Zero(mBlkLength,mBlkLength);
        for (long j=0; j<pi.size(); j++) {
            PHB.row(j) = HB.row(pi(j));
        }
        
        MatrixXd F =  H * G * PHB;
        
        // The fastfood matrix
        //MatrixXd F =  H * G * PI.cast<double>() * H * B.cast<double>();
        cout << endl << "ff F:" << F << endl;
        realF.middleRows(i * mBlkLength, mBlkLength) = F;
    }
    
    if (mResidualRows != 0) {
        realF.bottomRows(mResidualRows) = mResidual;
    }
    
    cout << endl << "Real F:" << realF << endl;
    
    Ret = input * realF;

    return Ret;
}
