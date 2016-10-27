#ifndef GUARD_TaskList_h__
#define GUARD_TaskList_h__

#include "./Task.h"
#include "./List.h"
class TaskList
{
public:
	TaskList():listHead(0){}
	bool in(Task* task){
		if (mutex.lock() == ThreadMutex::Succed) {
			if (listHead == NULL) {
				listHead = task;
				task->prev = NULL;
				task->next = NULL;
			}
			else {
				task->prev = NULL;
				task->next = listHead;
				listHead->prev = task;
				listHead = task;
			}
			mutex.unlock();
			return true;
		}
		else {
			LOG_E("TaskList.in failed");
			return false;
		}
	}
	bool out(Task* task) {
		if (mutex.lock() == ThreadMutex::Succed) {
			if (listHead == task) { listHead = task->next; }
			if (task->prev) { task->prev->next = task->next; }
			if (task->next) { task->next->prev = task->prev; }
			mutex.unlock();
			return true;
		}
		else {
			LOG_E("TaskList.out failed");
			return false;
		}
	}
	Task* getNextTask(Task* prev) {
		if (prev)return prev->next;
		return listHead;
	}
	void abortTask(u32 applicationID) {
		if (mutex.lock() == ThreadMutex::Succed) {
			Task* t = getNextTask(NULL);
			while (t) {
				if (t->getApplicationId() == applicationID)t->onEvent(AppEvent::AbortTasks, 0, 0, 0);
				t = getNextTask(t);
			}
			mutex.unlock();
		}
		else {
			LOG_E("abortTask failed");
		}
	}
	Task* findTask(u32 appid) {
		ThreadMutex::Result r = mutex.lock();
		if ( r == ThreadMutex::Succed) {
			Task* t = getNextTask(NULL);
			while (t) {
				if (t->getApplicationId() == appid) {
					mutex.unlock();
					return t;
				}
				t = getNextTask(t);
			}
			mutex.unlock();
		}
		else {
			LOG_E("findTask failed");
		}
		return NULL;
	}
private:
	Task* listHead;
	ThreadMutex  mutex;
};
#endif // GUARD_TaskList_h__
