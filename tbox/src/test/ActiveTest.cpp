#include "./ActiveTest.h"
#include "../tasks/BCMessage.h"
#include "../inc/Vehicle.h"
bool ActiveTest::reqRemoteUnlock()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 3, seqID);
	msg.appendAck(1);
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("req Auth failed");
		return true;
	}
	else {
		PostEvent(AppEvent::AutoStateChanged, Vehicle::Authing, 0, 0);
	}
	return false;
}
