//
//  BlockingQueue.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/9.
//  Copyright (c) 2014年 Tse-Han Huang. All rights reserved.
//

#ifndef __DistriSpectral__BlockingQueue__
#define __DistriSpectral__BlockingQueue__

#include <iostream>
#include <queue>
#endif /* defined(__DistriSpectral__BlockingQueue__) */


template <typename T>
class BlockingQueue {

public:
    
    T pop() {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty()) {
            cond_.wait(mlock);
        }
        auto val = queue_.front();
        queue_.pop();
        return val;
    }
    
    void pop(T& item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        while (queue_.empty()) {
            cond_.wait(mlock);
        }
        item = queue_.front();
        queue_.pop();
    }
    
    void push(const T& item) {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);
        mlock.unlock();
        cond_.notify_one();
    }

    BlockingQueue()=default;
    BlockingQueue(const BlockingQueue&) = delete;            // disable copying
    BlockingQueue& operator=(const BlockingQueue&) = delete; // disable assignment
    
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};