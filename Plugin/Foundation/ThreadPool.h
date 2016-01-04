#ifndef ThreadPool_h
#define ThreadPool_h

#ifndef vsWithTBB

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class WorkerThread;
class ThreadPool;
class TaskGroup;


class ThreadPool
{
friend class WorkerThread;
friend class TaskGroup;
public:
    static ThreadPool& getInstance();
    void enqueue(const std::function<void()> &f);

private:
    ThreadPool(size_t);
    ~ThreadPool();

private:
    std::vector< std::thread > m_workers;
    std::deque< std::function<void()> > m_tasks;
    std::mutex m_queue_mutex;
    std::condition_variable m_condition;
    bool m_stop;
};



class TaskGroup
{
public:
    TaskGroup();
    ~TaskGroup();
    template<class F> void run(const F &f);
    void wait();

private:
    std::atomic<int> m_active_tasks;
};

template<class F>
void TaskGroup::run(const F &f)
{
    ++m_active_tasks;
    ThreadPool::getInstance().enqueue([this, f](){
        f();
        --m_active_tasks;
    });
}

#else // vsWithTBB

#include <tbb/tbb.h>
typedef tbb::task_group TaskGroup;

#endif // vsWithTBB

#endif // ThreadPool_h
