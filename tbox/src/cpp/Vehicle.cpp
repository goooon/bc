#include "../inc/Vehicle.h"
#include "../inc/Application.h"
#include "../inc/Event.h"
Vehicle& Vehicle::getInstance()
{
	return Application::getInstance().getVehicle();
}

Vehicle::Vehicle() :authed(Unauthed),state(Enabled)
{
	apparatus.reset();
}

Operation::Result Vehicle::prepareVKeyIgnition()
{
	if (!authed) {
		return Operation::E_Auth;
	}
	if (driving) {
		return Operation::E_Driving;
	}
	if (state == Ignited) {
		return Operation::E_Ignited;
	}
	if (apparatus.vehiState.door.lh_front) {
		return Operation::E_DoorOpened;
	}
	return Operation::Succ;
}

Operation::Result Vehicle::prepareActiveDoorByVKey()
{
	/*if (!authed) {
		return Operation::E_Auth;
	}*/
	/*if (driving) {
		return Operation::E_Driving;
	}
	if (apparatus.door.lh_front) {
		return Operation::E_DoorOpened;
	}*/
	return Operation::Succ;
}

Operation::Result Vehicle::reqActiveDoorByVKey()
{
	LOG_I("do reqActiveDoorByVKey()");
	PostEvent(AppEvent::AutoEvent, Vehicle::ActiveDoorResult,true,0);
	return Operation::Succ;
}

Operation::Result Vehicle::reqDeactiveDoor()
{
	LOG_I("do reqDeactiveDoor()");
	if (!apparatus.misc.door_actived)return Operation::Succ;
	PostEvent(AppEvent::AutoEvent, Vehicle::DeactiveDoorResult, true, 0);
	return Operation::Succ;
}

//Operation::Result Vehicle::reqLockDoor()
//{
//	LOG_I("do reqLockDoor()");
//	PostEvent(AppEvent::AutoEvent, Vehicle::Event::DoorDeactived, 0, 0);
//	return Operation::Succ;
//}

void Vehicle::onEvent(u32 param1, u32 param2, void* data)
{
	switch (param1) {
	case ActiveDoorResult:
		if (param2) {
			apparatus.misc.door_actived = true;
		}
		break;
	case DeactiveDoorResult:
		if (param2) {
			apparatus.misc.door_actived = false;
		}
		break;
	case DoorOpened:
		apparatus.vehiState.door.lh_front = true;
		break;
	case DoorClosed:
		apparatus.vehiState.door.lh_front = false;
		break;
	default:
		LOG_W("unhandled State %d", param1);
		break;
	}
}

bool Vehicle::changeState(State next)
{
	//todo ...
	LOG_I("Vehicle state %d -> %d", state, next);
	State prev = state;
	state = next;
	PostEvent(AppEvent::AutoStateChanged, prev, state,0);
	return false;
}
