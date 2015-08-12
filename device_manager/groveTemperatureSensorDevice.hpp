#ifndef __GROVE_TEMPERATURE_SENSOR_DEVICE_HPP__
#define __GROVE_TEMPERATURE_SENSOR_DEVICE_HPP__

#include "TemperatureSensorDevice.hpp"

class groveTemperatureSensorDevice : public TemperatureSensorDevice
{
	private:
		string m_cur_value;
		string m_prev_value;

	protected:
		int RecvFromDevice();
		int SendToDevice();

		groveTemperatureSensorDevice() {}

	public:
		groveTemperatureSensorDevice(const string name, const string desc, DeviceConf_t conf)
			: TemperatureSensorDevice(name, desc, conf)
		{
		}

		~groveTemperatureSensorDevice() {}

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
