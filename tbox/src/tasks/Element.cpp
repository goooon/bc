#include "./Element.h"
#include "../inc/Vehicle.h"
#include "../inc/Config.h"
VehicleDesc::VehicleDesc()
{
	memset(this, 0, sizeof(VehicleDesc));
	//memcpy(vin, "VIN67423921      ", sizeof(vin));
	memset(vin, ' ', sizeof(vin));
	memset(tbox_serial, ' ', sizeof(tbox_serial));
	memset(imei, ' ', sizeof(imei));
	memset(iccid, ' ', sizeof(iccid));
	//memcpy(vin, Config::getInstance().vin, sizeof(vin));
	for (int i = 0; i < sizeof(vin); ++i) {
		char v = Config::getInstance().vin[i];
		if (v == 0)break;;
		vin[i] = v;
	}
	memcpy(iccid, Config::getInstance().iccid, sizeof(iccid));
	memcpy(imei, Config::getInstance().imei, sizeof(imei));
	memcpy(tbox_serial, Config::getInstance().tsn, sizeof(tbox_serial));
}

void TimeStamp::update()
{
	time_t timer;//time_t就是long int 类型
	struct tm *tblock;
	timer = time(NULL);
	tblock = localtime(&timer);
	day = tblock->tm_mday;
	hour = tblock->tm_hour;
	min = tblock->tm_min;
	month = tblock->tm_mon + 1;
	sec = tblock->tm_sec;
	year = tblock->tm_year;
}
