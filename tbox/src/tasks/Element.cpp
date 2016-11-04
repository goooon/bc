#include "./Element.h"
#include "../inc/Vehicle.h"
#include "../inc/Config.h"
VehicleDesc::VehicleDesc()
{
	memset(this, 0, sizeof(VehicleDesc));
	//memcpy(vin, "VIN67423921      ", sizeof(vin));
	memset(vin, ' ', sizeof(vin));
	//memcpy(vin, Config::getInstance().vin, sizeof(vin));
	for (int i = 0; i < sizeof(vin); ++i) {
		char v = Config::getInstance().vin[i];
		if (v == 0)break;;
		vin[i] = v;
	}
}
