#include "ContextManager.hpp"
#include <boost/algorithm/string.hpp>

ContextManager *s_context_manager = NULL;

using namespace std;

ContextManager *ContextManager::getInstance()
{
	if (NULL == s_context_manager)
	{
		s_context_manager = new ContextManager;
	}

	return s_context_manager;
}

int ContextManager::Init()
{
	string name("contextManager");
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
	service_path += "contextManager";

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

	m_obj = (ContextManagerObject *)g_object_new(CONTEXTMANAGER_TYPE_OBJECT, NULL);
	if (m_obj == NULL)
	{
		g_print("Failed to create contextManager gobj instance.\n");
	}

	g_print("Registering ContextManager DbusObject on the D-Bus.\n");

	/* 
	 * The function does not return any status, so can't check for
	 * errors here. 
	 */
	dbus_g_connection_register_g_object(m_bus,
			(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+name).c_str(),
			G_OBJECT(m_obj));

	g_print("%s ready to serve requests\n", name.c_str());

	/*
	 * Now create a proxy object so that we can communicate with 
	 * the RuleManager.
	 */
	service_path.assign(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "ruleManager";
	string obj_path(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "ruleManager";
	string iface_path(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "RuleManager";

	DBusGProxy *obj = dbus_g_proxy_new_for_name(m_bus,
										service_path.c_str(), /* name */
										obj_path.c_str(), /* obj path */
										iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	cout << "DBUS Proxy for RuleManager created successfully\n";
	m_ruleManagerObj = obj;
	
	/*
	 * Now create a proxy object so that we can communicate with 
	 * the RuleManager.
	 */
	service_path.assign(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "dataManager";
	obj_path.assign(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "dataManager";
	iface_path.assign(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "DataManager";

	obj = dbus_g_proxy_new_for_name(m_bus,
										service_path.c_str(), /* name */
										obj_path.c_str(), /* obj path */
										iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	cout << "DBUS Proxy for DataManager created successfully\n";
	m_dataManagerObj = obj;

	/*
	 * Now create a proxy object so that we can communicate with 
	 * the NotificationAgent.
	 */
	service_path.assign(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "notificationAgent";
	obj_path.assign(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "notificationAgent";
	iface_path.assign(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "NotificationAgent";

	obj = dbus_g_proxy_new_for_name(m_bus,
										service_path.c_str(), /* name */
										obj_path.c_str(), /* obj path */
										iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	cout << "DBUS Proxy for NotificationAgent created successfully\n";
	m_notificationAgentObj = obj;
	/*
	 * Initialize the Device Catalog 
	 */
	DEV_CATALOG->Init();

	return 0;
}

int ContextManager::HandleQueryUrl(const char *query_type, const char *params, char **response)
{
	if (0 == strcmp(query_type, "/rule"))
	{
		/* Request goes to RuleManager */
		cout << "Handle rule " << params << endl;
		return HandleRule(params, response);
	}
	else if (0 == strcmp(query_type, "/event"))
	{
		/* Request goes to RuleManager and NotificationAgent */
		cout << "Handle event request " << params << endl;
		return HandleEventRequest(params, response);
	}
	else if (0 == strcmp(query_type, "/getDevData"))
	{
		/* Request goes to DataManager */
		cout << "Get Dev Data " << params << endl;
		return HandleDevDataRequest(params, response);
	}
	else if (0 == strcmp(query_type, "/getOntData"))
	{
		/* Request satisfied by ContextManager itself */
		cout << "Get Ont Data " << params << endl;
		return HandleQuery(params, response);
	}
	else if (0 == strcmp(query_type, "/executeCommand"))
	{
		/* Request goes to DeviceManager */
		cout << "Exec command " << params << endl;
		return HandleDevCommand(params, response);
	}
	else
	{
		cout << "Error: Received unknown request from HTTP Server.\n";
		return -1;
	}

	return -1;
}

int ContextManager::HandleQuery(const char *params, char **resp)
{
	client::pairs_type m;
	bool ret = false;
	vector<DeviceObj *> devObjVec;
	string response;

	if (-1 == url_key_value_to_map(params, m))
	{
		cout << "Error: Could not parse params from HTTP Server request\n";
		return -1;
	}

	/* 
	 * We lookup in the device catalog for
	 * the specific device based on the URL/Ontology.
	 */

	ret = DEV_CATALOG->GetDeviceObjByFilter(m, devObjVec);

	if (false == ret)
	{
		/* 
		 * We did not find any valid devices using filtering.
		 * 
		 */
		return false;
	}
	else if (devObjVec.empty())
	{
		/*
		 * We did not find any valid device. So send back error
		 */
		response.assign("DevObjVec Empty!");
	}
	else
	{
		/*
		 * Found Devices.
		 */
		response.assign("Devices found: ");
		BOOST_FOREACH(DeviceObj *obj, devObjVec)
		{
			cout << obj->GetName() << endl;
			response.append(obj->GetName() + " ");
		}
	}

	cout << response << endl;
	*resp = strdup(response.c_str());
	return 0;
}

int ContextManager::HandleDeviceInfo(const char *params, char **resp)
{
	client::pairs_type m;
	DeviceObj *obj;
	map<string, string>::iterator it;
	char response[512];
	string poll_type;
	string data_storage_location;

	if (-1 == url_key_value_to_map(params, m))
	{
		cout << "Error: Could not parse params from HTTP Server request\n";
		return -1;
	}

	it = m.find("DeviceName");
	if (it == m.end())
	{
		cout << "Error: Could not find device name in keyvals map\n";
		return -1;
	}
	
	obj = DEV_CATALOG->GetDeviceObjByName(it->second.c_str());

	if (NULL == obj)
	{
		cout << "ERROR: Could not find device-obj during device-info req" << endl;
		return -1;
	}

	string service_path = SMARTGATEWAY_SERVICE_PATH_PREFIX + obj->GetName();
	string obj_path = SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX + obj->GetName();
	string iface_path = SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX + obj->GetDevIface()->GetName();

	/* Get Poll Type (time driven or event driven) from the Ontology */
	DEV_CATALOG->GetDataFromOntology(obj->GetName(), "hasDataUpdatePollMode", poll_type);
	DEV_CATALOG->GetDataFromOntology(obj->GetName(), "hasDataStorageLocation", data_storage_location);

	/* Prepare the response */
	snprintf(response, 512, "ServicePath=%s&ObjPath=%s&IfacePath=%s&PollType=%s&DataStorageLocation=%s", 
					service_path.c_str(), obj_path.c_str(), iface_path.c_str(), poll_type.c_str(), data_storage_location.c_str());

	cout << "Device info response: " << response << endl;

	*resp = strdup(response);

	return 0;
}

int ContextManager::HandleDevCommand(const char *params, char **resp)
{
	client::pairs_type m;
	DeviceObj *obj;
	map<string, string>::iterator it;
	map<string, string>::const_iterator cit;
	string devname;
	string response;
	string reqStr;

	ResolveDeviceReferences(params, reqStr);

	if (-1 == url_key_value_to_map(reqStr.c_str(), m))
	{
		cout << "Error: Could not parse params from HTTP Server request\n";
		return -1;
	}

	it = m.find("DeviceName");
	if (it == m.end())
	{
		cout << "Error: Could not find device name in keyvals map\n";
		return -1;
	}

	devname.assign(it->second);

	obj = DEV_CATALOG->GetDeviceObjByName(devname.c_str());

	if (NULL == obj)
	{
		response.assign("GetDeviceObjByName not found!");
		return 0;
	}
	/* 
	 * Now lookup the method that needs to be 
	 * executed on this deviceObj.
	 * This can be done using map of methods and comparing 
	 * against the requested method in "args".
	 * For now statically check the method.
	 */
	cout << "---Begin ARGS--- " << m.size() << endl;

	if (m.size() <= 0)
	{
		return true;
	}

	for (cit = m.begin(); cit != m.end(); cit++)
	{
		cout << cit->first << " : " << cit->second << endl;

		if (cit->first == "DeviceName")
		{
			/* skip "DeviceName" itself. */
			continue;
		}

		/*
		 * Check if the commands are any of the predefined ones.
		 * For now only "get_methods" is implemented.
		 * TODO: add more of such commands.
		 */
		if (cit->first == "get_methods")
		{
			vector<string> mvec;
			cout << "get_methods invoked" << endl;
			obj->GetMethodVec(mvec);

			response += "Available Methods:";
			BOOST_FOREACH(string mStr, mvec)
			{
				response += "\n\t";
				response += mStr;
			}
		}
		else
		{
			DeviceMethod *method;

			MethodArgs_t *argsIn = obj->PrepareMethodArgs(cit->first.c_str(), m);

			MethodArgs_t *argsOut = obj->ExecuteCommand(cit->first.c_str(), argsIn, &method);

			if (argsIn)
			{
				cout << "Args are: " << endl;
				cout << argsIn->m_first.i << endl;

				delete argsIn;
			}

			if (argsOut)
			{
				// generate output
				obj->PrepareJsonResponseFromArgs(argsOut, response);

				// delete the memory allocated by the dbus proxy call
				if (method->GetOutArgTypes().size() >= 1)
				{
					if ((method->GetOutArgTypes()[0] == METHOD_ARG_TYPE_STR)
							&& (argsOut->m_first.str != NULL))
						g_free(argsOut->m_first.str);
				}

				// delete once done
				delete argsOut;
			}
		}
	}

	//response += ".\n";
	*resp = strdup(response.c_str());
	cout << "---END ARGS---" << endl;

	return true;
}

int ContextManager::ResolveDeviceReferences(const char *params, string& ruleStr)
{
	istringstream strstream(params);
	string lineStr;
	map<string, string> varMap;

	/*
	 * Before the request can be processed, any device
	 * references need to be resolved based on the 
	 * Ontology.
	 * A sample request for a rule registration would contain:
	 * -------------
	 * DevRef=X&DeviceClass=Lighting&hasLocation=Bedroom&causes=Light
	 * DevRef=Y&DeviceClass=Device&hasLocation=Bedroom&measures=Temperature
	 * Action=insert_rule&IsOneShot=true&Rule=do(<X>, set_status, 0, Ts) :- get(<Y>, get_status, Val, Ts), Val =:= 1
	 * -------------
	 * In the above request, the first two lines need to be resolved via the 
	 * Ontology to find X and Y. Then replace <X> and <Y> with the appropriate
	 * device instances.
	 * After that send the modified rule to the RuleManager.
	 */

	/* Step 1: decode and resolve variables */
	while (getline(strstream, lineStr))
	{
		/* 
		 * Check if this line is a Device Reference 
		 * which means that the line contains a "DevRef"
		 * keyword. Look at the sample above for more details
		 */
		if (string::npos != lineStr.find("DevRef"))
		{
			int ret;
			vector<DeviceObj *> devObjVec;
			client::pairs_type m;

			/* Device Reference */
			if (-1 == url_key_value_to_map(lineStr.c_str(), m))
			{
				cout << "Error: Could not parse params from HTTP Server request\n";
				return -1;
			}

			map<string, string>::iterator it;
			it = m.find("DevRef");
			if (m.end() == it)
			{
				cout << "ERROR: Malformed DevRef string in request\n";
				return -1;
			}

			string varName(it->second);

			/* Now remove the DevRef key itself from the map */
			m.erase(it);

			/* Resolve the DevRef via the Ontology */
			ret = DEV_CATALOG->GetDeviceObjByFilter(m, devObjVec);

			if (devObjVec.empty())
			{
				cout << "ERROR: Could not resolve " << lineStr << ret << endl;
				return -1;
			}

			/*
			 * Found Devices.
			 * TODO: handle the case when multiple devices exist. For now just use the first.
			 */
			cout << "Devices found for: " << lineStr << endl;
			BOOST_FOREACH(DeviceObj *obj, devObjVec)
			{
				cout << obj->GetName() << endl;
			}

			varMap.insert(make_pair<string, string>(varName, devObjVec.front()->GetName()));
		}
		else
		{
			ruleStr.assign(lineStr);
		}
	}

	/* 
	 * Step 2: replace every occurrence of each Var in the varMap
	 * with its corresponding value in the ruleStr.
	 */
	map<string, string>::iterator it_var;

	for (it_var = varMap.begin(); it_var != varMap.end(); it_var++)
	{
		string replaceStr = "<" + it_var->first + ">";
		boost::replace_all(ruleStr, replaceStr.c_str(), it_var->second.c_str());
	}

	return 0;
}

int ContextManager::HandleRule(const char *params, char **response)
{
	client::pairs_type m;
	GError *error = NULL;
	string ruleStr;
	map<string, string>::iterator it;

	ResolveDeviceReferences(params, ruleStr);
	
	/* ruleStr now contains a Prolog-processable rule */
	cout << "RuleStr: " << ruleStr << endl;

	if (-1 == url_key_value_to_map(ruleStr.c_str(), m))
	{
		cout << "Error: Could not parse params from HTTP Server request\n";
		return -1;
	}
	
	/* Extract the rule action (insert or delete) */
	it = m.find("Action");
	if (it == m.end())
	{
		cout << "Invalid Args in rule handle request\n";
		return -1;
	}

	dbus_g_proxy_call (m_ruleManagerObj,
					   it->second.c_str(),
					   &error,
					   G_TYPE_STRING, 
					   ruleStr.c_str(),
					   G_TYPE_INVALID,
					   G_TYPE_STRING,
					   response,
					   G_TYPE_INVALID);

	return 0;
}

int ContextManager::HandleEventRequest(const char *params, char **response)
{
	client::pairs_type m;
	GError *error = NULL;
	string eventStr;
	map<string, string>::iterator it;

	ResolveDeviceReferences(params, eventStr);
	
	/* ruleStr now contains a Prolog-processable rule */
	cout << "EventStr: " << eventStr << endl;

	if (-1 == url_key_value_to_map(eventStr.c_str(), m))
	{
		cout << "Error: Could not parse params from HTTP Server request\n";
		return -1;
	}
	
	/* Extract the event action (insert/delete event or add/remove subscriber) */
	it = m.find("Action");
	if (it == m.end())
	{
		cout << "Invalid Args in rule handle request\n";
		return -1;
	}

	/* Notification agent needs to know about the everything */
	cout << "Informaing notificationAgent\n";
	dbus_g_proxy_call (m_notificationAgentObj,
					   it->second.c_str(),
					   &error,
					   G_TYPE_STRING, 
					   eventStr.c_str(),
					   G_TYPE_INVALID,
					   G_TYPE_STRING,
					   response,
					   G_TYPE_INVALID);

	if (string::npos != it->second.find("event"))
	{
		/* RuleManager needs to know about the event registrations only */
		cout << "Informaing ruleManager\n";
		dbus_g_proxy_call (m_ruleManagerObj,
					   it->second.c_str(),
					   &error,
					   G_TYPE_STRING, 
					   params,
					   G_TYPE_INVALID,
					   G_TYPE_STRING,
					   response,
					   G_TYPE_INVALID);
	}

	return 0;
}


int ContextManager::HandleDevDataRequest(const char *params, char **response)
{
	client::pairs_type m;
	GError *error = NULL;
	string devname;
	string command;
	int ts, num_elems;
	map<string, string>::iterator it;
	string reqStr;

	ResolveDeviceReferences(params, reqStr);

	if (-1 == url_key_value_to_map(reqStr.c_str(), m))
	{
		cout << "Error: Could not parse params from HTTP Server request\n";
		return -1;
	}

	/* Extract the DeviceName, Command and Ts info from the request */
	it = m.find("DeviceName");
	if (it == m.end())
	{
		cout << "Invalid Args in device data request\n";
		return -1;
	}

	devname = it->second;

	it = m.find("Command");
	if (it == m.end())
	{
		cout << "Invalid Args in device data request\n";
		return -1;
	}

	command = it->second;

	it = m.find("Ts");
	if (it == m.end())
	{
		cout << "Invalid Args in device data request\n";
		return -1;
	}
	
	ts = strtol(it->second.c_str(), NULL, 10);

	/*
	 * TODO:
	 * Check validity of the DeviceName. Basically it should be an Instance in the Ontology.
	 */

	dbus_g_proxy_call (m_dataManagerObj,
					   "find",
					   &error,
					   G_TYPE_STRING, 
					   devname.c_str(),
					   G_TYPE_STRING,
					   command.c_str(),
					   G_TYPE_INT,
					   ts,
					   G_TYPE_INVALID,
					   G_TYPE_STRING,
					   response,
					   G_TYPE_INT,
					   &num_elems,
					   G_TYPE_INVALID);
	
	cout << "Response received from DataManager=" << *response << "Num elements=" << num_elems << endl;

	return 0;
}
