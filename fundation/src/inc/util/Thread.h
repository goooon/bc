/*******************************************************************************
 * Copyright (c) 2009, 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial implementation
 *    Ian Craggs, Allan Stockdill-Mander - async client updates
 *    Ian Craggs - fix for bug #420851
 *******************************************************************************/

#if !defined(THREAD_H)
#define THREAD_H

#if defined(WIN32) || defined(WIN64)
	#include <windows.h>
	#define thread_type HANDLE
	#define thread_id_type DWORD
	#define thread_return_type DWORD
	#define thread_fn LPTHREAD_START_ROUTINE
	#define mutex_type HANDLE
	#define cond_type HANDLE
	#define sem_type HANDLE
	typedef HANDLE   pipe_type;
#else
	#include <pthread.h>
	#include <semaphore.h>
	#define thread_type pthread_t
	#define thread_id_type pthread_t
	#define thread_return_type void*
	typedef thread_return_type (*thread_fn)(void*);
	#define mutex_type pthread_mutex_t*
	typedef struct { pthread_cond_t cond; pthread_mutex_t mutex; } cond_type_struct;
	typedef cond_type_struct *cond_type;
	typedef sem_t *sem_type;
	typedef void* pipe_type;
	cond_type Thread_create_cond();
	int Thread_signal_cond(cond_type);
	int Thread_wait_cond(cond_type condvar, int timeout);
	int Thread_destroy_cond(cond_type);
#endif

thread_type Thread_start(thread_fn, void*);
thread_id_type Thread_getid();
class Runnable
{
public:
	virtual void run() = 0;
};
class Thread : public Runnable
{
public:
	typedef thread_id_type ID;
public:
	static void startThread(Runnable* fn);
	static thread_id_type getCurrentThreadId() {
		return Thread_getid();
	}
	void start()
	{
		startThread(this);
	}
	virtual void run()override{}
private:
	thread_type thread;
};

mutex_type Thread_create_mutex();
int Thread_lock_mutex(mutex_type);
int Thread_unlock_mutex(mutex_type);
void Thread_destroy_mutex(mutex_type);

class ThreadMutex
{
public:
	enum Result
	{
		Succed = 0
	};
public:
	ThreadMutex() {
		mutex = Thread_create_mutex();
	}
	~ThreadMutex() {
		Thread_destroy_mutex(mutex);
	}
	/**
	* Lock a mutex which has already been created, block until ready
	* @param mutex the mutex
	* @return completion code, 0 is success
	*/
	Result lock() { int r = Thread_lock_mutex(mutex); return (Result)r; }
	/**
	* Unlock a mutex which has already been locked
	* @param mutex the mutex
	* @return completion code, 0 is success
	*/
	Result unlock() { int r = Thread_unlock_mutex(mutex); return (Result)r; }
private:
	mutex_type mutex;
};

class MutexLocker
{
public:
	MutexLocker(ThreadMutex& mutex):m(mutex){ mutex.lock(); }
	~MutexLocker() { m.unlock(); }
private:
	ThreadMutex& m;
};

sem_type Thread_create_sem();
int Thread_wait_sem(sem_type sem, int timeout);
int Thread_check_sem(sem_type sem);
int Thread_post_sem(sem_type sem);
int Thread_destroy_sem(sem_type sem);

class ThreadEvent
{
public:
	enum WaitResult
	{
		EventOk,
		TimeOut,
		Errors
	};
	enum PostResult
	{
		PostOk = 0
	};
public:
	ThreadEvent() {
		event = Thread_create_sem();
	}
	WaitResult wait(int timeout) {
		return (WaitResult)Thread_wait_sem(event, timeout);
	}
	PostResult post() {
		return (PostResult)Thread_post_sem(event);
	}
	bool isPosted() {
		return Thread_check_sem(event) == 1;
	}
	~ThreadEvent() { Thread_destroy_sem(event); }
private:
	sem_type event;
};

#endif
