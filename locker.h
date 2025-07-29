/**
 * @file locker.h
 * @brief Synchronization primitives header file
 * @details Provides RAII wrapper classes for POSIX synchronization primitives:
 *          mutex, condition variable, and semaphore. These classes ensure
 *          proper resource management and provide a clean interface for
 *          thread synchronization.
 * 
 * Key Features:
 * - RAII design for automatic resource management
 * - Exception-safe initialization and cleanup
 * - Clean, consistent interface across all primitives
 * - Thread-safe operations with proper error handling
 */

#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>

/**
 * @class locker
 * @brief RAII wrapper for POSIX mutex
 * @details Provides automatic initialization and cleanup of pthread_mutex_t.
 *          Ensures thread-safe access to shared resources with proper
 *          resource management.
 */
class locker
{
public:
    /**
     * @brief Constructor - Initialize mutex
     * @throws std::exception if mutex initialization fails
     * @details Creates and initializes a POSIX mutex with default attributes.
     */
    locker() 
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }

    /**
     * @brief Destructor - Clean up mutex
     * @details Automatically destroys the mutex to prevent resource leaks.
     */
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    /**
     * @brief Lock the mutex
     * @return true if lock was acquired successfully, false otherwise
     * @details Blocks until the mutex can be acquired. Thread-safe operation.
     */
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    /**
     * @brief Unlock the mutex
     * @return true if unlock was successful, false otherwise
     * @details Releases the mutex lock. Should only be called by the thread
     *          that currently holds the lock.
     */
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    /**
     * @brief Get raw mutex pointer
     * @return Pointer to the underlying pthread_mutex_t
     * @details Provides access to the raw mutex for use with condition variables.
     */
    pthread_mutex_t * get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;  ///< The underlying POSIX mutex
};

/**
 * @class cond
 * @brief RAII wrapper for POSIX condition variable
 * @details Provides automatic initialization and cleanup of pthread_cond_t.
 *          Used in conjunction with mutexes for thread synchronization
 *          and signaling.
 */
class cond
{
public:
    /**
     * @brief Constructor - Initialize condition variable
     * @throws std::exception if condition variable initialization fails
     * @details Creates and initializes a POSIX condition variable with default attributes.
     */
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            throw std:: exception();
        }
    }

    /**
     * @brief Wait on condition variable
     * @param mutex Pointer to the associated mutex
     * @return true if wait was successful, false otherwise
     * @details Atomically releases the mutex and waits for the condition.
     *          The mutex is re-acquired when the function returns.
     */
    bool wait(pthread_mutex_t * mutex)
    {
        return pthread_cond_wait(&m_cond, mutex) == 0;
    }

    /**
     * @brief Timed wait on condition variable
     * @param mutex Pointer to the associated mutex
     * @param time Timeout specification
     * @return true if wait was successful, false on timeout or error
     * @details Atomically releases the mutex and waits for the condition
     *          with a timeout. The mutex is re-acquired when the function returns.
     */
    bool timedwait(pthread_mutex_t * mutex, struct timespec time)
    {
        return pthread_cond_timedwait(&m_cond, mutex, &time) == 0;
    }

    /**
     * @brief Signal one waiting thread
     * @return true if signal was successful, false otherwise
     * @details Wakes up one thread waiting on this condition variable.
     */
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

    /**
     * @brief Broadcast to all waiting threads
     * @return true if broadcast was successful, false otherwise
     * @details Wakes up all threads waiting on this condition variable.
     */
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

    /**
     * @brief Destructor - Clean up condition variable
     * @details Automatically destroys the condition variable to prevent resource leaks.
     */
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }

private:
    pthread_cond_t m_cond;  ///< The underlying POSIX condition variable
};

/**
 * @class signal
 * @brief RAII wrapper for POSIX semaphore
 * @details Provides automatic initialization and cleanup of sem_t.
 *          Used for counting semaphores and producer-consumer synchronization.
 */
class signal 
{
public:
    /**
     * @brief Constructor - Initialize semaphore with value 0
     * @throws std::exception if semaphore initialization fails
     * @details Creates and initializes a POSIX semaphore with initial value 0.
     */
    signal()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std:: exception();
        }
    }

    /**
     * @brief Constructor - Initialize semaphore with specified value
     * @param num Initial value for the semaphore
     * @throws std::exception if semaphore initialization fails
     * @details Creates and initializes a POSIX semaphore with the specified initial value.
     */
    signal(int num)
    { 
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std:: exception();
        }
    }

    /**
     * @brief Destructor - Clean up semaphore
     * @details Automatically destroys the semaphore to prevent resource leaks.
     */
    ~signal()
    {
        sem_destroy(&m_sem);
    }

    /**
     * @brief Wait (decrement) semaphore
     * @return true if wait was successful, false otherwise
     * @details Decrements the semaphore value. If the value is zero, blocks until
     *          the semaphore can be decremented.
     */
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }

    /**
     * @brief Post (increment) semaphore
     * @return true if post was successful, false otherwise
     * @details Increments the semaphore value and wakes up any threads waiting on it.
     */
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;  ///< The underlying POSIX semaphore
};

#endif