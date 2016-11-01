#ifndef GUARD_Vehicle_h__
#define GUARD_Vehicle_h__
#include "./dep.h"
#include "./Operation.h"
#include "./Apparatus.h"

class Vehicle
{
	friend class Application;
public:
	enum AuthState
	{
		Unauthed,
		Authing,
		Authed
	};
	enum Event {
		DoorActived,			//param2
		DoorDeactived,			//param2
		DoorOpened,				//param2
		DoorClosed				//param2
	};
	enum State {				//Æû³µµ±Ç°×´Ì¬
		Disabled,
		Enabled,
		NotReady,
		ReadyToIgnit,
		Ignited,
		Forwarding,
		Backwarding
	};
public:
	static Vehicle& getInstance();
	Vehicle();
	Operation::Result prepareVKeyIgnition();
	Operation::Result prepareActiveDoorByVKey();
	Operation::Result reqActiveDoorByVKey();
	Operation::Result reqDeactiveDoor();
private:
	void onEvent(u32 param1, u32 param2, void* data);
	bool changeState(State next);
private:
	bool driving;
	AuthState authed;
	State     state;
	Apparatus apparatus;
};

#endif // GUARD_Vehicle_h__
