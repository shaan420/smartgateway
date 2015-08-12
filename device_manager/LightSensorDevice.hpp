#ifndef __LIGHT_SENSOR_DEVICE_HPP__
#define __LIGHT_SENSOR_DEVICE_HPP__
#include "DeviceDbus.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "LightSensorObject.h"

class LightSensorDevice : public DeviceDbus<LightSensorObject>
{
	protected:
		virtual int RecvFromDevice() {return 0;}
		virtual int SendToDevice() {return 0;}

	public:
		LightSensorDevice(const string name, const string desc, DeviceConf_t conf) 
				: DeviceDbus(LIGHTSENSOR_TYPE_OBJECT, name, desc, conf)
		{ 
		}
		
		LightSensorDevice() {}

		virtual int OnInit() {return 0;}
		virtual void OnError() {}
		virtual void OnChange() {}
		virtual void OnSendDone() {}
		virtual bool HasChanged() {return 0;}
		virtual void SetDeviceParams() {}
		virtual void GetDeviceParams() {}
		virtual int ReadShared(void **mem) {return 0;}
		virtual int Write(void *mem, int len) {return 0;}
		virtual int Read(void *mem, int len) {return 0;}
		virtual int UpdateDataManager() {return 0;}
		virtual int PrepareOutputData(string& value) {return 0;}
};
#endif
