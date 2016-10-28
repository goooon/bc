#ifndef MQTT_GUARD_Event_h__
#define MQTT_GUARD_Event_h__

#include "./dep.h"
#include "./CycleQueue.h"

struct Package {
	enum Type
	{
		Mqtt,		//data is bcp_packet_t*
		Can,
		Wifi,
		Bt
	};
};
class AppEvent
{
public:
	enum Type{
		Customized = 0,

		InsertTask,				//data is Task*
		AbortTasks,				//param1 applicationID
		RemoveTask,				//data is Task*


		InsertSchedule,			//insert scheduled task,param1 is Timestamp::TimeVal_H,param2 is Timestamp::TimeVal_L,data is Task*
		UpdateSchedule,			//update scheduled task,param1 is Timestamp::TimeVal_H,param2 is Timestamp::TimeVal_L,data should be casted to applicationid
		RemoveSchedule,			//remove scheduled task,if data != null,data is Task* that shoud be removed,if data is null then param1 is applicationid

		NetStateChanged,		//param1 1:Connected,0:DisConnected

		MqttStateChanged,		//param1 is Mqtt::State prevState,param2 is Mqtt::State currState

		AutoEvent,				//param1 Vehicle::Event
		AutoStateChanged,		//param1 is Vehicle::State prevState,param2 is Vehicle::State currState

		SensorEvent,			//sensor event

		PackageArrived,			//param1 is Package::Type,data should be refferred to Package::Type
		TestEvent				//for only debug test
	};
};
//global api for application event
bool PostEvent(AppEvent::Type e, u32 param1, u32 param2, void* data);
class EventQueue
{
	struct Node : public BCMemory
	{
		AppEvent::Type e;
		u32 param1;
		u32 param2;
		void* data;
	};
public:
	EventQueue() :events(100) {}
	bool in(AppEvent::Type e, u32 param1, u32 param2, void* data)
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
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
			return false;
		}
	}
	bool out(AppEvent::Type& type, u32& param1, u32& param2,void*& data)
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
			unsigned int e = last_error();
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
			unsigned int e = last_error();
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
			unsigned int e = last_error();
			LOG_E("mutex.lock() failed %d", e);
			return events.isFull();
		}
	}
private:
	CycleQueue<Node>  events;
	ThreadMutex       mutex;
};
#endif // MQTT_GUARD_Event_h__
