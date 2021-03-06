//
//  Header.h
//  DistriSpectral
//
//  Created by Huang, Tse-Han on 2014/6/11.
//  Copyright (c) 2014 Tse-Han Huang. All rights reserved.
//

#ifndef DistriSpectral_BlockingQueue2_h
#define DistriSpectral_BlockingQueue2_h

#include <mutex>
#include <condition_variable>
#include <deque>

#endif


template <typename T>
class BlockingQueue {
private:
    std::mutex              d_mutex;
    std::condition_variable d_condition;
    std::deque<T>           d_queue;
    
public:
    
    void push(vector<T> vec) {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            for (int i=0; i<vec.size(); i++) {
                d_queue.push_front(vec.at(i));
            }
        }
        this->d_condition.notify_one();
        
    }
    
    void push(T const& value) {
        {
            std::unique_lock<std::mutex> lock(this->d_mutex);
            d_queue.push_front(value);
        }
        this->d_condition.notify_one();
    }
    
    T pop() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        this->d_condition.wait(lock, [=]{ return !this->d_queue.empty(); });
        T rc(std::move(this->d_queue.back()));
        this->d_queue.pop_back();
        return rc;
    }
    
    void clear() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        d_queue.clear();
    }
    
    int size() {
        std::unique_lock<std::mutex> lock(this->d_mutex);
        return d_queue.size();
    }
};