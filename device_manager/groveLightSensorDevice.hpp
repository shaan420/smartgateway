#ifndef __GROVE_LIGHT_SENSOR_DEVICE_HPP__
#define __GROVE_LIGHT_SENSOR_DEVICE_HPP__

#include "LightSensorDevice.hpp"

class groveLightSensorDevice : public LightSensorDevice
{
	private:
		string m_cur_value;
		string m_prev_value;

	protected:
		int RecvFromDevice();
		int SendToDevice();

		groveLightSensorDevice() {}

	public:
		groveLightSensorDevice(const string name, const string desc, DeviceConf_t conf)
			: LightSensorDevice(name, desc, conf)
		{
		}

		~groveLightSensorDevice() {}

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
