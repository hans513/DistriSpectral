
// Put the file code into Eigen/src/SVD/JacobiSVD.h

void pinv( MatrixType& pinvmat) const {

    eigen_assert(m_isInitialized && "SVD is not initialized.");
    double  pinvtoler=1.e-6; // choose your tolerance wisely!
    SingularValuesType singularValues_inv=m_singularValues;

    for ( long i=0; i<m_workMatrix.cols(); ++i) {
        if ( m_singularValues(i) > pinvtoler )
            singularValues_inv(i)=1.0/m_singularValues(i);
        else singularValues_inv(i)=0;
    }
    pinvmat= (m_matrixV*singularValues_inv.asDiagonal()*m_matrixU.transpose());
}    
