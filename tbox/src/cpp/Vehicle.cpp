#include "../inc/Vehicle.h"
#include "../inc/Application.h"
Vehicle& Vehicle::getInstance()
{
	return Application::getInstance().getVehicle();
}

Vehicle::Vehicle() :authed(Unauthed)
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
	if (apparatus.door.lh_front) {
		return Operation::E_DoorOpened;
	}
	return Operation::Succ;
}

Operation::Result Vehicle::prepareActiveDoorByVKey()
{
	if (!authed) {
		return Operation::E_Auth;
	}
	if (driving) {
		return Operation::E_Driving;
	}
	if (apparatus.door.lh_front) {
		return Operation::E_DoorOpened;
	}
	return Operation::Succ;
}

Operation::Result Vehicle::reqActiveDoorByVKey()
{
	LOG_I("do reqActiveDoorByVKey()");
	return Operation::Succ;
}

Operation::Result Vehicle::reqDeactiveDoor()
{
	LOG_I("do reqDeactiveDoor()");
	if (!apparatus.misc.door_actived)return Operation::Succ;
	return Operation::Succ;
}

Operation::Result Vehicle::reqLockDoor()
{
	LOG_I("do reqLockDoor()");
	return Operation::Succ;
}

void Vehicle::onStateChanged(u32 param1, u32 param2, void* data)
{
	switch (param1) {
	case DoorActived:
		apparatus.misc.door_actived = true;
		break;
	case DoorDeactived:
		apparatus.misc.door_actived = false;
		break;
	default:
		LOG_W("unhandled State %d", param1);
		break;
	}
}
