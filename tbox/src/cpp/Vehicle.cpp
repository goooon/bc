#include "../inc/Vehicle.h"
#include "../inc/Application.h"
#include "../inc/Event.h"
#include "../tasks/Element.h"

#undef TAG
#define TAG "Vehicle"

Vehicle& Vehicle::getInstance()
{
	return Application::getInstance().getVehicle();
}

Vehicle::Vehicle() :authed(Unauthed),state(Enabled),movingInAbnormal(false)
{
	gpsValid = false;
}

Operation::Result Vehicle::prepareVKeyIgnition(bool ready)
{
	if (ready) {
		if (!authed) {
			return Operation::E_Auth;
		}
		if (driving) {
			return Operation::E_Driving;
		}
		if (state == Ignited) {
			return Operation::E_Ignited;
		}
		//if (apparatus.vehiState.door.lh_front) {
		//	return Operation::E_DoorOpened;
		//}
		changeState(ReadyToIgnit);
	}
	else {
		changeState(NotReady);
	}
	return Operation::Succ;
}

Operation::Result Vehicle::prepareActiveDoorByVKey()
{
	if (!authed) {
		return Operation::E_Auth;
	}
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
	if (apparatus.misc.door_actived)return Operation::Succ;
	//todo send can bus command and will block this operation
	LOG_I("Blicking by can bus Active Door command ...");
	return Operation::S_Blocking;
}

Operation::Result Vehicle::reqDeactiveDoor()
{
	LOG_I("do reqDeactiveDoor()");
	if (!apparatus.misc.door_actived)return Operation::Succ;
	if (apparatus.vehiState.door.lh_front == 3 ||
		apparatus.vehiState.door.rh_front == 3 ||
		apparatus.vehiState.door.lh_rear == 3 ||
		apparatus.vehiState.door.rh_rear == 3){
		//todo...
		return Operation::E_DoorOpened;
	}
	//todo send can bus command and will block this operation
	LOG_I("Blicking by can bus Deactive Door command...");
	return Operation::S_Blocking;
}

void Vehicle::setGpsInfo(Vehicle::RawGps& loc)
{
	gpsData = loc;
}

bool Vehicle::getGpsInfo(Vehicle::RawGps& info)
{
	info = gpsData;
	return true;
}

bool Vehicle::isParkState()
{
	if (apparatus.vehiState.door.lh_front == 2 &&
		apparatus.vehiState.door.rh_front == 2 &&
		apparatus.vehiState.door.lh_rear == 2 &&
		apparatus.vehiState.door.rh_rear == 2 &&
		apparatus.vehiState.door.hood == 2 &&
		apparatus.vehiState.door.luggage_door == 2 &&

		apparatus.vehiState.window.lh_front == 2 &&
		apparatus.vehiState.window.rh_front == 2 &&
		apparatus.vehiState.window.lh_rear == 2 &&
		apparatus.vehiState.window.rh_rear == 2 &&

		apparatus.vehiState.pedal.shift_level == 1 &&
		apparatus.vehiState.pedal.parking_break == 3) {
		return true;
	}
	return false;
}

bool Vehicle::isReadyToIgnit()
{
	return state == ReadyToIgnit;
}

bool Vehicle::isAuthed()
{
	return authed == Authed;
}

bool Vehicle::isIgnited()
{
	return state == Ignited;
}

bool Vehicle::isDriving()
{
	return driving;
}

void Vehicle::setIsGpsValid(bool b)
{
	gpsValid = b;
}

bool Vehicle::isGpsValid()
{
	return gpsValid;
}

bool Vehicle::isMovingInAbnormal()
{
	return movingInAbnormal;
}

void Vehicle::setMovingInAbnormal(bool b)
{
	movingInAbnormal = b;
}

//Operation::Result Vehicle::reqLockDoor()
//{
//	LOG_I("do reqLockDoor()");
//	PostEvent(AppEvent::AutoEvent, Vehicle::Event::DoorDeactived, 0, 0);
//	return Operation::Succ;
//}

void Vehicle::onEvent(u32 param1, u32 param2, void* data)
{
	Event e = (Event)param1;
	switch (e) {
	case Vehicle::AuthIdentity:
		authed = (AuthState)param2;
		break;
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
		apparatus.vehiState.door.doors |= 1 << (param2 * 2);
		break;
	case DoorClosed:
		apparatus.vehiState.door.doors &= ~(1 << (param2 * 2));
		break;
	case WindOpened:
		apparatus.vehiState.window.winds |= 1 << (param2 * 2);
		break;
	case WindClosed:
		apparatus.vehiState.window.winds &= ~(1 << (param2 * 2));
		break;
	case Ignite:
		if (state != ReadyToIgnit) {
			LOG_W("Auto state is %d,Not Ready to Ignit",state);
			return;
		}
		if (getApparatus().vehiState.pedal.ingnition == 3) {
			LOG_W("Auto already ignited;");
		}
		else {
			getApparatus().vehiState.pedal.ingnition = 3;
			changeState(Ignited);
		}
		break;
	case UnIgnt:
		if (getApparatus().vehiState.pedal.ingnition == 2) {
			LOG_W("Auto already unignited;");
		}
		else {
			getApparatus().vehiState.pedal.ingnition = 2;
			changeState(ReadyToIgnit);
		}
		break;
	case ShiftLevel:
		if (param2 >= 5 || param1 == 0) {
			LOG_E("Unknow ShiftLevel %d", param1);
		}
		else{
			Vehicle::getInstance().getApparatus().vehiState.pedal.shift_level = param2;
		}
		break;
	case AbormalMove:
		movingInAbnormal = true;
		break;
	case NormalMove:
		movingInAbnormal = false;
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
