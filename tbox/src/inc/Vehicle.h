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
		ActiveDoorResult,		//param2 true:succ false:falied
		DeactiveDoorResult,		//param2
		AuthIdentity,			//param2 AuthState
		Ignite,				    //���
		UnIgnt,				    //Ϩ��
		ShiftLevel,				//param2 curr level
		DoorOpened,				//param2 door id
		DoorClosed,				//param2
		WindOpened,				//param2 window id
		WindClosed
	};
	enum State {				//������ǰ״̬
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
	Apparatus& getApparatus() { return apparatus; }
	bool isParkState();
	bool isAuthed();
	bool isDriving();
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
