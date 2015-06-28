#include "DeviceAgent.hpp"
#include "url_utils.hpp"

using namespace std;

extern DBusGProxy *g_dataManagerObj;

static DeviceAgent *g_deviceAgent = NULL;

int g_devId = 0;
/**
 * Print out an error message and optionally quit (if fatal is TRUE)
 */
/*static void handleError(const char* msg, const char* reason,
		gboolean fatal) {
	g_printerr(PROGNAME ": ERROR: %s (%s)\n", msg, reason);
	if (fatal) {
		exit(0);
	}
}*/

int DeviceAgent::AddDevice(DeviceBase *dev)
{
	m_deviceMap.insert(make_pair<std::string&, DeviceBase *>(dev->GetDeviceName(), dev));
	printf("Added %s to deviceMap\n", dev->GetDeviceName().c_str());
	return 0;
}

int DeviceAgent::RemoveDevice(string name)
{
	m_deviceMap.erase(name);
	//TODO: Cleanly shutdown all threads from this device.

	return 0;
}

DeviceAgent *DeviceAgent::GetInstance()
{
	if (NULL == g_deviceAgent)
	{
		g_deviceAgent = new DeviceAgent();
	}

	return g_deviceAgent;
}

int DeviceAgent::DeployDevices()
{
	int cnt = 0, ret = 0;
	map<string, DeviceBase*>::const_iterator it;

	for (it = m_deviceMap.begin(); it != m_deviceMap.end(); it++)
	{
		DeviceBase *dev = it->second;
		ret = dev->DeployDevice();
		if (-1 == ret)
		{
			printf("%s could not be deployed.\n", dev->GetDeviceName().c_str());
			return -1;
		}
		cnt++;
	}

	return cnt;
}

int DeviceAgent::DeployDevice(DeviceBase *dev)
{
	if (NULL == dev)
		return -1;
	dev->DeployDevice();
	return 0;
}

int DeviceAgent::Init()
{
	string name("deviceAgent");
	GError* error = NULL;
	guint result;
	DBusGProxy *busProxy;

	g_type_init();

	m_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (error != NULL) 
	{
		printf("Error: could not create dbus system-bus\n");
		return -1;
	}

	g_print("got dbus-session-bus successfully.\n");

	busProxy = dbus_g_proxy_new_for_name(m_bus,
			DBUS_SERVICE_DBUS,
			DBUS_PATH_DBUS,
			DBUS_INTERFACE_DBUS);

	if (busProxy == NULL) 
	{
		printf("Error: could not create dbus proxy for name registration\n");
		return -1;
	}

	string service_path(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "deviceAgent";

	if (!dbus_g_proxy_call(busProxy,
				/* Method name to call. */
				"RequestName",
				/* Where to store the GError. */
				&error,
				/* Parameter type of argument 0. Note that
				   since we're dealing with GLib/D-Bus
				   wrappers, you will need to find a suitable
				   GType instead of using the "native" D-Bus
				   type codes. */
				G_TYPE_STRING,
				/* Data of argument 0. In our case, this is
				   the well-known name for our server
				   example ("org.asu.smartgateway"). */
				service_path.c_str(),
				/* Parameter type of argument 1. */
				G_TYPE_UINT,
				/* Data of argument 0. This is the "flags"
				   argument of the "RequestName" method which
				   can be use to specify what the bus service
				   should do when the name already exists on
				   the bus. We'll go with defaults. */
				0,
				/* Input arguments are terminated with a
				   special GType marker. */
				G_TYPE_INVALID,
				/* Parameter type of return lighting 0.
				   For "RequestName" it is UINT32 so we pick
				   the GType that maps into UINT32 in the
				   wrappers. */
				G_TYPE_UINT,
				/* Data of return lighting 0. These will always
				   be pointers to the locations where the
				   proxy can copy the results. */
				&result,
				/* Terminate list of return lightings. */
				G_TYPE_INVALID)) {
					printf("D-Bus.RequestName RPC failed: %s\n", error->message);
					/* Note that the whole call failed, not "just" the name
					   registration (we deal with that below). This means that
					   something bad probably has happened and there's not much we can
					   do (hence program termination). */
					return -1;
				}
	/* Check the result code of the registration RPC. */
	printf("RequestName returned %d.\n", result);
	g_print("got dbus-proxy-object for name registration.\n");

	m_obj = (DeviceAgentObject *)g_object_new(DEVICEAGENT_TYPE_OBJECT, NULL);
	if (m_obj == NULL)
	{
		g_print("Failed to create deviceAgent gobj instance.\n");
	}

	g_print("Registering DeviceAgent DbusObject on the D-Bus.\n");

	/*
	 * Now create a proxy object so that we can communicate with 
	 * the DataManager.
	 */
	service_path.assign(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "dataManager";
	string obj_path(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "dataManager";
	string iface_path(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "DataManager";

	DBusGProxy *obj = dbus_g_proxy_new_for_name(m_bus,
			service_path.c_str(), /* name */
			obj_path.c_str(), /* obj path */
			iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	cout << "DBUS Proxy for DataManager created successfully\n";
	g_dataManagerObj = obj;

	/* 
	 * The function does not return any status, so can't check for
	 * errors here. 
	 */
	dbus_g_connection_register_g_object(m_bus,
			(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+name).c_str(),
			G_OBJECT(m_obj));

	g_print("%s ready to serve requests\n", name.c_str());

	return 0;
}

DeviceBase *DeviceAgent::CreateNewDevice(const char *iface_name, const char *params)
{
	/* Construct the class name for the device */
	DeviceConf_t conf;
	GError *error = NULL;
	string devname;

	/* Set the device configuration */
	if (StringToConf(params, &conf, devname))
	{
		return NULL;
	}

	/*
	 * Now instruct the DataManager to create a new StorageSlot
	 * for this device.
	 */
	dbus_g_proxy_call (g_dataManagerObj,
			"new",
			&error,
			G_TYPE_STRING,
			params,
			G_TYPE_INVALID,
			G_TYPE_INVALID);

	string iface(iface_name);
	iface += "Device";
	DeviceBase *dev = static_cast<DeviceBase*>(g_factory.construct(iface, devname.c_str(), iface, conf));
	AddDevice(dev);


	return dev;
}

int DeviceAgent::StringToConf(const char *params, DeviceConf_t *conf, string& name)
{
	map<string, string> m;
	map<string, string>::iterator it;

	if (url_key_value_to_map(params, m))
	{
		cout << "ERROR: Could not parse create-device request\n";
		return -1;
	}

	for (it = m.begin(); it != m.end(); it++)
	{
		if (it->first == "DeviceName")
		{
			name.assign(it->second);
		}
		else if (it->first == "InputSamplingRateMsec")
		{
			conf->m_inputSamplingRateMsec = atoi(it->second.c_str()+1);
		}
		else if (it->first == "OutputGenRateMsec")
		{
			conf->m_outputGenRateMsec = atoi(it->second.c_str()+1);
		}
		else if (it->first == "RetrievalFreq")
		{
			if (it->second == "\"periodic\"")
			{
				conf->m_retrievalFreq = DEVICE_DATA_RETRIEVAL_FREQ_PERIODIC;
			}
			else if (it->second == "\"ondemand\"")
			{
				conf->m_retrievalFreq = DEVICE_DATA_RETRIEVAL_FREQ_ONDEMAND;
			}
		}
		else if (it->first == "RetrievalMethod")
		{
			if (it->second == "\"pull\"")
			{
				conf->m_retrievalMethod = DEVICE_DATA_RETRIEVAL_METHOD_PULL;
			}
			else if (it->second == "\"push\"")
			{
				conf->m_retrievalMethod = DEVICE_DATA_RETRIEVAL_METHOD_PUSH;
			}
		}
	}

	if (name.empty())
	{
		cout << "ERROR: Could not get dev name from request.\n";
		return -1;
	}

	conf->m_devId = g_devId++;

	return 0;
}

int DeviceAgent::EmitSignalDeviceUpdate(const char *message)
{
	return 0;
}

