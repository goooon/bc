#ifndef GUARD_Schedule_h__
#define GUARD_Schedule_h__

#include "./dep.h"
class Task;
class Schedule
{
	friend class Application;
public:
	struct Node : BCMemory {
		Node* prev;
		Node* next;
		Timestamp fireTime;
		Task* task;
	};
	static Schedule& getInstance();
public:
	Schedule();
	void triger(Timestamp current);
	void insert(Timestamp fireTime, Task* task);
	void remove(u32 appId);
	bool remove(Task* task);
	void update(Timestamp fireTime, u32 appid);
private:
	Node nodes;
};
#endif // GUARD_Schedule_h__
