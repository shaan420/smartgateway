#include <iostream>
#include <stdio.h>

#include "DeviceAgent.hpp"
#include "LightingDevice.hpp"

int main()
{
	int ret;
	DeviceConf_t conf;
	GMainLoop *mainloop = NULL;
	DEV_AGENT->Init();

	mainloop = g_main_loop_new(NULL, FALSE);

	/* Set the device configuration */
	conf.m_devId = 1;
	conf.m_inputSamplingRateMsec = 10;
	conf.m_outputGenRateMsec = 10;
	conf.m_retreivalFreq = DEVICE_DATA_RETREIVAL_FREQ_PERIODIC;
	conf.m_retreivalMethod = DEVICE_DATA_RETREIVAL_METHOD_PULL;
	conf.m_commMethod = DEVICE_COMM_METHOD_USB;

	g_print("creating new lighting device.\n");
	DEV_AGENT->CreateNewDevice<LightingDevice>("lighting1", "Lighting Device", conf);

	g_print("deploying lighting device.\n");
	ret = DEV_AGENT->DeployDevices();
	if (-1 == ret)
	{
		printf("Error while deploying devices.\n");
		return 0;
	}

	g_print("starting main-loop.\n");
 	g_main_loop_run(mainloop);
	return 0;
}
