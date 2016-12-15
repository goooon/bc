#include "../inc/Vehicle.h"
#include "../inc/Application.h"
#include "../inc/Event.h"
#include "../tasks/Element.h"
#include "../inc/CanBus.h"
#include "../inc/RunTime.h"
#undef TAG
#define TAG "Vehicle"

Vehicle& Vehicle::getInstance()
{
	return Application::getInstance().getVehicle();
}

Vehicle::Vehicle() :authed(Unauthed),state(Disabled),movingInAbnormal(false)
{
	gpsValid = false;
	isVKey = false;
	seqId = (u64)(1ULL << 63) + 1;
	driving = false;
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
		//changeState(ReadyToIgnit);
	}
	else {
		//changeState(NotReady);
	}
	isVKey = true;
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
	return CanBus::getInstance().reqActiveDoor(true);
}

Operation::Result Vehicle::reqDeactiveDoor()
{
	LOG_I("do reqDeactiveDoor()");
	if (!apparatus.misc.door_actived)return Operation::Succ;
	if (apparatus.vehiState.door.lh_front == TriState::Valid_Opened ||
		apparatus.vehiState.door.rh_front == TriState::Valid_Opened ||
		apparatus.vehiState.door.lh_rear == TriState::Valid_Opened ||
		apparatus.vehiState.door.rh_rear == TriState::Valid_Opened){
		return Operation::E_DoorOpened;
	}
	return CanBus::getInstance().reqActiveDoor(false);
}

bool Vehicle::hasDoorOpened()
{
	if (apparatus.vehiState.door.lh_front == TriState::Valid_Opened ||
		apparatus.vehiState.door.rh_front == TriState::Valid_Opened ||
		apparatus.vehiState.door.lh_rear == TriState::Valid_Opened ||
		apparatus.vehiState.door.rh_rear == TriState::Valid_Opened) {
		return true;
	}
	return false;
}

bool Vehicle::isCtrlLockOpened()
{
	return apparatus.vehiState.door.ctl_lock == 3 ? true : false;
}

Operation::Result Vehicle::reqReadyToIgnition(bool b)
{
	return CanBus::getInstance().reqEnterReadyToIgnite(b);
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
	if (apparatus.vehiState.pedal.shift_type) {
		if (apparatus.vehiState.pedal.shift_level != 1)return false;
	}
	else {
		if (apparatus.vehiState.pedal.shift_level != 7)return false;
	}

	if (apparatus.vehiState.door.lh_front == TriState::Valid_Closed &&
		apparatus.vehiState.door.rh_front == TriState::Valid_Closed &&
		apparatus.vehiState.door.lh_rear == TriState::Valid_Closed &&
		apparatus.vehiState.door.rh_rear == TriState::Valid_Closed &&
		apparatus.vehiState.door.hood == TriState::Valid_Closed &&
		apparatus.vehiState.door.luggage_door == TriState::Valid_Closed &&

		apparatus.vehiState.window.lh_front == TriState::Valid_Closed &&
		apparatus.vehiState.window.rh_front == TriState::Valid_Closed &&
		apparatus.vehiState.window.lh_rear == TriState::Valid_Closed &&
		apparatus.vehiState.window.rh_rear == TriState::Valid_Closed &&
		apparatus.vehiState.window.sun_roof == 1 &&

		apparatus.vehiState.pedal.parking_break == TriState::Valid_On) {
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

void Vehicle::setAbnormalShaking(bool s)
{
	shaking = s;
}

u64 Vehicle::getTBoxSequenceId()
{
	if (seqId == 0)seqId = (u64)(1ULL << 63) + 1;
	return seqId++;
}

void Vehicle::control(u8 id, u8 arg)
{
	RunTime::getInstance().control(id, arg);
}

bool Vehicle::getControlResult(u8 id, u8& result)
{
	return RunTime::getInstance().getControlResult(id, result);
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
			changeState(Enabled);
		}
		break;
	case DeactiveDoorResult:
		if (param2) {
			apparatus.misc.door_actived = false;
		}
		break;
	case DoorOpened:
		if (!isCtrlLockOpened() && param2 != 4) {
			LOG_W("Ctrl_Lock still Shutted,but door %d oppened,something is wrong",param2);
		}
		apparatus.vehiState.door.doors |= 1 << (param2 * 2);
		changeState(NotReady);
		break;
	case DoorClosed:
		if (isCtrlLockOpened() && hasDoorOpened() && param2 == 4) {
			LOG_W("Door still Opened,can't shut Ctrl_Lock");
			break;
		}
		apparatus.vehiState.door.doors &= ~(1 << (param2 * 2));
		break;
	case WindOpened:
		if (param2 < 4) {
			apparatus.vehiState.window.winds |= 1 << (param2 * 2);
		}
		else if (param2 == 4) {
			apparatus.vehiState.window.sun_roof = 15;
		}
		break;
	case WindClosed:
		if (param2 < 4) {
			apparatus.vehiState.window.winds &= ~(1 << (param2 * 2));
		}
		else if (param2 == 4) {
			apparatus.vehiState.window.sun_roof = 1;
		}
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
			changeState(NotReady);
		}
		break;
	case ShiftType:
		getApparatus().vehiState.pedal.shift_type = param2;
		break;
	case ShiftLevel:
		getApparatus().vehiState.pedal.shift_level = param2;
		break;
	case AbormalMove:
		movingInAbnormal = true;
		break;
	case NormalMove:
		movingInAbnormal = false;
		break;
	case LeaveReadToIgnit:
		changeState(NotReady);
		break;
	case EnterReadToIgnit:
		changeState(ReadyToIgnit);
		break;
	default:
		LOG_W("unhandled State %d", param1);
		break;
	}
}

const static char* sstate[] = {
	"Disabled",
	"Enabled",
	"NotReady",
	"ReadyToIgnit",
	"Ignited",
	"Forwarding",
	"Backwarding"
};

bool Vehicle::changeState(State next)
{
	//todo ...
	//LOG_I("Vehicle state %s -> %s", sstate[state], sstate[next]);
	State prev = state;
	state = next;
	PostEvent(AppEvent::AutoStateChanged, prev, state,0);
	return false;
}
