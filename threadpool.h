/**
 * @file threadpool.h
 * @brief Thread pool implementation header file
 * @details Provides a template-based thread pool for concurrent task processing.
 *          The thread pool uses a producer-consumer pattern with semaphores for
 *          synchronization and a work queue for task distribution.
 * 
 * Key Features:
 * - Template-based design for flexibility
 * - Producer-consumer pattern with work queue
 * - Semaphore-based synchronization
 * - Automatic thread lifecycle management
 * - Configurable thread count and queue size
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>
#include <list>
#include "locker.h"
#include <cstdio>

/**
 * @class threadpool
 * @brief Template-based thread pool for concurrent task processing
 * @tparam T Type of task/request to be processed by worker threads
 * @details Implements a thread pool using the producer-consumer pattern.
 *          Tasks are added to a work queue and processed by worker threads.
 *          Uses semaphores for synchronization and mutex for queue protection.
 */
template<typename T>
class threadpool
{
public:
    /**
     * @brief Constructor - Initialize thread pool
     * @param num Number of worker threads (default: 8)
     * @param max_num Maximum number of tasks in queue (default: 10000)
     * @throws std::exception if initialization fails
     * @details Creates the specified number of worker threads and initializes
     *          the work queue with the given maximum size.
     */
    threadpool(int num = 8, int max_num = 10000);
    
    /**
     * @brief Destructor - Clean up thread pool resources
     * @details Stops all worker threads and frees allocated memory
     */
    ~threadpool();   
    
    /**
     * @brief Add a task to the work queue
     * @param request Pointer to the task/request to be processed
     * @return true if task was successfully added, false otherwise
     * @details Adds the task to the work queue and signals a worker thread
     *          to process it. Returns false if queue is full.
     */
    bool append(T* request);

private:
    /**
     * @brief Worker thread function (static wrapper)
     * @param arg Pointer to the thread pool instance
     * @return Pointer to the thread pool instance
     * @details Static function that serves as the entry point for worker threads.
     *          Calls the run() method of the thread pool instance.
     */
    static void* worker(void* arg);
    
    /**
     * @brief Main worker thread loop
     * @details Continuously waits for tasks from the work queue and processes them.
     *          This method runs in each worker thread.
     */
    void run();

private:
    // Thread pool configuration
    int thread_num;         ///< Number of worker threads
    pthread_t * m_threads;  ///< Array of worker thread IDs
    int max_requests;       ///< Maximum number of tasks in work queue
    
    // Work queue and synchronization
    std::list< T*> workqueue;  ///< Queue of pending tasks/requests
    locker queueLocker;        ///< Mutex for protecting the work queue
    signal queueSem;           ///< Semaphore for signaling available tasks
    bool m_stop;               ///< Flag to stop all worker threads
};

/**
 * @brief Thread pool constructor implementation
 * @details Creates the specified number of worker threads and initializes
 *          synchronization primitives. Each worker thread runs the run() method.
 */
template<typename T>
threadpool<T>::threadpool(int num, int max_num):
    thread_num(num), max_requests(max_num),
    m_stop(false), m_threads(NULL) 
    {
        // Validate constructor parameters
        if (thread_num <= 0 || max_requests <= 0)
        {
            throw std::exception();
        }

        // Allocate thread ID array
        m_threads = new pthread_t[thread_num];
        if (!m_threads) throw std::exception();

        // Create worker threads
        for (int i = 0; i < thread_num; ++i)
        {
            printf("create the %d thread\n", i);
            if (pthread_create(m_threads + i, NULL, worker, this) != 0)
            {
                delete [] m_threads;
                throw std::exception();
            }

            // Detach threads for automatic cleanup
            if (pthread_detach(m_threads[i]) != 0)
            {
                delete [] m_threads;
                throw std::exception();
            }
        }
    }

/**
 * @brief Thread pool destructor implementation
 * @details Stops all worker threads and frees allocated memory.
 *          Worker threads will exit when m_stop becomes true.
 */
template<typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_stop = true;
}

/**
 * @brief Add task to work queue implementation
 * @details Adds the task to the work queue and signals a worker thread.
 *          Returns false if the queue is full or parameters are invalid.
 */
template<typename T>
bool threadpool<T>::append(T* request)
{
    queueLocker.lock();
    if (thread_num > max_requests)
    {
        queueLocker.unlock();
        return false;
    }

    // Add task to work queue
    workqueue.push_back(request);
    queueLocker.unlock();
    
    // Signal a worker thread that a task is available
    queueSem.post();
    return true;
}

/**
 * @brief Worker thread function implementation
 * @details Static wrapper function that calls the run() method of the thread pool.
 */
template<typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool * pool = (threadpool *) arg;
    pool->run();
    return pool;
}

/**
 * @brief Main worker thread loop implementation
 * @details Continuously waits for tasks from the work queue and processes them.
 *          Each worker thread runs this loop until the thread pool is stopped.
 */
template<typename T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        // Wait for a task to become available
        queueSem.wait();
        
        // Lock the queue to get a task
        queueLocker.lock();
        if (workqueue.empty())
        {
            queueLocker.unlock();
            continue;
        }

        // Get the next task from the queue
        T* request = workqueue.front();
        workqueue.pop_front();
        queueLocker.unlock();
        
        // Process the task if it's valid
        if (!request) continue;

        // Call the task's process method
        request->process();
    }
}

#endif