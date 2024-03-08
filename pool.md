### 编写线程池时遇到的问题

初版
```c++
class ThreadPool{
private:
    typedef function<void()> task;
    queue<task> q;
    mutex mt;
    condition_variable cnd;
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
        isClosed = true;
    }
    ThreadPool(int sz = 8): isClosed(false){
        for(int i=0;i<sz;i++){
            thread([_pool = this](){
                while(true){
                    if(_pool->empty()){
                        if(_pool->closed()) break;
                        else  _pool->wait();
                    }
                    else{
                        auto tk = _pool->pop();
                        tk();
                    }
                }
            }).detach();
        }
    }
    ~ThreadPool(){
        close();
        cnd.notify_all();
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
```
detach将主线程和工作线程分离，线程在结束运行时自行释放所占用的内存资源，但带来的问题是主线程结束后销毁线程池，导致与工作线程共享的资源被销毁，工作线程访问已经被销毁的资源，导致程序崩溃。

改进版
```c++
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
```
线城不分离，主线程析构函数等待工作线程结束后再销毁线程池，保证了工作线程访问的资源不会被销毁。

分离的正确用法
```c++
class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
            assert(threadCount > 0);
            for(size_t i = 0; i < threadCount; i++) {
                std::thread([pool = pool_] {
                    // std::cout << std::this_thread:: get_id () << std::endl;
                    while(true) {
                        std::unique_lock<std::mutex> locker(pool->mtx);
                        if(!pool->tasks.empty()) {
                            auto task = std::move(pool->tasks.front());
                            pool->tasks.pop();
                            locker.unlock();
                            task();
                            locker.lock();
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker);
                    }
                }).detach();
            }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;
    
    ~ThreadPool() {
        if(static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true;
            }
            pool_->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};
```
使用智能指针管理资源，保证所有线程结束后再销毁线程池。
