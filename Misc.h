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