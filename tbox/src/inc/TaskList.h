#ifndef GUARD_TaskList_h__
#define GUARD_TaskList_h__

#include "./Task.h"
#include "./List.h"
class TaskList
{
public:
	TaskList():listHead(0){}
	void in(Task* task){
		mutex.lock();
		if (listHead == nullptr) {
			listHead = task;
			task->prev = nullptr;
			task->next = nullptr;
		}
		else {
			task->prev = nullptr;
			task->next = listHead;
			listHead->prev = task;
			listHead = task;
		}
		mutex.unlock();
	}
	void out(Task* task) {
		mutex.lock();
		if (listHead == task) {listHead = task->next;}
		if (task->prev)task->prev->next = task->next;
		if (task->next)task->next->prev = task->prev;
		mutex.unlock();
	}
private:
	Task* listHead;
	ThreadMutex  mutex;
};
#endif // GUARD_TaskList_h__
