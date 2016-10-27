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
		Authed,
		DoorActived,			//
		DoorDeactived,			
		DoorOpened,
		DoorClosed
	};
public:
	static Vehicle& getInstance();
	Vehicle();
	Operation::Result prepareVKeyIgnition();
	Operation::Result prepareActiveDoorByVKey();
	Operation::Result reqActiveDoorByVKey();
	Operation::Result reqDeactiveDoor();
	Operation::Result reqLockDoor();
private:
	void onStateChanged(u32 param1, u32 param2, void* data);
private:
	bool driving;
	AuthState authed;
	Apparatus apparatus;
};

#endif // GUARD_Vehicle_h__
