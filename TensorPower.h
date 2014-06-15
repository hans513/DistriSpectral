//
//  TesnsorPower.h
//  SpecGmm
//
//  Created by Huang, Tse-Han on 2014/5/10.
//  Copyright (c) 2014年 Huang, Tse-Han. All rights reserved.
//

#ifndef __SpecGmm__TesnsorPower__
#define __SpecGmm__TesnsorPower__

#include <iostream>
#include <Eigen/Dense>
#endif /* defined(__SpecGmm__TesnsorPower__) */


#ifndef __SpecGmm__D3Matrix__
#include "D3Matrix.h"
#endif /* defined(__SpecGmm__D3Matrix__) */

/*
#ifndef __SpecGmm__Test__
#include "test.h"
#endif *//* defined(__SpecGmm__Test__) */

using namespace Eigen;


template <typename Derived>
class TensorPower {
    
public:
    
    static const bool DBG = false;
    
    MatrixXd tFunction(D3Matrix<Derived> &T, MatrixBase<Derived> &u, MatrixBase<Derived> &v, MatrixBase<Derived> &w) {
        
        // the length of the tensor
        const long dim = T.rows();
        
        if (u.rows()!= dim || v.rows()!=dim || w.rows()!=dim) {
            cout << "ERROR: tFunction: the dimension of input matrices are not correspondent to the Tensor" << endl;
            MatrixXd zero = MatrixXd::Zero(1,1);
            return zero;
        }
        
        const long s1 = u.cols(), s2 = v.cols(),  s3 = w.cols();
        
        D3Matrix<Derived> ret = D3Matrix<Derived>(s1, s2, s3);
        D3Matrix<Derived> temp = D3Matrix<Derived>(s1, s2, dim);
        
        /**
         % Treat the 3D tesor as lots of layer of 2D Matrix
         % In 2 dimension space => A(V1,V2) = V1'*A*V2
         % So it's just like deal with T(u,v) at layer j3
         */
        for (int j3=0; j3<dim; j3++) {
            
            MatrixXd a = u.transpose() * T.getLayer(j3) * v;
            temp.setLayer(j3,a);
            
            // For each column vectoer in w (usually there is only one cloumn)
            for (int wInd=0; wInd<s3; wInd++) {
                
                /**
                 % A(V1,V2) at layer j3 * w(j3)
                 % and the sum of every layer is the answer
                 */
                MatrixXd b = ret.getLayer(wInd) + temp.getLayer(j3) * w(j3,wInd);
                ret.setLayer(wInd, b);
            }
        }
        
        // Debug
        if (DBG) for (int i=0; i<ret.layers(); i++) {
            cout << "RESULT layer "<< i <<" \n" <<ret.getLayer(i);
        }
        
        if (ret.layers() != 1) {
            cout << "tFunction Error!!  We can't handle returning 3D matrix yet" << endl;
            cout << "In this algorithm, we should return only 1 layer!!" << endl;
        }
        
        return ret.getLayer(0);
        
    }
    
    void testTFuction() {
        
        MatrixXd u = MatrixXd::Identity(3,3);
        MatrixXd v = MatrixXd::Identity(3,3);
        MatrixXd w(3,1);
        w << 1,1,1;
        
        MatrixXd t1(3,3);
        MatrixXd t2(3,3);
        MatrixXd t3(3,3);
        t1 << 1,2,3,4,5,6,7,8,9;
        t2 << 10,11,12,13,14,15,16,17,18;
        t3 << 19,20,21,22,23,24,25,26,27;
        
        D3Matrix<MatrixXd> T(3,3,3);
        T.setLayer(0, t1);
        T.setLayer(1, t2);
        T.setLayer(2, t3);
        
        MatrixXd result = tFunction(T,u,v,w);
        
        MatrixXd solution(3,3);
        solution << 30,33,36,39,42,45,48,51,54;
        
        cout << "===== TEST CASE RESULT : TFunction ===== " << endl;
        cout << "Test error should be a zero matrix" << endl;
        cout << "test error = " << endl << solution-result << endl ;
    }
    
   
    void compute (D3Matrix<Derived> &T, int L, int N) {
        
        /*vvvvvvvv
         if size(T,1)~=size(T,2) || size(T,1)~=size(T,3)
         disp('ERROR')
         return;
         end
         */
        if (T.rows()!=T.cols() || T.rows()!=T.layers()) {
            cout << __FUNCTION__ << " Error!!" << endl;
            cout << "Input tensor is not in a cube shape" << endl;
        }
        
        //k = size(T,1); vvvvvvvv
        const long k = T.rows();
        
        /*
         theta = zeros(k, L, N+1);
         for tau=1:L
         
         theta_0 = randn(k,1);
         theta(:, tau, 1) = theta_0/norm(theta_0);
         
         for t=2:N+1
         preTheta = theta(:, tau, t-1);
         tensorTheta = TFunction(T, eye(k), preTheta, preTheta);
         theta(:, tau, t) = tensorTheta / norm(tensorTheta);
         end
         
         end
         */
        
        D3Matrix<Derived> theta = D3Matrix<Derived>(k, L, N+1);
        for (int tau=0; tau<L; tau++) {
            
            MatrixXd theta0 = MatrixXd::Random(k, 1);
            theta0 = theta0/theta0.norm();
            
            //MatrixXd l1Theta  = theta.getLayer(0);
            //l1Theta.col(tau) = theta0;
            
            MatrixXd temp = theta.getLayer(0);
            temp.col(tau) = theta0;
            theta.setLayer(0, temp);
            
            //theta.getLayer(0).col(tau) = theta0;
            
            
            //MatrixXd aa(3,3);
            //aa << 1,2,3,4,5,6,7,8,9;
            //aa.col(1) = theta0;
            
            //cout << "====theta03" << temp << endl;
            //cout << "====theta03" << theta.getLayer(0).col(tau) << endl;

            
            for (int t=1; t<N+1; t++) {
                MatrixXd preTheta = theta.getLayer(t-1).col(tau);
                MatrixXd eye = MatrixXd::Identity(k,k);
                MatrixXd tensorTheta = tFunction(T, eye, preTheta, preTheta);
                
               // cout << "====tensor Theta" << tensorTheta;
                MatrixXd temp = theta.getLayer(t);
                temp.col(tau) = tensorTheta / tensorTheta.norm();
                theta.setLayer(t, temp);
                
//                theta.getLayer(t).col(tau) = tensorTheta / tensorTheta.norm();
               // cout << "====colAA" << tensorTheta / tensorTheta.norm();
               // cout << "====col" << theta.getLayer(t).col(tau) <<endl;

            }
            
        }

        /*
         
         for tau=1:L
         
         cur = theta(:, tau, N+1);
         lambdaBuf(tau) = TFunction(T, cur, cur, cur);
         
         end
         
         [~, maxTau] = max(lambdaBuf);
         best = theta(:, maxTau, N+1);
         
         for i=2:N+1
         best(:,1) = TFunction(T, eye(k), best, best) / norm(TFunction(T, eye(k), best, best));
         end
         
         lambda = TFunction(T, best, best, best);
         
         deflate = T - lambda * outerProduct(outerProduct(best, best),best);
         theta = best;
         
         */
        
        double lambdaArray[L];
        
        for (int tau=0; tau<L; tau++) {
            MatrixXd cur = theta.getLayer(N).col(tau);
            
          //  cout << "cur=" << theta.getLayer(N) << endl;
            MatrixXd lambda = tFunction(T, cur, cur, cur);
            //cout << "tau=" << tau << "  "<< lambda<< endl;
            
            lambdaArray[tau] = lambda(0);
        }
        
        double max = 0;
        int maxTau = 0;
        for (int i=0; i<L; i++) {
            
            
            // cout << "i=" << i << "  "<< lambdaArray[i]<< endl;
            
            if(lambdaArray[i]>max) {
                max = lambdaArray[i];
                maxTau = i;
            }
        }
        //cout << "===== TC:" << endl << "maxTau:" << maxTau << endl;
        
        MatrixXd best = theta.getLayer(N).col(maxTau);
        
        for (int i=0; i<N; i++) {
            MatrixXd eye = MatrixXd::Identity(k,k);
            MatrixXd temp = tFunction(T, eye, best, best);
            best =  temp / temp.norm();
        }
        
        MatrixXd lambda = tFunction(T, best, best, best);
        
        bestTheta = best;
        bestLambda = lambda(0,0);
        
        MatrixXd temp = outer(bestTheta, bestTheta)->getLayer(0);
        D3Matrix<MatrixXd> tt = T - *outer(temp, bestTheta)*bestLambda;
        deflateT = &tt;
        //*deflateT = T - (*outer(temp, bestTheta)*bestLambda);
      

    }

    TensorPower (D3Matrix<Derived> &T, int L, int N) {
        
        //deflateT = new D3Matrix<MatrixXd>(T.rows(),T.cols(),T.layers());
        compute (T, L, N);
    }
    
    ~TensorPower() {
        
       // if(deflateT) delete deflateT;
    }
    
    MatrixXd theta() {
        return bestTheta;
    }
    
    double lambda() {
        return bestLambda;
    }
    
    D3Matrix<MatrixXd>* deflate() {
        return deflateT;
    }

private:
    MatrixXd bestTheta;
    double bestLambda;
    D3Matrix<MatrixXd>* deflateT;
};


