#include "../inc/Vehicle.h"
#include "../inc/Application.h"
#include "../inc/Event.h"
Vehicle& Vehicle::getInstance()
{
	return Application::getInstance().getVehicle();
}

Vehicle::Vehicle() :authed(Unauthed),state(Enabled)
{

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

bool Vehicle::isAuthed()
{
	return authed == Authed;
}

bool Vehicle::isDriving()
{
	return driving;
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
		if (Vehicle::getInstance().getApparatus().vehiState.pedal.ingnition == 3) {
			LOG_W("Auto already ignited;");
		}
		else {
			Vehicle::getInstance().getApparatus().vehiState.pedal.ingnition = 3;
			changeState(Ignited);
		}
		break;
	case UnIgnt:
		if (Vehicle::getInstance().getApparatus().vehiState.pedal.ingnition == 2) {
			LOG_W("Auto already unignited;");
		}
		else {
			Vehicle::getInstance().getApparatus().vehiState.pedal.ingnition = 2;
			changeState(ReadyToIgnit);
		}
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
