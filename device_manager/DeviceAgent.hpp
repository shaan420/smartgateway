#ifndef __DEVICE_AGENT_HPP__
#define __DEVICE_AGENT_HPP__

#include <pthread.h>
#include <map>
#include <dbus/dbus-glib.h>
#include "DeviceFactory.hpp"
#include "DeviceAgentObject.h"
#include "../common/include/common-defs.h"
#define DEV_AGENT DeviceAgent::GetInstance()

extern DeviceFactory g_factory;

class DeviceAgent
{
	private:
		map<std::string, DeviceBase *> m_deviceMap;

		/* The GObject representing a D-Bus connection. */
		DBusGConnection *m_bus;
		DeviceAgentObject *m_obj;

		/* Proxy DBUS object to communicate with DataManager */
		DBusGProxy *m_dataManagerObj;

		int AddDevice(DeviceBase *dev);

		int RemoveDevice(string name);

		int StringToConf(const char *params, DeviceConf_t *conf, string& name);

	public:
		DeviceAgent() {}
		~DeviceAgent() {}

#if 0
		template<class DeviceType>
		DeviceBase *CreateNewDevice(string name, string desc, DeviceConf_t& conf)
		{
			DeviceType *dev = new DeviceType(name, desc, conf);
			AddDevice(static_cast<DeviceBase*>(dev));

			g_print("Registering DbusObject on the D-Bus.\n");
			/* The function does not return any status, so can't check for
			   errors here. */
			dbus_g_connection_register_g_object(m_bus,
					(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+name).c_str(),
					G_OBJECT(dev->GetObj()));

			g_print("%s ready to serve requests\n", name.c_str());

			return dev;
		}
#endif

		DeviceBase *CreateNewDevice(const char *iface_name, const char *devname);

		DBusGConnection *GetDbusSession() const
		{
			return m_bus;
		}

		int EmitSignalDeviceUpdate(const char *message);

		int Init(void);
		int DeployDevices(void);
		int DeployDevice(DeviceBase *dev);
		static DeviceAgent *GetInstance();
		DBusGProxy *DataManagerObj()
		{
			return m_dataManagerObj;
		}
};
#endif
