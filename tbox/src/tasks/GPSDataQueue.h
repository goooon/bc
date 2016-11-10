#ifndef GUARD_GPSDataQueue_h__
#define GUARD_GPSDataQueue_h__
#include "../inc/dep.h"
#include "../inc/CycleQueue.h"
#include "./Element.h"
class GPSDataQueue
{
public:
	GPSDataQueue();
	static GPSDataQueue& getInstance();
	bool in(AutoLocation& data)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = pkgs.push(data);
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
	bool out(AutoLocation& data)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = pkgs.pop(data);
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
	AutoLocation* getNext() {
		if (mutex.lock() == ThreadMutex::Succed)
		{
			AutoLocation* r = pkgs.atPop();
			mutex.unlock();
			return r;
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
			bool r = pkgs.isEmpty();
			mutex.unlock();
			return r;
		}
		else
		{
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
			return pkgs.isEmpty();
		}
	}
	bool isFull() {
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = pkgs.isFull();
			mutex.unlock();
			return r;
		}
		else
		{
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
			return pkgs.isFull();
		}
	}
private:
	CycleQueue<AutoLocation> pkgs;
	ThreadMutex       mutex;
};

#endif // GUARD_GPSDataQueue_h__
