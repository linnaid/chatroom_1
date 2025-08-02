// #include "ThreadSafeQueue.hpp"

// template<class T>
// void threadsafequeue<T>::Push(const T& test)
// {
//     std::lock_guard<std::mutex> lock(_mutex);
//     _queue.push(test);
//     cond.notify_one();
// }

// template<class T>
// bool threadsafequeue<T>::Pop(T& test)
// {
//     std::unique_lock<std::mutex> lock(_mutex);
//     cond.wait(lock, [&]{return !_queue.empty(); });
//     if(_queue.empty()) return false;
//     test = _queue.front();
//     _queue.pop();
//     return true;
// }


