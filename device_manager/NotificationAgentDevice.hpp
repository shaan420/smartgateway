#ifndef __NOTIFICATIONAGENT_DEVICE_HPP__
#define __NOTIFICATIONAGENT_DEVICE_HPP__
#include "DeviceDbus.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "NotificationAgentObject.h"
#include "Event.hpp"

class NotificationAgentDevice : public DeviceDbus<NotificationAgentObject>
{
	public:
		typedef map<string, Event*> EventMap_t;

	protected:
		int RecvFromDevice();
		int SendToDevice();

	private:
		EventMap_t m_map;
		string m_val;

		NotificationAgentDevice() {}

	public:

		NotificationAgentDevice(const string name, const string desc, DeviceConf_t conf) 
				: DeviceDbus(NOTIFICATIONAGENT_TYPE_OBJECT, name, desc, conf)
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
		int PrepareOutputData(string& value);
		int UpdateDataManager();

		/*
		 * Helper function specific to Notification Agent
		 */
		int AddEvent(const char *event);
		int AddSubscriberForEvent(const char *sub_params);
};
#endif
