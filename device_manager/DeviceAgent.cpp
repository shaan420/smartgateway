#include "DeviceAgent.hpp"
#include "url_utils.hpp"
#include <algorithm>

using namespace std;

static DeviceAgent *g_deviceAgent = NULL;

volatile int g_devId = 0;

int g_cnt;

pthread_mutex_t g_dbus_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_dbus_cond = PTHREAD_COND_INITIALIZER;

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
static void *dbus_offload_thread_func(void *data)
{
	DbusAsyncMsg_t *msg = NULL;
	DeviceAgent::DbusOffloadQueue_t *q = (DeviceAgent::DbusOffloadQueue_t *)data;
	if (NULL == q)
	{
		cout << "ERROR: invalid dbus offload queue\n";
		return NULL;
	}

	while(1)
	{
		pthread_cond_wait(&g_dbus_cond, &g_dbus_mutex);

		while (q->pop(msg))
		{
			/* 
			 * This is to serialize the invocations to DBUS proxy.
			 */
			//cout << "Dbus Offload Thread pop data: " << msg->params << endl;
			DEV_AGENT->SendToDataManager(msg);
		}
	}

	return NULL;
}

static void sendToDataManagerCompleted(DBusGProxy* proxy,
									   DBusGProxyCall* call,
									   gpointer userData) {

	/* This will hold the GError object (if any). */
	GError* error = NULL;

	DbusAsyncMsg *msg = (DbusAsyncMsg *)userData;

	//cout << "sendToDataManagerCompleted\n";

	/* We next need to collect the results from the RPC call.
	   The function returns FALSE on errors (which we check), although
	   we could also check whether error-ptr is still NULL. */
	if (!dbus_g_proxy_end_call(proxy,
				/* The call that we're collecting. */
				call,
				/* Where to store the error (if any). */
				&error,
				/* Next we list the GType codes for all
				   the arguments we expect back. In our
				   case there are none, so set to
				   invalid. */
				G_TYPE_INVALID)) {
		/* Some error occurred while collecting the result. */
		cout << " ERROR: " << error->message << endl;
		g_error_free(error);
		free (msg->params);
		free (msg);
		return;
	}

	if (msg->action == DBUS_DATA_MANAGER_ACTION_NEW)
	{
		/* Construct the class name for the device */
		DeviceConf_t conf;
		GError *error = NULL;
		string devname;

		/* Set the device configuration */
		if (DEV_AGENT->StringToConf(msg->params, &conf, devname))
		{
			free (msg->params);
			free (msg);
			return;
		}

		DeviceBase *dev = static_cast<DeviceBase*>(g_factory.construct(devname.c_str(), conf));
		if (NULL == dev)
		{
			cout << "ERROR: Suitable Driver for Device not found. " << devname.c_str() << endl;;
			free (msg->params);
			free (msg);
			return;
		}

		DEV_AGENT->AddDevice(dev);
		DEV_AGENT->DeployDevice(dev);
	}

	free (msg->params);
	free (msg);
}

void DeviceAgent::SendToDataManager(DbusAsyncMsg_t *msg)
{
	GError *error = NULL;

	dbus_g_proxy_begin_call(m_dataManagerObj,
			/* Method name. */
			(msg->action == DBUS_DATA_MANAGER_ACTION_NEW) ? "new" : "insert",
			/* Callback to call on "completion". */
			sendToDataManagerCompleted,
			/* User-data to pass to callback. */
			msg,
			/* Function to call to free userData after
			   callback returns. */
			NULL,
			/* Arguments */
			G_TYPE_STRING,
			msg->params,
			/* Terminate argument list. */
			G_TYPE_INVALID);

	return;
}

/*
 * This function expects params to be already allocated.
 * It will be freed once the dbus proxy call completes
 * asynchronously.
 */
void DeviceAgent::UpdateDataManager(const char *action, char *params)
{
	GError *error = NULL;

	DbusAsyncMsg_t *msg = (DbusAsyncMsg_t *) malloc(sizeof(DbusAsyncMsg_t));

	if (NULL == msg)
	{
		cout << "ERROR: malloc failed for DbusAsyncMsg" << endl;
		free (params);
		return;
	}

	if (0 == strcmp(action, "insert"))
	{
		msg->action = DBUS_DATA_MANAGER_ACTION_INSERT;
	} 
	else if (0 == strcmp(action, "new"))
	{
		msg->action = DBUS_DATA_MANAGER_ACTION_NEW;
	}
	else
	{
		cout << "ERROR: Unknown action in DbusAsyncMsg" << endl;
		free (params);
		free (msg);
		return;
	}

	msg->params = params;

	m_dbus_offload_queue.push(msg);

	pthread_cond_signal(&g_dbus_cond);
}


int DeviceAgent::AddDevice(DeviceBase *dev)
{
	g_mutex_lock(m_deviceMapLock);
	m_deviceMap.insert(make_pair<std::string&, DeviceBase *>(dev->GetDeviceName(), dev));
	g_mutex_unlock(m_deviceMapLock);

	printf("Added %s to deviceMap\n", dev->GetDeviceName().c_str());
	return 0;
}

int DeviceAgent::RemoveDevice(string name)
{
	g_mutex_lock(m_deviceMapLock);
	m_deviceMap.erase(name);
	g_mutex_unlock(m_deviceMapLock);
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
	m_dataManagerObj = obj;

	m_dbus_offload_th = g_thread_new("DbusOffloadThread", &dbus_offload_thread_func, (gpointer)&m_dbus_offload_queue);

	/* 
	 * The function does not return any status, so can't check for
	 * errors here. 
	 */
	dbus_g_connection_register_g_object(m_bus,
			(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+name).c_str(),
			G_OBJECT(m_obj));

	m_deviceMapLock = g_mutex_new();	

	g_print("%s ready to serve requests\n", name.c_str());

	return 0;
}

int DeviceAgent::CreateNewDeviceAsync(const char *iface_name, char *params)
{
	GError *error = NULL;

	/*
	 * Now instruct the DataManager to create a new StorageSlot
	 * for this device.
	 */
	UpdateDataManager("new", params);

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
	dbus_g_proxy_call (m_dataManagerObj,
			"new",
			&error,
			G_TYPE_STRING,
			params,
			G_TYPE_INVALID,
			G_TYPE_INVALID);

	DeviceBase *dev = static_cast<DeviceBase*>(g_factory.construct(devname.c_str(), conf));
	if (NULL == dev)
	{
		cout << "ERROR: Suitable Driver for Device not found. " << devname.c_str() << endl;;
		return NULL;
	}

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
		else if (it->first == "CommMethod")
		{
			if (it->second == "\"gpio\"")
			{
				conf->m_commMethod = DEVICE_COMM_METHOD_GPIO;
			}
		}
		else if (it->first == "CommParams")
		{
			conf->m_commParams.assign(it->second);
		}
		else if (it->first == "DriverName")
		{
			it->second.erase(std::remove(it->second.begin(), it->second.end(), '\"'), it->second.end());
			conf->m_driverName.assign(it->second);
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

