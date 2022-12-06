#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>
#include <list>
#include "locker.h"
#include <cstdio>

template<typename T>
class threadpool
{
public:
    threadpool(int num = 8, int max_num = 10000);
    ~threadpool();   
    bool append(T* request);

private:
    static void* worker(void* arg);
    void run();

private:
    
    int thread_num;
    pthread_t * m_threads;
    int max_requests;
    std::list< T*> workqueue;

    locker queueLocker;
    signal queueSem;
    bool m_stop;
};

template<typename T>
threadpool<T>::threadpool(int num, int max_num):
    thread_num(num), max_requests(max_num),
    m_stop(false), m_threads(NULL) 
    {
        if (thread_num <= 0 || max_requests <= 0)
        {
            throw std::exception();
        }

        m_threads = new pthread_t[thread_num];
        if (!m_threads) throw std::exception();

        for (int i = 0; i < thread_num; ++i)
        {
            printf("create the %d thread\n", i);
            if (pthread_create(m_threads + i, NULL, worker, this) != 0)
            {
                delete [] m_threads;
                throw std::exception();
            }

            if (pthread_detach(m_threads[i]) != 0)
            {
                delete [] m_threads;
                throw std::exception();
            }
        }
    }


template<typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_stop = true;
}

template<typename T>
bool threadpool<T>::append(T* request)
{
    queueLocker.lock();
    if (thread_num > max_requests)
    {
        queueLocker.unlock();
        return false;
    }

    workqueue.push_back(request);
    queueLocker.unlock();
    queueSem.post();
    return true;
}

template<typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool * pool = (threadpool *) arg;
    pool->run();
    return pool;
}

template<typename T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        queueSem.wait();
        queueLocker.lock();
        if (workqueue.empty())
        {
            queueLocker.unlock();
            continue;
        }

        T* request = workqueue.front();
        workqueue.pop_front();
        queueLocker.unlock();
        if (!request) continue;

        request->process();

    }
}





#endif