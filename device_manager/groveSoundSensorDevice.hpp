#ifndef __GROVE_SOUND_SENSOR_DEVICE_HPP__
#define __GROVE_SOUND_SENSOR_DEVICE_HPP__

#include "SoundSensorDevice.hpp"

class groveSoundSensorDevice : public SoundSensorDevice
{
	private:
		string m_cur_value;
		string m_prev_value;

	protected:
		int RecvFromDevice();
		int SendToDevice();

		groveSoundSensorDevice() {}

	public:
		groveSoundSensorDevice(const string name, const string desc, DeviceConf_t conf)
			: SoundSensorDevice(name, desc, conf)
		{
		}

		~groveSoundSensorDevice() {}

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
