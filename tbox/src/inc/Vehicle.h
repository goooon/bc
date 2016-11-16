#ifndef GUARD_Vehicle_h__
#define GUARD_Vehicle_h__
#include "./dep.h"
#include "./Operation.h"
#include "./Apparatus.h"
#include "../tasks/Element.h"
struct AutoLocation;
class Vehicle
{
	friend class Application;
public:
	struct RawGps
	{
		double longitude;
		double latitude;
		double altitude;
		double dirAngle;
		double speed;
		u32    satelliteNumber;
		RawGps() {
			longitude = 104.06f;
			latitude = 30.67f;
			altitude = 124.56f;
			dirAngle = 89.0f;
			speed = 100.0f;
			satelliteNumber = 121;
		}
	};
public:
	enum AuthState
	{
		Unauthed,
		Authing,
		Authed
	};
	enum Event {
		ActiveDoorResult,		//param2 true:succ false:falied
		DeactiveDoorResult,		//param2 true:succ false:failed
		AuthIdentity,			//param2 AuthState
		Ignite,				    //点火
		UnIgnt,				    //熄火
		ShiftLevel,				//param2 curr level
		DoorOpened,				//param2 door id
		DoorClosed,				//param2
		WindOpened,				//param2 window id
		WindClosed,
		AbormalMove,
		NormalMove
	};
	enum State {				//汽车当前状态
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
	Operation::Result prepareVKeyIgnition(bool b);
	Operation::Result prepareActiveDoorByVKey();
	Operation::Result reqActiveDoorByVKey();
	Operation::Result reqDeactiveDoor();
	Apparatus& getApparatus() { return apparatus; }
	void setGpsInfo(Vehicle::RawGps& info);
	bool getGpsInfo(Vehicle::RawGps& info);
	bool isParkState();
	bool isReadyToIgnit();
	bool isAuthed();
	bool isIgnited();
	bool isDriving();
	void setIsGpsValid(bool b);
	bool isGpsValid();
	bool isMovingInAbnormal();
	void setMovingInAbnormal(bool b);
	State getState() { return state; }
	const char* getStateString() {
		const char* desc[] = {
			"Disabled",
			"Enabled",
			"NotReady",
			"ReadyToIgnit",
			"Ignited",
			"Forwarding",
			"Backwarding"
		};
		return desc[state];
	}
private:
	void onEvent(u32 param1, u32 param2, void* data);
	bool changeState(State next);
private:
	bool driving;
	bool movingInAbnormal;
	bool gpsValid;
	AuthState authed;
	State     state;
	Apparatus apparatus;
public:
	RawGps    gpsData;
};

#endif // GUARD_Vehicle_h__
