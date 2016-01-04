#include "pch.h"
#include "ThreadPool.h"

#ifndef fcWithTBB

class WorkerThread
{
public:
    void operator()();
};


void WorkerThread::operator()()
{
    ThreadPool &pool = ThreadPool::getInstance();
    std::function<void()> task;
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(pool.m_queue_mutex);
            while (!pool.m_stop && pool.m_tasks.empty()) {
                pool.m_condition.wait(lock);
            }
            if (pool.m_stop) { return; }

            task = pool.m_tasks.front();
            pool.m_tasks.pop_front();
        }
        task();
    }
}

ThreadPool::ThreadPool(size_t threads)
    : m_stop(false)
{
    for (size_t i = 0; i < threads; ++i) {
        m_workers.push_back(std::thread(WorkerThread()));
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    m_condition.notify_all();

    for (auto& worker : m_workers) {
        worker.join();
    }
}

ThreadPool& ThreadPool::getInstance()
{
    static ThreadPool s_instance(std::thread::hardware_concurrency());
    return s_instance;
}

void ThreadPool::enqueue(const std::function<void()> &f)
{
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_tasks.push_back(std::function<void()>(f));
    }
    m_condition.notify_one();
}



TaskGroup::TaskGroup()
    : m_active_tasks(0)
{
}

TaskGroup::~TaskGroup()
{
}

void TaskGroup::wait()
{
    ThreadPool &pool = ThreadPool::getInstance();
    while (m_active_tasks > 0)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(pool.m_queue_mutex);
            if (!pool.m_tasks.empty()) {
                task = pool.m_tasks.front();
                pool.m_tasks.pop_front();
            }
        }
        if (task) { task(); }
    }
}

#endif // fcWithTBB
