//
//  Misc.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/16.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#ifndef DistriSpectral_Misc_h
#define DistriSpectral_Misc_h

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif


#include <Eigen/KroneckerProduct>

#endif


#ifndef __SpecGmm__D3Matrix__
#include "D3Matrix.h"
#endif

typedef long long int64;
typedef unsigned long long uint64;

static int64 GetTimeMs64() {
#ifdef WIN32
    /* Windows */
    FILETIME ft;
    LARGE_INTEGER li;
    
    /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
     * to a LARGE_INTEGER structure. */
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    
    uint64 ret = li.QuadPart;
    ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
    ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */
    
    return ret;
#else
    /* Linux */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64 ret = tv.tv_usec;
    /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
    ret /= 1000;
    
    /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
    ret += (tv.tv_sec * 1000);
    
    return ret;
#endif
}

template <typename Derived>
static D3Matrix<Derived> outer(const MatrixBase<Derived> &A, MatrixBase<Derived> &B) {
    
    MatrixXd m = kroneckerProduct(A,B).eval();
    
    //cout << "M row:" << m.rows() <<"  m.cols() " <<m.cols() << endl;
    
    unsigned long size[3];
    unsigned long index = 0;
    
    if (A.rows()>1) size[index++] = A.rows();
    if (A.cols()>1) size[index++] = A.cols();
    if (B.rows()>1) size[index++] = B.rows();
    if (B.cols()>1) size[index++] = B.cols();
    
    if (index>3) cout << "outer: size is larger than 3!!" << endl;
    else if (index<3) {
        //cout << "outer: size is smaller than 3!!" << endl;
        size[2] = 1;
    }
    
    D3Matrix<MatrixXd> ret(size[0], size[1], size[2]);
    
    long layerSize = size[0] * size[1];
    long colNeed = layerSize/m.rows();
    
    MatrixXd layer0 = m.leftCols(colNeed);
    layer0.resize(size[0],size[1]);
    ret.setLayer(0, layer0);
    
    if (index < 3)  return ret;
    
    int i=1;
    for (unsigned long colInd=colNeed; colInd<m.cols(); colInd+=colNeed) {
        MatrixXd temp  = m.middleCols(colInd, colNeed);
        temp.resize(size[0],size[1]);
        ret.setLayer(i++, temp);
    }
    
    return ret;
    
}

static inline long
pow2roundup (long x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}

static Eigen::MatrixXd edoSketching(Eigen::MatrixXd A, int target) {
    
    MatrixXd B = MatrixXd::Zero(target, A.cols());
    int maxEigen = target > A.cols()? target: A.cols();

    // Indicator for the position in matrix B to insert next row
    int zeroRowInd = 0;
    
    
    //cout << " Bsize :" << B.rows()<< "  col:" << B.cols()<<endl;
    //cout << " 1 " << endl;
    
    for (int i=0; i<A.rows(); i++) {
        
        //cout << " 2 " << zeroRowInd<< "  target:" << target << "  i:"<<i<<endl;
        
        B.row(zeroRowInd++) = A.row(i);
        // cout << " 22 " << endl;
        
        if (zeroRowInd == target) {

            const JacobiSVD<MatrixXd> svd(B , ComputeFullU|ComputeFullV);

            
            MatrixXd S = svd.singularValues();
            
            //cout << " 3 :" << S.cols() << endl;

            // Prevent l/2 is larger than the number of eigenvalue
            if (target/2 < maxEigen) {
                double delta = pow(S(target/2),2);
                //cout << " 4 " << endl;
                S = S.array().square() - delta;
                
                // Array<double,Dynamic,Dynamic> lessInd = S.array() >= 0;
                // S = S.array() * lessInd;
                //cout << " 5 " << endl;
                for (int i=0; i<S.cols(); i++) {
                    if (S(i) < 0) S(i) = 0;
                }
                
                S = S.array().pow(0.5);
            } else {
                cout << "Warning: Matrix_Sketching  natually eliminate" << endl;
            }
            


            MatrixXd extendS(B.rows(),B.cols());
            MatrixXd SMatrix = S.asDiagonal();
            
            //cout << " 55  S:"<< SMatrix.rows() << "  " <<  SMatrix.cols() << endl;
            
            extendS << SMatrix , MatrixXd::Zero(B.rows()-B.cols(), B.cols());
            
            //cout << " 6  S:"<< extendS.rows() << "  V:" <<  svd.matrixV().rows() << " " << svd.matrixV().cols() << endl;
            
            B = extendS * svd.matrixV().transpose();
            zeroRowInd = target/2;

        }
        
    }

    return B;
}

static void testEdoSketching() {
    
    MatrixXd basisRank50 = MatrixXd::Random(50, 50);
    for (int i=0; i<50; i++) {
        basisRank50.row(i) = basisRank50.row(i)/basisRank50.row(i).norm();
    }

    MatrixXd basisRank5 = basisRank50.middleRows(0, 5);
    MatrixXd basisRank10 = basisRank50.middleRows(0, 10);

    // 10 * 100  rank=5
    MatrixXd stackBasisRank5(10, 50);
    stackBasisRank5 << basisRank5, basisRank5;
    MatrixXd rank5_1e2 = stackBasisRank5 * MatrixXd::Random(stackBasisRank5.cols(), 100);
    // 20 * 1000  rank=10

    MatrixXd stackBasisRank10(20,50);
    stackBasisRank10 << basisRank10, basisRank10;
    MatrixXd rank10_1e3 = stackBasisRank10 * MatrixXd::Random(stackBasisRank10.cols(), 100);

    MatrixXd B1 = edoSketching(rank5_1e2.transpose(), 12);
    MatrixXd B2 = edoSketching(rank10_1e3.transpose(), 30);

    MatrixXd diff1 = rank5_1e2 *rank5_1e2.transpose() - B1.transpose()*B1;
    MatrixXd diff2 = rank10_1e3*rank10_1e3.transpose() - B2.transpose()*B2;
    
    double Error1 = diff1.array().abs().sum();
    double Error2 = diff2.array().abs().sum();
    
    cout << "Error1:" << endl << Error1 << endl;
    cout << "Error2:" << endl << Error2 << endl;
  
}