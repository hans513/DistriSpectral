//
//  Logic.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/9.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#include "Logic.h"


using namespace std;
using namespace Eigen;


void Logic::start(MatrixXd X, int K, double noise) {
    
    cout << endl << endl << "[LOGIC] : START";
    
    changeWaitState(STATE_ACTIVE);
    
    int nDimension = X.rows();
    
    // TODO: How to decide p??
    IndexType p = K + floor(nDimension/10)<100? floor(nDimension/10): 100;
    if (p<nDimension) p = nDimension;
    
    MatrixXd rpj = initialize(X, p);
    MatrixXd basis = calculateBasis(rpj, p);
    
    cout << endl << endl << "[LOGIC] : BASIS="  << basis << endl << endl;
    
    MatrixXd Z = computeZ(basis);
    
    cout << endl << endl << "[LOGIC] : Z=" << endl <<Z << endl << endl;
    
    MatrixXd W = calculateWhiten(Z, basis, K, X.cols());
    
    cout << endl << endl << "[LOGIC] : W=" << endl << W << endl;
    
    afterWhiten( X, W, K, noise);
    finish();
}


// Split data and send to slave for random projection
MatrixXd Logic::initialize(MatrixXd X, int nTarget) {

    // TODO: how to decide number of chunk??
    int nChunk = 4;
    int nDimension = X.rows();
    
    int blk = X.cols() / nChunk;
    for (int i=0; i<nChunk-1; i++) {
        mChunkVec.push_back(ChunkInfo(i*blk, (i+1)*blk));
    }
    mChunkVec.push_back(ChunkInfo((nChunk-1)*blk, X.cols()));

    IndexType retSize[2] = {nDimension, nTarget};
    Callback* callback = new Callback(retSize, nChunk, this, &Logic::initialize_cb);
    changeWaitState(STATE_WAIT);
    
    for (int i=0; i<nChunk; i++) {
        IndexType nCol = mChunkVec.at(i).end() - mChunkVec.at(i).start();
        IndexType size[2] = {nDimension, nCol};
        Task task(Task::INITIAL, size, nTarget);
        TaskParcel tp(task, X.middleCols(mChunkVec.at(i).start(), nCol), callback);
        mDispatcher->submit(tp);
    }
    
    cout << endl << "[LOGIC] : STATE_1  START TO WAIT";
    std::unique_lock<std::mutex> lock(mState_mutex);
    mState_condition.wait(lock);
    cout << endl << "[LOGIC] : STATE_1  FINISH WAITING";

    MatrixXd result = callback->result();
    delete callback;
    
    return result;
}

void Logic::initialize_cb() {
 
    changeWaitState(STATE_ACTIVE);
    mState_condition.notify_one();
}

// TODO: shoud I use svd instead?
MatrixXd Logic::calculateBasis(MatrixXd rpj, int nBasis) {
    
    //const JacobiSVD<MatrixXd> svd(rpj, ComputeThinU | ComputeThinV);
    //MatrixXd Ub = svd.matrixU();
    
    cout << endl << "[LOGIC] : STATE_4 CALCULATE BASIS";
    
    HouseholderQR<MatrixXd> qr(rpj);
    MatrixXd Q = MatrixXd::Identity(rpj.rows(), nBasis);
    Q = qr.householderQ() * Q;

    return Q;
}

// Send basis to every slave and aggregate the result after multiplication
MatrixXd Logic::computeZ(MatrixXd basis) {

    IndexType retSize[2] = {basis.cols(), basis.cols()};
    
    // TODO 2=nprocess
    Callback* callback = new Callback(retSize, 2, this, &Logic::computeZ_cb);
    changeWaitState(STATE_WAIT);
    
    IndexType  size[2] = {basis.cols(), basis.cols()};
    Task task(Task::BASIS_MUL, size);
    TaskParcel tp(task, basis, callback);
    mDispatcher->submit(tp);

    cout << endl << "[LOGIC] : STATE_3  START TO WAIT";
    std::unique_lock<std::mutex> lock(mState_mutex);
    mState_condition.wait(lock);
    cout << endl << "[LOGIC] : STATE_3  FINISH WAITING";
    
    MatrixXd result = callback->result();
    delete callback;
    
    return result;
}

void Logic::computeZ_cb() {

    changeWaitState(STATE_ACTIVE);
    mState_condition.notify_one();
}

MatrixXd Logic::calculateWhiten(MatrixXd bpj, MatrixXd basis, int K, IndexType  nData) {

    cout << endl << "[LOGIC] : STATE_4 CALCULATE WHITEN MATRIX";
    
    const JacobiSVD<MatrixXd> svd(bpj/nData, ComputeThinU|ComputeThinV);
    MatrixXd Ub = svd.matrixU();
    MatrixXd U = basis * Ub;

    VectorXd S = svd.singularValues();
    IndexType  nNoise = S.rows() - K;
    double sigma = S.tail(nNoise).sum()/nNoise;
    VectorXd D = S.array() - sigma;

    // When matrix size change, we need another variable to store the result
    VectorXd validD = D.head(K).array().pow(-0.5);
    MatrixXd W = U.leftCols(K) * validD.asDiagonal();

    return W;
}

void Logic::afterWhiten(MatrixXd X, MatrixXd W, int  K, double sigma) {
    
    IndexType  nData = X.cols();
    IndexType  nDimension = X.rows();
    
    // Form tensor
    D3Matrix<MatrixXd> EWtX3(K,K,K);
    for (IndexType i=0; i<nData; i++) {
        MatrixXd temp = W.transpose() * X.col(i);
        MatrixXd temp2 = outer(temp,temp)->getLayer(0);
        EWtX3 += *outer(temp2, temp);
    }
    
    //int64 t3 = GetTimeMs64();
    //if (TIME_MEASURE) cout << "Time: Tensor forming=" << (t3-t2)<< endl;
    
    // EWtX3 = EWtX3/nData;
    for (int i=0; i<EWtX3.layers(); i++) {
        MatrixXd temp = EWtX3.getLayer(i);
        temp = temp.array() / nData;
        EWtX3.setLayer(i, temp);
    }
    
    //if (DBG) cout<< endl << endl;
    //if (DBG) cout<< "EWtX3" << endl;
    //if (DBG) EWtX3.print();
    
    MatrixXd EWtX = (W.transpose()*X).rowwise().sum().array() / nData;
    
    //if (DBG) cout<< endl << endl;
    //if (DBG) cout<< "EWtX" << endl << EWtX;
    
    D3Matrix<MatrixXd> sigTensor(K,K,K);
    
    for (int i=0; i<nDimension; i++){
        MatrixXd ei = MatrixXd::Zero(nDimension,1);
        ei(i,0) = 1;
        
        MatrixXd WtEi = W.transpose()*ei;
        MatrixXd temp1 = outer(EWtX,  WtEi)->getLayer(0);
        MatrixXd temp2 = outer(WtEi, EWtX)->getLayer(0);
        MatrixXd temp3 = outer(WtEi, WtEi)->getLayer(0);
        sigTensor +=   *outer(temp1, WtEi);
        sigTensor +=   *outer(temp2, WtEi);
        sigTensor +=   *outer(temp3, EWtX);
    }
    
    sigTensor = sigTensor*sigma;
    
    //if (DBG) cout<< endl << endl;
    //if (DBG) cout<< "sigTensor" << endl;
    //if (DBG) sigTensor.print();
    
    D3Matrix<MatrixXd> T = EWtX3 - sigTensor;
    
    //if (DBG) cout<< endl << endl;
    //if (DBG) T.print();
    
    //int64 t4 = GetTimeMs64();
    //if (TIME_MEASURE) cout << "Time: Tensor calculation=" << (t4-t3)<< endl;
    
    tensorDecompose(T, W);
    
}

template <typename Derived>
void Logic::tensorDecompose(D3Matrix<Derived> T, MatrixXd W) {
    
    vector<MatrixXd> theta;
    vector<double> lambda;
    vector<MatrixXd> centers;
    
    while (lambda.size()==0  || abs(lambda.back())>0.01 ) {
        TensorPower<MatrixXd> current(T, 10, 10);
        T = *current.deflate();
        theta.push_back(current.theta());
        lambda.push_back(current.lambda());
        //cout << endl <<"theta="<<endl<<current.theta();
        //cout << endl <<"lambda="<<endl<<current.lambda();
        
        // Recover the center
        const JacobiSVD<MatrixXd> pinvSvd(W.transpose(), ComputeThinU | ComputeThinV);
        MatrixXd invTemp;
        pinvSvd.pinv(invTemp);
        MatrixXd result = current.lambda()*invTemp*current.theta();
        centers.push_back(result);
    }
    
    // Print out the result
    MatrixXd centerMatrix(W.rows(), lambda.size());
    cout << endl <<"TensorDecompose result =" << endl;
    cout << "lambda =" << endl << "\t";
    
    vector<double> mWeight;
    
    mWeight.clear();
    for (int i=0; i<centers.size(); i++) {
        centerMatrix.col(i) = centers.at(i).col(0);
        cout << pow(lambda.at(i),-2) <<"\t";
        mWeight.push_back(pow(lambda.at(i),-2));
    }
    cout << endl <<"centers ="<<endl<< centerMatrix << endl;
    
    //mData->evaluate(centerMatrix);
    //cout << endl <<"REAL centers ="<<endl<< mData->center();
    mCenters = centerMatrix;
}

void Logic::finish() {
    
    mDispatcher->terminate();
    cout << endl << "[LOGIC] : =====  FINISH  =====";
}

void Logic::changeWaitState(int state) {
    std::unique_lock<std::mutex> lock(mState_mutex);
    mWait = state;
}

