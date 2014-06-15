//
//  D3Matrix.h
//  SpecGmm
//
//  Created by Huang, Tse-Han on 2014/5/9.
//  Copyright (c) 2014年 Huang, Tse-Han. All rights reserved.
//

#ifndef __SpecGmm__D3Matrix__
#define __SpecGmm__D3Matrix__

#include <iostream>
#include <Eigen/KroneckerProduct>
#include <Eigen/Dense>
#include <Eigen/Core>


#endif /* defined(__SpecGmm__D3Matrix__) */

using namespace Eigen;
using namespace std;

template<typename Derived>
class D3Matrix {
    
public:
    
    D3Matrix(long row, long col, long layer) : nRow(row), nCol(col), nLayer(layer) {

        layerPtr = new MatrixXd[nLayer];
        
        for (long i=0; i<nLayer; i++) {
            layerPtr[i] =  MatrixXd::Zero(nRow,nCol);
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
        return layerPtr[layer];
    };
    
    
    bool setLayer(unsigned long layer, MatrixBase<Derived> &matrix){
        if (layer>=nLayer) return false;
        layerPtr[layer] = matrix;
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
            MatrixXd temp =layerPtr[i];
            cout << "D3Matrix layer"<< i << "=>\n"<<  temp <<endl;
            
        }
        cout << endl;
    }
    
    
    D3Matrix& operator+=(const D3Matrix &rhs) {
        if (nLayer!=rhs.layers() || nRow!=rhs.rows() || nCol!=rhs.cols()) {
            cout << "D3Matrix, operator+ =>"<<" Dimension mismatch";
        }
        
        for (int i=0; i<nLayer; i++) {
            layerPtr[i] += rhs.getLayer(i);
        }
        
        return *this;
    }
    

private:
    // Array of MatrixXd
    MatrixXd *layerPtr;
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


template <typename Derived>
static D3Matrix<Derived>* outer(const MatrixBase<Derived> &A, MatrixBase<Derived> &B) {
    
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
    
    D3Matrix<MatrixXd> *ret = new D3Matrix<MatrixXd>(size[0], size[1], size[2]);
    
    long layerSize = size[0] * size[1];
    long colNeed = layerSize/m.rows();
    
    MatrixXd layer0 = m.leftCols(colNeed);
    layer0.resize(size[0],size[1]);
    ret->setLayer(0, layer0);
    
    if (index < 3)  return ret;
    
    int i=1;
    for (unsigned long colInd=colNeed; colInd<m.cols(); colInd+=colNeed) {
        MatrixXd temp  = m.middleCols(colInd, colNeed);
        temp.resize(size[0],size[1]);
        ret->setLayer(i++, temp);
    }
    
    return ret;
    
}