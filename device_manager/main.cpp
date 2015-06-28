#include <iostream>
#include <stdio.h>

#include "DeviceAgent.hpp"
#include "GyroSensorDevice.hpp"
#include "DistanceSensorDevice.hpp"
#include "LightingDevice.hpp"
#include "NotificationAgentDevice.hpp"
#include "DeviceFactory.hpp"

DeviceFactory g_factory;

int main()
{
	GMainLoop *mainloop = NULL;
	DEV_AGENT->Init();

	mainloop = g_main_loop_new(NULL, FALSE);

	/*
	 * Instantiating the Notification Agent
	 */
	const char *params = "DeviceName=notificationAgent&InputSamplingRateMsec=\"1000\"&RetrievalFreq=\"ondemand\"&RetrievalMethod=\"push\"&DataStorageLocation=\"cloud\"";
	DEVICE_FACTORY_REGISTER_DEVICE(NotificationAgentDevice);
	DeviceBase *dev = DEV_AGENT->CreateNewDevice("NotificationAgent", params);

	if (NULL != dev)
	{
		cout << "Deploying notificationAgent" << endl;
		dev->DeployDevice();
	}

//	DEVICE_FACTORY_REGISTER_DEVICE(GyroSensorDevice);
//	DEVICE_FACTORY_REGISTER_DEVICE(DistanceSensorDevice);
	DEVICE_FACTORY_REGISTER_DEVICE(LightingDevice);

	g_print("starting main-loop.\n");
 	g_main_loop_run(mainloop);
	return 0;
}
