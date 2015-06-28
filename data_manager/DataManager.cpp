#include "DataManager.hpp"
#include "url_utils.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <exception>
#include <sstream>

DataManager *s_data_manager = NULL;

DataManager *DataManager::GetInstance()
{
	if (NULL == s_data_manager)
	{
		s_data_manager = new DataManager();

		if (NULL == s_data_manager)
		{
			cout << "ERR: DataManager could not be created.\n";
			return NULL;
		}
	}

	return s_data_manager;
}

int DataManager::Init()
{
	string name("dataManager");
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
	service_path += "dataManager";

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

	m_obj = (DataManagerObject *)g_object_new(DATAMANAGER_TYPE_OBJECT, NULL);
	if (m_obj == NULL)
	{
		g_print("Failed to create deviceAgent gobj instance.\n");
	}

	g_print("Registering DataManager DbusObject on the D-Bus.\n");

	/* 
	 * The function does not return any status, so can't check for
	 * errors here. 
	 */
	dbus_g_connection_register_g_object(m_bus,
			(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+name).c_str(),
			G_OBJECT(m_obj));

	g_print("%s ready to serve requests\n", name.c_str());

	/* Initialize the Time Manager */
	TIME_MANAGER->Init();

	/*
	 * Initialize the cloud catalog
	 */
	CloudCatalogInit();

	return 0;
}

int DataManager::CloudCatalogInit()
{
	char *channels = (char *)malloc(MAXLINE);
	if (channels == NULL)
	{
		cout << "ERROR: malloc failed\n";
		return -1;
	}

	char *body;
	int size;
	ts_context_t *temp_ctx = ts_create_context_empty(TS_USER_KEY);
	get_all_channels(temp_ctx, &channels[0]);

	body = strstr(channels, "\r\n");
	if (body == NULL)
	{
		cout << "ERROR: get_all_channels failed\n";
		free (channels);
		return -1;
	}
	
	/* get the size of the body */
	size = strtol(channels, NULL, 16);

	if (size >= MAXLINE-5)
	{
		cout << "ERROR: response size from Thingspeak cloud platform excedded limit\n";
		free (channels);
		return -1;
	}

	/* skip the \r\n */
	body += 2;

	/* mark the end of the body */
	body[size] = '\0';

	cout << "JSON Response from cloud:\n" << body << endl;

	/* 
	 * Now body points to a valid json string. 
	 * Parse the string into the CloudCatalog
	 */
	stringstream ss;
	ss << body;

	boost::property_tree::ptree pt;
	boost::property_tree::read_json(ss, pt);
	free (channels);

	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child(""))
	{
		int feed_id;
		string api_key;
		string name;
		ts_context_t *ctx = NULL;
		feed_id = v.second.get<int>("id");
		name = v.second.get<string>("name");
		boost::property_tree::ptree api_keys = v.second.get_child("api_keys");
		BOOST_FOREACH(boost::property_tree::ptree::value_type &key, api_keys.get_child(""))
		{
			api_key = key.second.get<string>("api_key");
			break;
		}

		cout << "Name: " << name << " Api_key: " << api_key << " ID: " << feed_id << endl;

		ctx = ts_create_context(api_key.c_str(), feed_id);

		if (NULL == ctx)
		{
			cout << "ERROR: ts_create_context failed\n";
			return -1;
		}

		snprintf(ctx->user_key, 16, TS_USER_KEY);
		ctx->user_key[16] = '\0';

		m_cloudCatalog.insert(make_pair<string, ts_context_t *>(name, ctx));
	}

	return 0;
}

int DataManager::CreateNewDevStorage(const char *params)
{
	/*
	 * Get the storage preference details from the params 
	 */
	string devStr;
	map<string, string> m;
	map<string, string>::iterator it;
	StorageSlotType_t slotType;

	if (url_key_value_to_map(params, m))
	{
		cout << "ERROR: Parsing new device req failed " << endl;
		return -1;
	}

	it = m.find("DeviceName");
	if (it == m.end())
	{
		cout << "DeviceName not found!\n";
		return -1;
	}

	devStr.assign(it->second);

	it = m.find("DataStorageLocation");
	if (it == m.end())
	{
		cout << "DataStorageLocation not found!\n";
		return -1;
	}
	
	slotType = ((it->second == "\"cloud\"") ? STORAGE_SLOT_TYPE_CLOUD : STORAGE_SLOT_TYPE_RING_BUFFER);

	DevStorage *d = new DevStorage(devStr.c_str(), slotType);
	if (NULL == d)
	{
		cout << "ERR: could not allocate dev storage.\n";
		return -1;
	}

	m_devStorageMap.insert(make_pair<string, DevStorage*>(devStr.c_str(), d));

	return 0;
}

ts_context_t *DataManager::GetCloudChannelCtx(const char *name)
{
	ts_context_t *ctx = NULL;
	CloudCatalog_t::iterator it;

	it = m_cloudCatalog.find(name);

	if (m_cloudCatalog.end() == it)
	{
		/* not found, create */
		ctx = ts_create_channel(TS_USER_KEY, name);
		m_cloudCatalog.insert(make_pair<string, ts_context_t *>(name, ctx));
	}
	else
	{
		/* found, return */
		ctx = it->second;
	}

	return ctx;
}

int DataManager::Insert(const char *devname, 
						const char *command, 
						const char *value)
{
	int ret;

	/* find appropriate devStorate */
	DevStorage *d = FindDevStorage(devname);

	if (NULL == d)
	{
		cout << "ERR insert(): unknown slot." << endl;
		return -1;
	}

	StorageSlotBase *slotBase = d->FindStorageSlot(command, true);

	if (NULL == slotBase)
	{
		cout << "ERR insert(): couldn't find slot." << endl;
		return -1;
	}

	switch (slotBase->Type())
	{
		case STORAGE_SLOT_TYPE_RDF:
//			{
//				RdfStorage *storage = static_cast<RdfStorage *>(slotBase);
//				ret = storage->Insert(value, val_len);
//			}
			break;
		case STORAGE_SLOT_TYPE_RING_BUFFER:
			{
				RingBufferStorage<int> *storage = static_cast<RingBufferStorage<int> *>(slotBase);
				ret = storage->Insert(value);
			}
			break;

		case STORAGE_SLOT_TYPE_CLOUD:
			{
				CloudStorage<float> *storage = static_cast<CloudStorage<float> *>(slotBase);
				ret = storage->Insert(value);
			}
			break;



		default:
			cout << "Unknown storage slot type" << endl;
			break;
	}


	if (-1 == ret)
	{
		cout << "ERR insert(): insert failed." << endl;
		return -1;
	}

	return 0;
}

int DataManager::Find(const char *devname, const char *command, int ts, char **value, int *num_elems)
{
	int ret = 0;

	/* find appropriate devStorate */
	DevStorage *d = FindDevStorage(devname);

	if (NULL == d)
	{
		cout << "ERR insert(): unknown slot." << endl;
		return -1;
	}

	StorageSlotBase *slotBase = d->FindStorageSlot(command, false);

	if (NULL == slotBase)
	{
		cout << "ERR insert(): couldn't find slot." << endl;
		return -1;
	}

	switch (slotBase->Type())
	{
		case STORAGE_SLOT_TYPE_RDF:
//			{
//				RdfStorage::Iterator it;
//				RdfStorage *storage = static_cast<RdfStorage *>(slotBase);
//				it = storage->Find(key);
//			}
			break;

		case STORAGE_SLOT_TYPE_RING_BUFFER:
			{
				RingBufferStorage<int> *storage = static_cast<RingBufferStorage<int> *>(slotBase);
				if (0 == storage->Find(ts, value, num_elems))
				{
					/* Successfully got results */
					cout << "Found: " << *value << endl;
					return 0;
				}
				else
				{
					cout << "ERROR: Could not find data\n";
					return -1;
				}
			}
			break;


		default:
			cout << "Unknown storage slot type" << endl;
			break;
	}


	if (-1 == ret)
	{
		cout << "ERR insert(): insert failed." << endl;
		return -1;
	}

	return -1;
}
