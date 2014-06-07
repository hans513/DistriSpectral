//
//  Master.cpp
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/6.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#include "Master.h"

void Master::run() {
    cout << "This is master";
    
    test();
}

void Master::test() {
    
    MatrixXd a(3,3);
    a << 1,2,3,4,5,6,7,8,9;
    
    int size[]={3,3};
    
    Task task(Task::MULTIPLY, size);
    
    
    //MPI_Send(size1, 2, MPI_INT, id, 1, MPI_COMM_WORLD);
    //MPI_Send(temp1.data(), temp1.size(), MPI_DOUBLE, id, 1, MPI_COMM_WORLD);
    MPI_Send(&task, sizeof(task), MPI_CHAR, 1, 1, MPI_COMM_WORLD);
    
}