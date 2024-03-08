#include<iostream>
#include<mutex>
#include<queue>
#include<functional>
#include<vector>
#include<condition_variable>
#include<thread>
#include<unistd.h>
#include<ctime>
#include<cassert>
using namespace std;
class ThreadPool{
private:
    typedef function<void()> task;
    struct Pool{
        queue<task> q;
        mutex mt;
        condition_variable cnd;
        bool isClosed;
        vector<thread> vec;
    } pool;

public:
    ThreadPool(int sz = 8){
        pool.isClosed = false;
        pool.vec.resize(sz);
        for(int i=0;i<sz;i++){
            pool.vec[i] = thread([_pool = &pool](){
                while(true){
                    unique_lock<mutex> lg(_pool->mt);
                    if(!_pool->q.empty()){
                        auto t = _pool->q.front();
                        _pool->q.pop();
                        lg.unlock();
                        t();
                        lg.lock();
                    }
                    else if(_pool->isClosed) break;
                    else _pool->cnd.wait(lg);
                }
            });
        }
    }
    ~ThreadPool(){
        {
            lock_guard<mutex> lg(pool.mt);
            pool.isClosed = true;
        }
        pool.cnd.notify_all();
        for(auto &i : pool.vec){
            i.join();
        }
    }
    void add(task t){
        {
            lock_guard<mutex> lg(pool.mt);
            pool.q.push(t);
        }
        pool.cnd.notify_one();
    }
};

void f(){
    cout << "hello world" << endl;
}
int main(){
    ThreadPool pool;
    for(int i=0;i<8;i++){
        pool.add(f);
    }
}



// class ThreadPool{
// private:
//     typedef function<void()> task;
//     struct Pool{
//         queue<task> q;
//         mutex mt;
//         condition_variable cnd;
//         bool isClosed;
//         vector<thread> vec;
//     } pool;

// public:
//     ThreadPool(int sz = 8){
//         pool.isClosed = false;
//         pool.vec.resize(sz);
//         for(int i=0;i<sz;i++){
//             pool.vec[i] = thread([_pool = &pool](){
//                 while(true){
//                     unique_lock<mutex> lg(_pool->mt);
//                     if(!_pool->q.empty()){
//                         auto t = _pool->q.front();
//                         _pool->q.pop();
//                         lg.unlock();
//                         t();
//                         lg.lock();
//                     }
//                     else if(_pool->isClosed) break;
//                     else _pool->cnd.wait(lg);
//                 }
//             });
//         }
//     }
//     ~ThreadPool(){
//         {
//             lock_guard<mutex> lg(pool.mt);
//             pool.isClosed = true;
//         }
//         pool.cnd.notify_all();
//         for(auto&i : pool.vec){
//             i.join();
//         }
//     }
//     void add(task t){
//         {
//             lock_guard<mutex> lg(pool.mt);
//             pool.q.push(t);
//         }
//         pool.cnd.notify_one();
//     }
// };