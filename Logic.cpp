//
//  Logic.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/9.
//  Copyright (c) 2014 Tse-Han Huang. All rights reserved.
//

#include <math.h>
#include "Logic.h"


using namespace std;
using namespace Eigen;

static int serialNum = 0;

void Logic::start(MatrixXd X, int K, double noise) {
    
    cout << endl << endl << "[LOGIC] : START";

    changeWaitState(STATE_ACTIVE);
    mTime.push_back(GetTimeMs64());
    
    IndexType p = decideP(K, X.rows());
    MatrixXd rpj = initialize(X, p);
    mTime.push_back(GetTimeMs64());

    MatrixXd basis = calculateBasis(rpj, p);
    mTime.push_back(GetTimeMs64());
    if(PRINT_MATRIX) cout << endl << endl << "[LOGIC] : BASIS="  << basis << endl << endl;
    
    MatrixXd Z = computeZ(basis);
    mTime.push_back(GetTimeMs64());
    if(PRINT_MATRIX) cout << endl << endl << "[LOGIC] : Z=" << endl <<Z << endl << endl;
    
    MatrixXd W = calculateWhiten(Z, basis, K, X.cols());
    mTime.push_back(GetTimeMs64());
    if(PRINT_MATRIX) cout << endl << endl << "[LOGIC] : W=" << endl << W << endl;
    
    D3Matrix<MatrixXd> T = buildTensor( X, W, K, noise);
    mTime.push_back(GetTimeMs64());
    
    tensorDecompose(T, W);
    mTime.push_back(GetTimeMs64());
    
    /*
    if (TIME_MEASURE) {
        cout << endl <<  endl <<"******************** TIME MEASUREMENT *******************";
        cout << endl <<  "* Time: S1 Random Projection (Distributed)\t  " << (t1-t0) <<"\t*";
        cout << endl <<  "* Time: S2 Calculate Basis (Only master)\t  " << (t2-t1) <<"\t*";
        cout << endl <<  "* Time: S3 Basis Multiplication (Distributed)\t  " << (t3-t2) <<"\t*";
        cout << endl <<  "* Time: S4 Calculate Whiten Matrix (Only master)  " << (t4-t3) <<"\t*";
        cout << endl <<  "* Time: S5 Tensor Building (Distributed)\t  " << (t5-t4) <<"\t*";
        cout << endl <<  "* Time: S6 Tensor Decomposition (Only master)\t  " << (t6-t5) <<"\t*";
        cout << endl <<  "* Time: TOTAL time consumption\t\t\t  " << (t6-t0) << "\t*";
        cout << endl <<"*********************************************************" << endl;
    }
    */
}

// Split the data and send to slaves for random projection
// parameter:X data
// parameter:nTarget    the column size we want to shrink to
MatrixXd Logic::initialize(MatrixXd X, int nTarget) {

    int nDimension = X.rows();
    int nChunk = decideChunks(X);
    
    cout << endl << "[LOGIC] : Data Dimension : " << X.rows() << " * " << X.cols();
    cout << "  Size:" << X.size()*sizeof(DataType)/(1000*1000) << "M  nChunk:" << nChunk;
    
    // Assign callback function
    Callback* callback;
    IndexType retSize[2] = {nDimension, nTarget};
    if (mDispatcher->withDistSvd()) {
        callback = new EdoLibertyCallback(retSize, nChunk, this, &Logic::initialize_cb);
    }
    else callback = new Callback(retSize, nChunk, this, &Logic::initialize_cb);
    
    // Splitting data
    vector<TaskParcel> vec;
    int blkCols = X.cols() / nChunk;
    for (int i=0; i<nChunk; i++) {
        // Record the chunk info
        if (i!=nChunk-1) mChunkVec.push_back(ChunkInfo(i*blkCols, (i+1)*blkCols));
        else mChunkVec.push_back(ChunkInfo(i*blkCols, X.cols()));

        int nCol = mChunkVec.at(i).end() - mChunkVec.at(i).start();
        IndexType size[2] = {nDimension, nCol};
        Task task(Task::INITIAL, size, nTarget, ++serialNum);
        TaskParcel tp(task, X.middleCols(i*blkCols, nCol), callback);
        vec.push_back(tp);
    }
    mDispatcher->submit(vec);

    changeWaitState(STATE_WAIT);
    cout << endl << "[LOGIC] : STATE_1 START TO WAIT";
    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mState_condition.wait(lock);
    }
    cout << endl << "[LOGIC] : STATE_1 FINISH WAITING";

    MatrixXd result = callback->result();
    delete callback;
    
    return result;
}

void Logic::initialize_cb() {
 
    changeWaitState(STATE_ACTIVE);
    mState_condition.notify_one();
}

MatrixXd Logic::calculateBasis(MatrixXd rpj, int nBasis) {
    
    cout << endl << "[LOGIC] : STATE_2 CALCULATE BASIS";
    
    const JacobiSVD<MatrixXd> svd(rpj, ComputeThinU | ComputeThinV);
    MatrixXd Ub = svd.matrixU();

    return Ub;
}

// Send basis to every slave and aggregate the result after multiplication
MatrixXd Logic::computeZ(MatrixXd basis) {

    IndexType retSize[2] = {basis.cols(), basis.cols()};
    Callback* callback = new Callback(retSize, 1, this, &Logic::computeZ_cb);
    
    // TODO: check if this is useless??
    IndexType  size[2] = {basis.cols(), basis.cols()};
    Task task(Task::BASIS_MUL, size, 0, ++serialNum);
    TaskParcel tp(task, basis, callback);
    mDispatcher->submit(tp);

    changeWaitState(STATE_WAIT);
    cout << endl << "[LOGIC] : STATE_3  START TO WAIT";
    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mState_condition.wait(lock);
    }
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

D3Matrix<MatrixXd> Logic::buildTensor(MatrixXd X, MatrixXd W, int  K, double sigma) {
    
    cout << endl << "[LOGIC] : STATE_5 BUILD TENSOR";
    
    // 1. Distribute W to every slaves
    //    Then wait for callback
    
    
    // The result will be a Flatten matrix which puts k-by-1 and k k-by-K matrices together
    //                                                E[W'X]     E[W'X (x)^3]
    // So the size will be k * (1 + k*k)
    IndexType k = W.cols();
    IndexType retSize[2] = {k, 1+k*k};
    
    // TODO 2=nprocess
    Callback* callback = new Callback(retSize, 1, this, &Logic::buildTensor_cb);
    changeWaitState(STATE_WAIT);
    
    // TODO: check if this is useless??
    IndexType size[2] = {W.rows(), W.cols()};
    Task task(Task::CAL_TENSOR, size, 0, ++serialNum);
    TaskParcel tp(task, W, callback);
    mDispatcher->submit(tp);
    
    // 2. Each slave compute E[W'x]  and E[W'x (^)3] then send back
    cout << endl << "[LOGIC] : STATE_5  START TO WAIT";
    {
        std::unique_lock<std::mutex> lock(mState_mutex);
        mState_condition.wait(lock);
    }
    cout << endl << "[LOGIC] : STATE_5  FINISH WAITING";
    
    MatrixXd result = callback->result();
    delete callback;
    
    IndexType  nData = X.cols();
    IndexType  nDimension = X.rows();

    result = result / nData;
    
    // The first column of the result is E[W'x]
    MatrixXd EWtX = result.col(0);
    
    // The rest of the result is a flattened tensor
    // Build up the tensor!
    D3Matrix<MatrixXd> EWtX3(K,K,K);
    for (int layer=0; layer<k; layer++) {
        MatrixXd slice = result.middleCols(layer*k+1, k);
        EWtX3.setLayer(layer, slice);
    }
    
    //cout << endl << "[LOGIC] : distributed EWtX:" <<EWtX<< endl;

    D3Matrix<MatrixXd> sigTensor(K,K,K);
    
    for (int i=0; i<nDimension; i++){
        MatrixXd ei = MatrixXd::Zero(nDimension,1);
        ei(i,0) = 1;
        MatrixXd WtEi = W.transpose()*ei;
        MatrixXd temp1 = outer(EWtX, WtEi).getLayer(0);
        MatrixXd temp2 = outer(WtEi, EWtX).getLayer(0);
        MatrixXd temp3 = outer(WtEi, WtEi).getLayer(0);
        sigTensor +=  outer(temp1, WtEi);
        sigTensor +=  outer(temp2, WtEi);
        sigTensor +=  outer(temp3, EWtX);
    }
    
    sigTensor = sigTensor*sigma;
    D3Matrix<MatrixXd> T = EWtX3 - sigTensor;
    
    return T;
}

void Logic::buildTensor_cb() {
    
    changeWaitState(STATE_ACTIVE);
    mState_condition.notify_one();
}

void Logic::afterWhiten(MatrixXd X, MatrixXd W, int  K, double sigma) {
    
    cout << endl << "[LOGIC] : STATE_5 AFTER WHITEN";
    
    IndexType  nData = X.cols();
    IndexType  nDimension = X.rows();
    
    // Form tensor
    D3Matrix<MatrixXd> EWtX3(K,K,K);
    for (IndexType i=0; i<nData; i++) {
        MatrixXd temp = W.transpose() * X.col(i);
        MatrixXd temp2 = outer(temp,temp).getLayer(0);
        EWtX3 += outer(temp2, temp);
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
    
    cout << endl << "[LOGIC] : real EWtX:" <<EWtX<< endl;
    
    D3Matrix<MatrixXd> sigTensor(K,K,K);
    
    for (int i=0; i<nDimension; i++){
        MatrixXd ei = MatrixXd::Zero(nDimension,1);
        ei(i,0) = 1;
        
        MatrixXd WtEi = W.transpose()*ei;
        MatrixXd temp1 = outer(EWtX,  WtEi).getLayer(0);
        MatrixXd temp2 = outer(WtEi, EWtX).getLayer(0);
        MatrixXd temp3 = outer(WtEi, WtEi).getLayer(0);
        sigTensor +=  outer(temp1, WtEi);
        sigTensor +=  outer(temp2, WtEi);
        sigTensor +=  outer(temp3, EWtX);
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
        T = current.deflate();
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
    
    vector<double> mWeight;
    
    mWeight.clear();
    for (int i=0; i<centers.size(); i++) {
        centerMatrix.col(i) = centers.at(i).col(0);
        //cout << pow(lambda.at(i),-2) <<"\t";
        mWeight.push_back(pow(lambda.at(i),-2));
    }

    if (PRINT_MATRIX) {
        cout << endl <<"TensorDecompose result =" << endl;
        cout << "lambda =" << endl << "\t";
        cout << endl <<"centers ="<<endl<< centerMatrix << endl;
    }
    
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


string Logic::getTimeDetail() {
    
    string result;
    
    result = "\n\n******************** TIME MEASUREMENT *******************";
    result = result +  "\n* Time: S1 Random Projection (Distributed)\t  " + to_string(mTime[1]-mTime[0]) +"\t*";
    result = result +  "\n* Time: S2 Calculate Basis (Only master)\t  " + to_string(mTime[2]-mTime[1]) +"\t*";
    result = result +  "\n* Time: S3 Basis Multiplication (Distributed)\t  " + to_string(mTime[3]-mTime[2]) +"\t*";
    result = result +  "\n* Time: S4 Calculate Whiten Matrix (Only master)  " + to_string(mTime[4]-mTime[3]) +"\t*";
    result = result +  "\n* Time: S5 Tensor Building (Distributed)\t  " + to_string(mTime[5]-mTime[4]) +"\t*";
    result = result +  "\n* Time: S6 Tensor Decomposition (Only master)\t  " + to_string(mTime[6]-mTime[5]) +"\t*";
    result = result +  "\n* Time: TOTAL time consumption\t\t\t  " + to_string(mTime[6]-mTime[0]) + "\t*";
    result = result +  "\n*********************************************************\n";
    
    return result;
}

// Calculate the number of chunks we should split the original data
// Round up to the nearest factor of nSlave
int Logic::decideChunks(MatrixXd X) {
    
    int nSlave = mDispatcher->nProc()-1;
    int nRawChunk = (X.size()*sizeof(DataType)/MAX_CHUNK)+1;
    
    // Round up to the factor of nSlave
    int nChunk = ceil((double)nRawChunk/nSlave) * nSlave;
    return nChunk;
}

IndexType Logic::decideP(int K, int dimension) {
    
    IndexType p = K>1000? K+100: K*1.1;
    if (p<dimension) p = dimension;
    return p;
}

