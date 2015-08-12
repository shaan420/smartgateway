#ifndef __GROVE_TOUCH_SENSOR_DEVICE_HPP__
#define __GROVE_TOUCH_SENSOR_DEVICE_HPP__

#include "TouchSensorDevice.hpp"

class groveTouchSensorDevice : public TouchSensorDevice
{
	private:
		string m_cur_value;
		string m_prev_value;

	protected:
		int RecvFromDevice();
		int SendToDevice();

		groveTouchSensorDevice() {}

	public:
		groveTouchSensorDevice(const string name, const string desc, DeviceConf_t conf)
			: TouchSensorDevice(name, desc, conf)
		{
		}

		~groveTouchSensorDevice() {}

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
