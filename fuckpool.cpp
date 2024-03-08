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
    queue<task> q;
    mutex mt;
    condition_variable cnd;
    vector<thread> vec;
    bool isClosed;
public:
    bool empty(){
        lock_guard<mutex> lk(mt);
        return q.empty();
    }
    task pop(){
        lock_guard<mutex> lk(mt);
        auto tk = q.front();
        q.pop();
        return tk;
    }
    void wait(){
        unique_lock<mutex> lk(mt);
        cnd.wait(lk);
    }
    void add(task tk){
        {
            lock_guard<mutex> lk(mt);
            q.push(tk);
        }
        cnd.notify_one();
    }
    bool closed(){
        lock_guard<mutex> lk(mt);
        return isClosed;
    }
    void close(){
        lock_guard<mutex> lk(mt);
        cout << "closed" << endl;
        isClosed = true;
    }
    ThreadPool(int sz = 8): vec(sz), isClosed(false){
        for(int i=0;i<sz;i++){
            vec[i] = thread([_pool = this](){
                while(true){
                    if(_pool->empty()){
                        if(_pool->closed()) break;
                        else  _pool->wait();
                    }
                    // else if(_pool->closed()){
                    //     break;
                    // }
                    else{
                        auto tk = _pool->pop();
                        tk();
                    }
                }
            });
            vec[i].detach();
        }
    }
    ~ThreadPool(){
        cnd.notify_all();
    }
};

void f(){
    cout << "hello world" << endl;
}
int main(){
    ThreadPool pool;
    for(int i=0;i<8;i++){
        // pool.AddTask(f);
        pool.add(f);
    }
    // sleep(1);
    // pool.closed();
}