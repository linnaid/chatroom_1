#pragma once

#include "include.h"

template <class T>
class threadsafequeue{
public:
    void Push(const T& test){
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(test);
        cond.notify_one();
    }
    bool Pop(T& test){
        std::unique_lock<std::mutex> lock(_mutex);
        cond.wait(lock, [&]{return !_queue.empty(); });
        if(_queue.empty()) return false;
        test = _queue.front();
        _queue.pop();
        return true;
    }

private:

    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable cond;
};