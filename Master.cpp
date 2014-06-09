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
    
    //test();
    cout << endl << "Master:Initial" << endl;
    test_initial();
    cout << endl << "Master:Terminate" << endl;
    terminate();
    cout << endl << "Master:Finish Terminate" << endl;
}

void Master::test() {
    
    MatrixXd a(3,3);
    a << 1,2,3,4,5,6,7,8,9;
    
    int size[]={3,3};
    
    //Task task(Task::MULTIPLY, size);
    
    
    //MPI_Send(size1, 2, MPI_INT, id, 1, MPI_COMM_WORLD);
    //MPI_Send(temp1.data(), temp1.size(), MPI_DOUBLE, id, 1, MPI_COMM_WORLD);
    //MPI_Send(&task, sizeof(task), MPI_CHAR, 1, 1, MPI_COMM_WORLD);
    
}

void Master::test_initial() {
    
    MatrixXd a(3,3);
    a << 1,2,3,4,5,6,7,8,9;
    
    int size[]={3,3};
    
    Task task(Task::INITIAL, size);
    
    MPI_Send(&task, sizeof(task), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
    MPI_Send(a.data(), a.size(), MPI_DOUBLE, 1, 1, MPI_COMM_WORLD);
    
}

void Master::terminate() {
    
    Task task(Task::TERMINATE);
    for (int id=1; id<mNumProc; id++) {
        MPI_Send(&task, sizeof(task), MPI_CHAR, id, 0, MPI_COMM_WORLD);
    }
}