#ifndef __GROVE_LIGHTING_DEVICE_HPP__
#define __GROVE_LIGHTING_DEVICE_HPP__

#include "LightingDevice.hpp"

class groveLightingDevice : public LightingDevice
{
	private:
		string m_cur_value;
		string m_prev_value;

	protected:
		int RecvFromDevice();
		int SendToDevice();

		groveLightingDevice() {}

	public:
		groveLightingDevice(const string name, const string desc, DeviceConf_t conf)
			: LightingDevice(name, desc, conf)
		{
		}

		~groveLightingDevice() {}

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
