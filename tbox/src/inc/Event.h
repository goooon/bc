#ifndef MQTT_GUARD_Event_h__
#define MQTT_GUARD_Event_h__

#include "./dep.h"
#include "./CycleQueue.h"

enum AppEvent
{
	NetConnected,
	MqttConnected,
	MqttDisconnected,
	NetDisconnected
};

bool PostEvent(AppEvent e,u32 param, void* data, int len);

class EventQueue
{
	struct Node
	{
		AppEvent e;
		u32 param;
		void* data;
		int len;
	};
public:
	EventQueue() :events(100) {}
	bool in(AppEvent type, u32 param, void* data, int len)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Node t;
			t.data = data;
			t.e = type;
			t.param = param;
			t.len = len;
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
	bool out(AppEvent& type, u32& param, void*& data, int& len)
	{
		if (mutex.lock() == ThreadMutex::Succed)
		{
			Node out;
			bool ret = events.pop(out);
			mutex.unlock();
			type = out.e;
			param = out.param;
			data = out.data;
			len = out.len;
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
