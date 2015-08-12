#ifndef __DEVICE_AGENT_HPP__
#define __DEVICE_AGENT_HPP__

#include <pthread.h>
#include <map>
#include <dbus/dbus-glib.h>
#include <boost/lockfree/spsc_queue.hpp>
#include "DeviceFactory.hpp"
#include "DeviceAgentObject.h"
#include "../common/include/common-defs.h"
#define DEV_AGENT DeviceAgent::GetInstance()

extern DeviceFactory g_factory;

typedef enum DbusDataManagerAction
{
	DBUS_DATA_MANAGER_ACTION_NEW = 0,
	DBUS_DATA_MANAGER_ACTION_INSERT
} DbusDataManagerAction_t;

typedef struct DbusAsyncMsg
{
	DbusDataManagerAction_t action;
	char *params;
} DbusAsyncMsg_t;


class DeviceAgent
{
	public:
		typedef boost::lockfree::spsc_queue<DbusAsyncMsg *, boost::lockfree::capacity<1024> > DbusOffloadQueue_t;

	private:
		GMutex *m_deviceMapLock;
		map<std::string, DeviceBase *> m_deviceMap;

		/* The GObject representing a D-Bus connection. */
		DBusGConnection *m_bus;
		DeviceAgentObject *m_obj;

		/* Proxy DBUS object to communicate with DataManager */
		DBusGProxy *m_dataManagerObj;

		DbusOffloadQueue_t m_dbus_offload_queue;
		GThread *m_dbus_offload_th;

	
		int RemoveDevice(string name);


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

		void UpdateDataManager(const char *, char *);
		void SendToDataManager(DbusAsyncMsg_t *);

		DeviceBase *CreateNewDevice(const char *iface_name, const char *devname);
		int CreateNewDeviceAsync(const char *iface_name, char *params);

		DBusGConnection *GetDbusSession() const
		{
			return m_bus;
		}

		int EmitSignalDeviceUpdate(const char *message);

		int Init(void);
		int DeployDevices(void);
		int StringToConf(const char *params, DeviceConf_t *conf, string& name);
		int AddDevice(DeviceBase *dev);
		int DeployDevice(DeviceBase *dev);
		static DeviceAgent *GetInstance();
		DBusGProxy *DataManagerObj()
		{
			return m_dataManagerObj;
		}
};
#endif
