#ifndef GUARD_TaskFifo_h__
#define GUARD_TaskFifo_h__

#include "./Task.h"
#include "./CycleQueue.h"
class TaskQueue
{
public:
	TaskQueue():tasks(100){}
	bool in(Task* t)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = tasks.push(t);
			mutex.unlock();
			return r;
		}
		else 
		{
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
			return false;
		}
	}
	Task* out()
	{ 
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Task* t = NULL;
			tasks.pop(t);
			mutex.unlock();
			return t;
		}
		else
		{
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
			return NULL;
		}
	}
	bool isEmpty() { 
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = tasks.isEmpty();
			mutex.unlock();
			return r;
		}
		else
		{
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d",e);
			return tasks.isEmpty();
		}
	}
	void abortTask(u32 applicationId) {
		if (mutex.lock() == ThreadMutex::Succed)
		{
			int i = 0;
			Task* t = tasks.getDataAt(i);
			while (t) {
				if (t->getApplicationId() == applicationId) {
					t->onEvent(AppEvent::AbortTasks, applicationId, 0, 0);
				}
				i++;
			}
			mutex.unlock();
		}
		else {
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
		}
	}
	bool isFull() {
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = tasks.isFull();
			mutex.unlock();
			return r;
		}
		else
		{
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
			return tasks.isFull();
		}
	}
private:
	CycleQueue<Task*> tasks;
	ThreadMutex       mutex;
};
#endif // GUARD_TaskFifo_h__
