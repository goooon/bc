#ifndef GUARD_TaskFifo_h__
#define GUARD_TaskFifo_h__

#include "./Task.h"
#include "./CycleQueue.h"
class TaskQueue
{
public:
	TaskQueue():tasks(100){}
	bool push(Task* t) { return tasks.push(t); }
	Task* pop() { Task* t = nullptr;tasks.pop(t);return t; }
private:
	CycleQueue<Task*> tasks;
	ThreadMutex       mutex;

};
#endif // GUARD_TaskFifo_h__
