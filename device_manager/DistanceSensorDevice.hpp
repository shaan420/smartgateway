#ifndef __DISTANCE_SENSOR_DEVICE_HPP__
#define __DISTANCE_SENSOR_DEVICE_HPP__
#include "DeviceDbus.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <string.h>

#include "DistanceSensorObject.h"

class DistanceSensorDevice : public DeviceDbus<DistanceSensorObject, int>
{
	protected:
		int RecvFromDevice();
		int SendToDevice();

	private:
		string m_cur_value;
		string m_prev_value;
		struct pollfd m_poll_sensor;
		int m_echo_fd;
		int m_trigger_fd;

		DistanceSensorDevice() {}

	public:
		DistanceSensorDevice(const string name, const string desc, DeviceConf_t conf) 
				: DeviceDbus(DISTANCESENSOR_TYPE_OBJECT, name, desc, conf)
		{ 
		}

		int OnInit();
		void OnError();
		void OnChange();
		void OnSendDone();
		bool HasChanged();
		void SetDeviceParams();
		void GetDeviceParams();
		int ReadShared(void **mem);
		int Write(void *mem, int len);
		int Read(void *mem, int len);
		int PrepareOutputData(string& output);
		int UpdateDataManager();
};
#endif
