//
//  Master.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014 Tse-Han Huang. All rights reserved.
//

# include "DataGenerator.h"

using namespace std;
using namespace Eigen;

void DataGenerator::initialize() {
    
    mCenters = MatrixXd::Random(mPara.dimension(),   mPara.cluster());
    
    for (int i=0; i<  mPara.cluster(); i++) {
        mCenters.col(i) = mCenters.col(i)/mCenters.col(i).norm();
        mCenters.col(i) *= mPara.unitRadius();
    }
    
    mX = MatrixXd::Zero(mPara.dimension(), mPara.dataPerCluster()*  mPara.cluster());
    
    random_device rd;
    default_random_engine generator(rd());
    
    for (unsigned long cluster=0; cluster<  mPara.cluster(); cluster++) {
        
        unsigned long margin = cluster * mPara.dataPerCluster();
        
        VectorXd currentCenter = mCenters.col(cluster);
        vector<normal_distribution<double> > normalVec;
        
        for (unsigned long dimension=0; dimension<mPara.dimension(); dimension ++) {
            normalVec.push_back(normal_distribution<double>(currentCenter(dimension), mPara.noise()));
        }
        
        for (unsigned long index=0; index<mPara.dataPerCluster(); index++) {
            for (unsigned long dim=0; dim<mPara.dimension(); dim++) {
                mX(dim, margin+index) = normalVec.at(dim)(generator);
            }
        }
    }
}

double DataGenerator::evaluate(MatrixXd estimate) {
    
    if (estimate.rows()!=mCenters.rows()) {
        cout << endl <<"ERROR:DataGenerator evaluate"<< endl;
        cout << "resultEvaluation dimension mismatch "<< endl;
        return 0;
    }
    
    //MatrixXd finalEstimate(mPara.dimension(),   mPara.cluster());
    unsigned long nEstimate = estimate.cols();
    
    //cout << "finalEstimate row:"<< finalEstimate.rows() << endl;
    //cout << "estimate row:"<< estimate.rows() << endl;
    //cout << "nEstimate:"<< nEstimate <<"      mPara.cluster():"<<  mPara.cluster() << endl;
    
    
    //finalEstimate.leftCols(nEstimate) = estimate;
    
    MatrixXd finalEstimate = estimate;
    
    /*
     if (nEstimate !=   mPara.cluster()) {
     unsigned long sizeDiff =   mPara.cluster() - nEstimate;
     
     if (sizeDiff>0) {
     finalEstimate.rightCols(sizeDiff) = MatrixXd::Zero(mPara.dimension(), sizeDiff);
     cout << "# of decomposed element is less than expected!! expect:" <<   mPara.cluster() << "  only:" << nEstimate << endl;
     nEstimate = nEstimate + sizeDiff;
     } else {
     cout << endl << "ERROR:DataGenerator evaluate" << endl;
     cout <<"Decompsed center is more than expected. We already pick dominant centers!!" << endl;
     
     }
     }
     */
    
    // Assign all the real center to the nearest estimater center
    
    MatrixXd bestMatch(mPara.dimension(),   mPara.cluster());
    
    double error = 0;
    
    for (unsigned long i=0; i<  mPara.cluster(); i++) {
        MatrixXd currentRep = mCenters.col(i).replicate(1,nEstimate);
        MatrixXd diff = finalEstimate - currentRep;
        diff = diff.array().pow(2);
        VectorXd dist = diff.colwise().sum();
        
        MatrixXf::Index minRow, minCol;
        error += pow(dist.minCoeff(&minRow, &minCol),0.5);
        bestMatch.col(i) = finalEstimate.col(minRow);
    }
    
    //cout << endl <<" ===== Best match ===== " << endl;
    //cout << bestMatch << endl;
    //cout << " ===== Original ===== " << endl;
    //cout << mCenters << endl;
    // cout << endl << "Avg RMSE=" << error/  mPara.cluster() << endl;
    
    return (error/  mPara.cluster());
}
