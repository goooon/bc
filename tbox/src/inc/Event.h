#ifndef MQTT_GUARD_Event_h__
#define MQTT_GUARD_Event_h__

#include "./dep.h"
#include "./CycleQueue.h"

enum AppEvent
{
	NetConnected,
	MqttEvent,
	NetDisconnected
};

bool PostEvent(AppEvent e,u32 param1, u32 param2,void* data);

class EventQueue
{
	struct Node
	{
		AppEvent e;
		u32 param1;
		u32 param2;
		void* data;
	};
public:
	EventQueue() :events(100) {}
	bool in(AppEvent e, u32 param1, u32 param2, void* data)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Node t;
			t.data = data;
			t.e = e;
			t.param1 = param1;
			t.param2 = param2;
			bool r = events.push(t);
			mutex.unlock();
			return r;
		}
		else
		{
			DWORD e = GetLastError();
			LOG_E("mutex.lock() failed %d", e);
			return false;
		}
	}
	bool out(AppEvent& type, u32& param1, u32& param2,void*& data)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Node out;
			bool ret = events.pop(out);
			mutex.unlock();
			type = out.e;
			param1 = out.param1;
			data = out.data;
			param2 = out.param2;
			return ret;
		}
		else
		{
			DWORD e = GetLastError();
			LOG_E("mutex.lock() failed %d", e);
			return false;
		}
	}
	bool isEmpty() {
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = events.isEmpty();
			mutex.unlock();
			return r;
		}
		else
		{
			DWORD e = GetLastError();
			LOG_E("mutex.lock() failed %d", e);
			return events.isEmpty();
		}
	}
	bool isFull() {
		if (mutex.lock() == ThreadMutex::Succed)
		{
			bool r = events.isFull();
			mutex.unlock();
			return r;
		}
		else
		{
			DWORD e = GetLastError();
			LOG_E("mutex.lock() failed %d", e);
			return events.isFull();
		}
	}
private:
	CycleQueue<Node>  events;
	ThreadMutex       mutex;
};
#endif // MQTT_GUARD_Event_h__
