#ifndef __GROVE_BUZZER_DEVICE_HPP__
#define __GROVE_BUZZER_DEVICE_HPP__

#include "SoundDevice.hpp"

class groveBuzzerDevice : public SoundDevice
{
	private:
		string m_cur_value;
		string m_prev_value;

		bool m_status_on;

	protected:
		int RecvFromDevice();
		int SendToDevice();

		groveBuzzerDevice() {}

	public:
		groveBuzzerDevice(const string name, const string desc, DeviceConf_t conf)
			: SoundDevice(name, desc, conf)
		{
			m_status_on = false;
		}

		~groveBuzzerDevice() {}

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
