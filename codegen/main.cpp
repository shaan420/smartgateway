#include <iostream>
#include <stdio.h>

#include "DeviceAgent.hpp"
#include "LightingDevice.hpp"
#include "NotificationAgentDevice.hpp"
#include "DistanceSensorDevice.hpp"
#include "GyroSensorDevice.hpp"

#include "DeviceFactory.hpp"

DeviceFactory g_factory;

int main()
{
	GMainLoop *mainloop = NULL;
	DEV_AGENT->Init();

	mainloop = g_main_loop_new(NULL, FALSE);

	DEVICE_FACTORY_REGISTER_DEVICE(LightingDevice);
DEVICE_FACTORY_REGISTER_DEVICE(NotificationAgentDevice);
DEVICE_FACTORY_REGISTER_DEVICE(DistanceSensorDevice);
DEVICE_FACTORY_REGISTER_DEVICE(GyroSensorDevice);


	g_print("starting main-loop.\n");
 	g_main_loop_run(mainloop);
	return 0;
}
