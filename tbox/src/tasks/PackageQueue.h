#ifndef GUARD_PackageQueue_h__
#define GUARD_PackageQueue_h__
#include "../inc/dep.h"
#include "../inc/CycleQueue.h"
class PackageQueue
{
	struct Node
	{
		Node(){}
		Node(u8* b,u32 l):buf(b),len(l){}
		u8* buf;
		u32 len;
	};
public:
	PackageQueue() :pkgs(200) {}
	static PackageQueue& getInstance();
	bool in(u8* buf,u32 len)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Node node(buf,len);
			bool r = pkgs.push(node);
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
	bool out(u8*& buf,u32& len)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Node n;
			bool r = pkgs.pop(n);
			buf = n.buf;
			len = n.len;
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
	u8* getNext(u32& len) {
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Node n;
			bool r = pkgs.pop(n);
			u8* buf = r ? n.buf : 0;
			len = n.len;
			mutex.unlock();
			return buf;
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
	CycleQueue<Node> pkgs;
	ThreadMutex       mutex;
};
#endif // GUARD_PackageQueue_h__
