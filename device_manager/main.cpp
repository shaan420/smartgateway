#include <iostream>
#include <stdio.h>

#include "DeviceAgent.hpp"
#include "GyroSensorDevice.hpp"
#include "DistanceSensorDevice.hpp"
#include "NotificationAgentDevice.hpp"
#include "DeviceFactory.hpp"

/* Device Driver includes */
#include "groveTemperatureSensorDevice.hpp"
#include "groveLightSensorDevice.hpp"
#include "groveSoundSensorDevice.hpp"
#include "groveTouchSensorDevice.hpp"
#include "groveLightingDevice.hpp"
#include "groveBuzzerDevice.hpp"
#include "ledLightingDevice.hpp"
#include "testLightingDevice.hpp"

DeviceFactory g_factory;

int main()
{
	GMainLoop *mainloop = NULL;
	DEV_AGENT->Init();

	mainloop = g_main_loop_new(NULL, FALSE);

	/*
	 * Instantiating the Notification Agent
	 */
	const char *params = "DeviceName=notificationAgent&InputSamplingRateMsec=\"1000\"&RetrievalFreq=\"ondemand\"&RetrievalMethod=\"push\"&DataStorageLocation=\"cloud\"&DriverName=\"NotificationAgentDevice\"";
	DEVICE_FACTORY_REGISTER_DEVICE(NotificationAgentDevice);
	DeviceBase *dev = DEV_AGENT->CreateNewDevice("NotificationAgent", params);

	if (NULL != dev)
	{
		cout << "Deploying notificationAgent" << endl;
		dev->DeployDevice();
	}

//	DEVICE_FACTORY_REGISTER_DEVICE(GyroSensorDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(DistanceSensorDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(ledLightingDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(groveTemperatureSensorDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(groveSoundSensorDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(groveLightSensorDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(groveTouchSensorDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(groveBuzzerDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(groveLightingDevice);
	DEVICE_FACTORY_REGISTER_DEVICE(testLightingDevice);

	g_print("starting main-loop.\n");
 	g_main_loop_run(mainloop);
	return 0;
}
