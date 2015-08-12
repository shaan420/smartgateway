#ifndef __LIGHTING_DEVICE_HPP__
#define __LIGHTING_DEVICE_HPP__
#include "DeviceDbus.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "LightingObject.h"

class LightingDevice : public DeviceDbus<LightingObject, int>
{
	protected:
		int RecvFromDevice();
		int SendToDevice();

	private:
		string m_cur_value;
		string m_prev_value;

		LightingDevice() {}

	public:
		LightingDevice(const string name, const string desc, DeviceConf_t conf) 
				: DeviceDbus(LIGHTING_TYPE_OBJECT, name, desc, conf)
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
		int UpdateDataManager();
		int PrepareOutputData(string& value);
};
#endif
