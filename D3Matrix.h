//
//  D3Matrix.h
//  SpecGmm
//
//  Created by Huang, Tse-Han on 2014/5/9.
//  Copyright (c) 2014年 Huang, Tse-Han. All rights reserved.
//

#ifndef __SpecGmm__D3Matrix__
#define __SpecGmm__D3Matrix__

#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Core>

#endif /* defined(__SpecGmm__D3Matrix__) */

using namespace Eigen;
using namespace std;

template<typename Derived>
class D3Matrix {
    
public:
    
    D3Matrix(long row, long col, long layer) : nRow(row), nCol(col), nLayer(layer) {
        
        //layer = new MatrixXd[nLayer];
        
        for (long i=0; i<nLayer; i++) {
            mlayerVec.push_back(MatrixXd::Zero(nRow,nCol));
        }
    }
    
    ~D3Matrix() {
        
    }
    
    MatrixXd getLayer(unsigned long layer) const {
        if (layer>=nLayer) {
            cout << "\nD3<Matrix getLayer error  layer="<<layer<< "  max layer=" << nLayer << endl;
            MatrixXd zero = MatrixXd::Zero(nRow,nCol);
            return zero;
        }
        
        return mlayerVec.at(layer);
    };
    
    
    bool setLayer(unsigned long layer, MatrixBase<Derived> &matrix){
        if (layer>=nLayer) return false;
        mlayerVec.at(layer) = matrix;
        return true;
    };
    
    long rows() const {return nRow;}
    long cols() const {return nCol;}
    long layers() const {return nLayer;}
    
    void check(){
        cout<<"Good";
    };
    
    void print() {
        for (long i=0; i<nLayer; i++) {
            MatrixXd temp =mlayerVec[i];
            cout << "D3Matrix layer"<< i << "=>\n"<<  temp <<endl;
            
        }
        cout << endl;
    }
    
    
    D3Matrix& operator+=(const D3Matrix &rhs) {
        if (nLayer!=rhs.layers() || nRow!=rhs.rows() || nCol!=rhs.cols()) {
            cout << "D3Matrix, operator+ =>"<<" Dimension mismatch";
        }
        
        for (int i=0; i<nLayer; i++) {
            mlayerVec[i] += rhs.getLayer(i);
        }
        
        return *this;
    }
    
    
private:
    // Array of MatrixXd
    //MatrixXd *layer;
    
    vector<MatrixXd> mlayerVec;
    
    long nRow;
    long nCol;
    long nLayer;
    
};

template<typename Derived>
D3Matrix<Derived> operator+(D3Matrix<Derived>  &lhs, const D3Matrix<Derived> &rhs) {
    if (lhs.layers()!=rhs.layers() || lhs.rows()!=rhs.rows() || lhs.cols()!=rhs.cols()) {
        cout << "D3Matrix, operator+ =>"<<" Dimension mismatch";
    }
    
    for (int i=0; i<lhs.layers(); i++) {
        
        MatrixXd temp = lhs.getLayer(i);
        temp = temp + rhs.getLayer(i);
        lhs.setLayer(i,temp);
        
    }
    
    return lhs;
}


template<typename Derived>
D3Matrix<Derived> operator-(D3Matrix<Derived>  &lhs, const D3Matrix<Derived> &rhs) {
    if (lhs.layers()!=rhs.layers() || lhs.rows()!=rhs.rows() || lhs.cols()!=rhs.cols()) {
        cout << "D3Matrix, operator+ =>"<<" Dimension mismatch";
    }
    
    for (int i=0; i<lhs.layers(); i++) {
        MatrixXd temp = lhs.getLayer(i);
        temp = temp - rhs.getLayer(i);
        lhs.setLayer(i,temp);
    }
    
    return lhs;
}

template<typename Derived>
D3Matrix<Derived> operator*(D3Matrix<Derived>  &lhs, double scalar) {
    
    for (int i=0; i<lhs.layers(); i++) {
        MatrixXd temp = lhs.getLayer(i);
        temp = temp.array()*scalar;
        lhs.setLayer(i,temp);
    }
    
    return lhs;
}