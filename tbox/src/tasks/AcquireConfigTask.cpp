#include "./AcquireConfigTask.h"

AcquireConfigTask::AcquireConfigTask(u32 appId) :Task(appId, true)
{

}

Task* AcquireConfigTask::Create(u32 appId)
{
	return bc_new AcquireConfigTask(appId);
}

void AcquireConfigTask::doTask()
{
	
	expire.update();
	for (;;) {
		ThreadEvent::WaitResult wr = msgQueue.wait(500);
		if (wr == ThreadEvent::TimeOut) {
			Timestamp now;
			if (expire < now) {
				reqConfig();
				expire.update(Config::getInstance().getAuthRetryInterval());
			}
		}
		else if (wr == ThreadEvent::EventOk) {
			AppEvent::Type e;
			u32 param1;
			u32 param2;
			void* data;
			if (msgQueue.out(e, param1, param2, data)) {
				if (e == AppEvent::NetStateChanged) {
					if (param1 == 0) {
						LOG_I("AcquireConfigTask Exit By Net Disconnected");
						return;
					}
				}
				else if (e == AppEvent::MqttStateChanged) {
					if (param2 == MqttClient::Disconnected) {
						LOG_I("AcquireConfigTask Exit By Mqtt Disconnected");
						return;
					}
				}
				else if (e == AppEvent::PackageArrived)
				{
					if (param1 == Package::Mqtt && data != 0)
					{
						BCPackage pkg((BCPackage*)data);
						BCMessage msg(0);
						BCMessage m = pkg.nextMessage(msg);
						if (m.getApplicationId() == APPID_ACQUIRE_CONFIG)
						{
							rspAck();
							if (m.msg) {

								BCMessage::Index idx;
								Identity at;
								if (idx = m.getFirstElement(&at)) {
									Config::getInstance().setAuthToken(at.token.dw);
								}
								else {
									LOG_E("GetConfig failed with imcomplete data");
									return;
								}
								TimeStamp ts;
								if (idx = m.getNextElement(&ts, idx)) {}
								else {
									LOG_E("GetConfig failed with imcomplete data");
									return;
								}
								ErrorElement ee;
								if (idx = m.getNextElement(&ee, idx)) {
									if (ee.errorcode == 0) {
										LOG_I("Vehicle::GetConfig with Identity 0x%x(%u) at %d-%d-%d %d:%d:%d", at.token, at.token, 1900 + ts.year, ts.month, ts.day, ts.hour, ts.min, ts.sec);
									}
									else {
										LOG_E("GetConfig failed whit errcode : %d", ee.errorcode);
									}
								}
								else {
									LOG_E("GetConfig failed with imcomplete data");
									return;
								}
								ConfigElement ce;
								if (idx = m.getNextElement(&ce, idx)){
									parseConfig(ce);
								}
								else {
									LOG_E("GetConfig failed with imcomplete data");
									return;
								}
							}
						}
						return;
					}
				}
				else {

				}
			}
		}
		else {
			LOG_E("taskMessage.wait(5000) failed %d", wr);
		}
	}
}

void AcquireConfigTask::reqConfig()
{
	BCPackage pkg;
	BCMessage msg = pkg.appendMessage(appID, 0, 0);
	msg.appendIdentity();
	msg.appendTimeStamp();

	LOG_I("reqConfig ...");
	if (!pkg.post(Config::getInstance().pub_topic, Config::getInstance().getMqttDefaultQos(), Config::getInstance().getMqttSendTimeOut())) {
		LOG_E("reqConfig failed");
	}
}

void AcquireConfigTask::parseConfig(ConfigElement& ce)
{
	ConfigElement::Node* n = 0;
	for (int i = 0; i < ce.count; ++i) {
		n = ce.getNextNode(n);
		switch (n->index)
		{
		case 1://Location Run Frequency
			if (n->arglen == 2) {
				u16 freq = Endian::toU16(&n->arg[0]);
				Config::getInstance().setGpsIntervalInDriving(freq * 1000);
			}
			else {
				LOG_E("Location Run Frequency length wrong %d", n->arglen);
			}
			break;
		case 2://Location Stopped Frequency
			if (n->arglen == 2) {
				u16 freq = Endian::toU16(&n->arg[0]);
				Config::getInstance().setIgntActivationTimeOut(freq * 1000);
			}
			else {
				LOG_E("Location Stopped Frequency length wrong %d", n->arglen);
			}
			break;
		case 3://Abnormal Moving Video Duration
			if (n->arglen == 2) {
				u16 freq = Endian::toU16(&n->arg[0]);
				Config::getInstance().setAbnormalMovingDuration(freq * 1000);
			}
			else {
				LOG_E("Abnormal Moving Video Duration length wrong %d", n->arglen);
			}
			break;
		case 4://Abnormal Moving StartTimeLimit	4	1	车辆异动开始的时间判断标准 单位：秒
			if (n->arglen == 1) {
				u8 s = n->arg[0];
				Config::getInstance().setAbnormalMovingDuration(s * 1000);
			}
			else {
				LOG_E("Abnormal Moving StartTimeLimit length wrong %d", n->arglen);
			}
			break;
		case 5://Abnormal Moving StartDistanceLimit	5	1	车辆异动开始的距离判断标准 单位：米
			if (n->arglen == 1) {
				u8 s = n->arg[0];
				Config::getInstance().setAbnormalMovingStartDistanceLimit(s);
			}
			else {
				LOG_E("Abnormal Moving StartDistanceLimit length wrong %d", n->arglen);
			}
			break;
		case 6://Abnormal Moving StopTimeLimit	6	1	车辆异动停止的时间判断标准 单位：秒
			if (n->arglen == 1) {
				u8 s = n->arg[0];
				Config::getInstance().setAbnormalMovingStopTimeLimit(s * 1000);
			}
			else {
				LOG_E("Abnormal Moving StartDistanceLimit length wrong %d", n->arglen);
			}
			break;
		case 7://Abnormal Moving StopDistanceLimit	7	1	车辆异动停止的距离判断标准 单位：米
			if (n->arglen == 1) {
				u8 s = n->arg[0];//durationEnterNormal
				Config::getInstance().setAbnormalMovingStopDistanceLimit(s);
			}
			else {
				LOG_E("Abnormal Moving StartDistanceLimit length wrong %d", n->arglen);
			}
			break;
		default:
			break;
		}
	}
}

