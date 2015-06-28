#include <iostream>
#include <stdlib.h>
#include "DeviceCatalog.hpp"
#include <boost/foreach.hpp>
#include "boost/filesystem.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "../common/include/common-defs.h"
#include "OntologyManager.hpp"

static DeviceCatalog *s_dev_cat = NULL;

using namespace std;
using namespace boost;
using boost::property_tree::ptree;

namespace fs = boost::filesystem;

DeviceCatalog *DeviceCatalog::GetInstance()
{
	if (NULL == s_dev_cat)
	{
		s_dev_cat = new DeviceCatalog;
	}

	return s_dev_cat;
}

DeviceObj *DeviceCatalog::GetDeviceObjByName(const char *name)
{
	DeviceCatalogMap_t::iterator it = m_deviceCatalogMap.find(name);
	if (it != m_deviceCatalogMap.end())
	{
		cout << "DeviceObj FOUND" << endl;
		return (it->second->GetDeviceObj());
	}

	return NULL;
}

bool DeviceCatalog::GetDeviceObjByFilter(const map<string, string>& keyvals, 
										 vector<DeviceObj*>& devObjVec)
{
	DeviceObj *obj = NULL;
	vector<string> devList;

	if (ONT_MANAGER->GetDeviceListByFilter(keyvals, devList))
	{
		/* Got list */
		BOOST_FOREACH(string& name, devList)
		{
			cout << "Searching DeviceObj " << name << endl;
			obj = GetDeviceObjByName(name.c_str());
			if (obj)
			{
				cout << "Got DeviceObj" << endl;
				devObjVec.push_back(obj);	
			}
		}

		return true;
	}

	return false;
}

int DeviceCatalog::GetDataFromOntology(const string& subj, const string& pred, string& obj)
{
	return ONT_MANAGER->GetData(subj, pred, obj);
}

DeviceIface *DeviceCatalog::GetIfaceByName(const string& name)
{
	DeviceIfaceMap_t::iterator it = m_deviceIfaceMap.find(name);
	if (it != m_deviceIfaceMap.end())
	{
		cout << "FOUND" << endl;
		return (it->second);
	}

	return NULL;
}


int DeviceCatalog::RegisterNewDevice(const string& dev_name)
{
	/* Create new catalog-entry for every device/file. */ 
	DeviceCatalogEntry *dce = new DeviceCatalogEntry(dev_name);
	if (NULL == dce)
	{
		cout << "ERROR: Could not allocate memory for DeviceCatalogEntry\n";
		return -1;
	}

	/* Create the deviceObj which in turn creates the d-bus obj */
	if (-1 != dce->CreateDeviceObj())
	{
		cout << "Inserting New Catalog Entry: " << dev_name << endl;
		InsertNewCatalogEntry(dce);
	}
	else
	{
		cout << "Could not create deviceObj for " << dev_name << endl; 
		delete dce;
		return -1;
	}

	return 0;
}

int DeviceCatalogEntry::CreateDeviceObj()
{
	int ret;
	DeviceObj *pDevObj = new DeviceObj;
	if (NULL == pDevObj)
	{
		cout << "Could not allocate memory for deviceObj " << endl;
		return -1;
	}

	if (m_phyName.empty())
	{
		delete pDevObj;
		cout << "Error: m_phy_name not set. Cannot CreateDeviceObj." << endl;
		return -1;
	}

	cout << "Initializing DeviceObj..." << endl;

	ret = pDevObj->Init(m_phyName);

	if (-1 == ret)
	{
		cout << "DeviceObj Init failed" << endl;
		delete pDevObj;
		return -1;
	}

	cout << "Successfully Initialized DeviceObj" << endl;
	m_devObj = pDevObj;
	return 0;
}

int DeviceObj::Init(string& name)
{
	int ret;
	cout << "DeviceObj Init() dev_name: " << name << endl;
	SetName(name.c_str());

	DeviceIface *iface = LookupIface();

	if (NULL == iface)
	{
		cout << "Error: Lookup iface failed" << endl;
		return -1;
	}

	m_devIface = iface;

	/* Now initialize the d-bus object */
	string service_path(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += GetName();
	ret = CreateDbusObj(service_path,
						GetName(),
						iface->GetName());

	if (-1 == ret)
	{
		cout << "Error: CreateDbusObj failed" << endl;
		return -1;
	}

	return 0;
}

DeviceIface *DeviceObj::LookupIface()
{
	vector<string> ifaces;

	ONT_MANAGER->GetSuperClasses(ifaces, GetName(), OWL_NODE_TYPE_INSTANCE, true);
	if (ifaces.size() > 0)
	{
		DeviceIface *iface = DEV_CATALOG->GetIfaceByName(ifaces.front());

		if (NULL == iface)
		{
			cout << "Error: DeviceIface not found: " << ifaces.front() << endl;
			return NULL;
		}
		
		return iface;
	}

	return NULL;
}

/*
 * This function first creates a dbus-obj for the new device
 * and then creates a proxy obj in this process to access that 
 * dbus-obj.
 * For eg: It first invokes "new_device" for a "Lighting" iface with 
 * "lighting1" as the dbus-obj name. Therefore, this obj can be invoked using
 * the name "lighting1". Next we create a proxy-obj for "lighting1" 
 * so that commands can be executed on that device.
 */
int DeviceObj::CreateDbusObj(string& service_name,
							   	     string& obj_name,
						  			 string& iface_name)
{
	DBusGConnection *bus;
	DBusGProxy *obj;
	GError *error = NULL;

	string obj_path = SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX + obj_name;
	string iface_path = SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX + iface_name;

	/*
	 * Instruct the deviceAgent to first instantiate a dbus-obj for this 
	 * iface and device.
	 */
	DEV_CATALOG->InitNewDeviceDbus(iface_name, obj_name);

	/*
	 * Now create a proxy obj for the newly created dbus-device obj
	 * so that we can execute commands on that device.
	 */
	bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (error != NULL) 
	{
		cout << "Error registering device" << endl;
		return -1;
	}

	obj = dbus_g_proxy_new_for_name(bus,
									 service_name.c_str(), /* name */
									 obj_path.c_str(), /* obj path */
									 iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	m_obj = obj;

	return 0;
}

const ptree& empty_ptree()
{
	static ptree t;
	return t;
}

int DeviceCatalog::LoadDeviceInterfaces()
{
	ptree tree;

	fs::path targetDir("/home/root/device_interface_files"); 

	fs::directory_iterator it(targetDir), eod;

	BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod))   
	{ 
		if(is_regular_file(p))
		{
			/* Load Interface XML file */
			cout << "Reading XML" << endl;

			read_xml(p.string(), tree);

			const ptree& methods = tree.get_child("node.interface", empty_ptree());
			const string& iface_path = methods.get<string>("<xmlattr>.name");

			const string& iface_name = iface_path.substr(iface_path.find(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX) 
											+ strlen(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX));
			/* Create new device interface */
			DeviceIface *pIface = new DeviceIface(iface_name);

			cout << "Parsing Device Interface File: " << p.string() << endl;
			BOOST_FOREACH(const ptree::value_type &method, methods)
			{
				if ((method.first == "method"))
				{
					/* Get method name from interface file */
					cout << "Extracting attributes from " << method.first << ":" << endl;
					const string& method_name = method.second.get<string>("<xmlattr>.name");
					cout << "method: " << method_name << endl;

					/* Create a new method object */
					DeviceMethod *pDevMethod = new DeviceMethod(method_name);
					const ptree& args = method.second;
					BOOST_FOREACH(const ptree::value_type &arg, args)
					{
						if (arg.first == "arg")
						{
							const string arg_type = arg.second.get<string>("<xmlattr>.type");
							const string arg_dir = arg.second.get<string>("<xmlattr>.direction");

							/* Insert the new arg in the method */
							pDevMethod->InsertNewArg((arg_type == "i") ? METHOD_ARG_TYPE_INT : METHOD_ARG_TYPE_STR,
									(arg_dir == "in") ? METHOD_ARG_DIR_IN : METHOD_ARG_DIR_OUT);

							cout << " arg_type: " << arg_type << " arg_dir: " << arg_dir << endl;
						}
					}

					cout << "Inserting New Method: " << pDevMethod->GetName() << endl;
					cout << "Method type: " << pDevMethod->GetType() << endl;
					cout << "Method in_args: " << pDevMethod->GetNumArgsIn() << endl;
					cout << "Method out_args: " << pDevMethod->GetNumArgsOut() << endl;
					pIface->InsertNewMethod(pDevMethod);
				}
			}

			/* Insert the newly created interface into the interface map */
			cout << "Inserting New Interface: " << pIface->GetName() << endl;
			InsertNewInterface(pIface);
		} 
	}

	return 0;
}

int DeviceCatalog::Init()
{
	int ret;
	DBusGConnection *bus;
	DBusGProxy *obj;
	GError *error = NULL;

	g_type_init();
	
	ONT_MANAGER->Init(SMARTGATEWAY_HOME_ONTOLOGY_FILEPATH);
	/* 
	 * Set the smartgateway service path that will be 
	 * used by the glib-dbus calls
	 */
	SetServiceName(SMARTGATEWAY_SERVICE_PATH_PREFIX);

	/* 
	 * Load the capabilities/interfaces - Methods and Signals 
	 * This loads the device interface information from the dbus
	 * xmls. This will populate the interface map which will 
	 * used in the lookup path from the web-browser request.
	 */
	cout << "Loading Device Interfaces..." << endl;
	ret = LoadDeviceInterfaces();

	/* 
	 * Now load the physical devices that are actually present.
	 * This information comes from the Ontology file.
	 * Each physical device gets linked to a particular interface
	 * which was loaded above.
	 * Which interface to link the particular physical device to,
	 * also is governed by the Ontology file.
	 * For example, if "lighting1" is a "NamedIndividual" in the 
	 * Ontology which is a member of class "Lighting", then
	 * first an interface for "Lighting" is created in 
	 * LoadDeviceInterfaces() and then the call below creates an
	 * DeviceCatalogEntry with a DeviceObj that has a dbus obj
	 * "lighting1".
	 * Therefore, 
	 * [DCE --> DeviceObj --> DbusObj (lighting1)] --> [Lighting]
	 * This implies we can have multiple physical objects that point 
	 * to the same interface.
	 */

	vector<string> phy_devices;
	cout << "Getting Physical Devices..." << endl;
	ret = GetPhysicalDevices(phy_devices);

	if (0 == ret)
	{
		cout << "No physical devices found!" << endl;
		return -1;
	}

	/*
	 * Initialize the deviceAgent proxy obj so that we can register
	 * new devices from here.
	 */
	bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (error != NULL) 
	{
		cout << "Error registering device" << endl;
		return -1;
	}

	cout << "---" << endl;
	string service_path(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "deviceAgent";
	string obj_path(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "deviceAgent";
	cout << "---" << endl;
    string iface_path(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "DeviceAgent";
	cout << "---" << endl;

	obj = dbus_g_proxy_new_for_name(bus,
									 service_path.c_str(), /* name */
									 obj_path.c_str(), /* obj path */
									 iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	m_deviceAgentObj = obj;

	cout << "Registering New Devices..." << endl;
	BOOST_FOREACH(const string& dev, phy_devices)
	{
		if (-1 == RegisterNewDevice(dev))
		{
			cout << "Failed to register device: " << dev << endl;
		}
		else
		{
			cout << "Registered device: " << dev << endl;
		}
	}

	return 0;
}

int DeviceCatalog::GetPhysicalDevices(vector<string>& phy_devices)
{
	string device_str("Device");
	int ret = ONT_MANAGER->GetInstances(phy_devices, device_str, OWL_NODE_TYPE_CONCEPT, false);
	cout << "Done fetching physical devices." << endl;
	return ret;
}

MethodArgs_t *DeviceObj::ExecuteCommand(const char *command, MethodArgs_t *argsIn, DeviceMethod **m)
{
	DeviceMethod *method;
	GError **error = NULL;
	MethodArgs_t *argsOut = NULL;

	method = m_devIface->GetDeviceMethod(command);

	if (NULL == method)
	{
		cout << "Method not found" << endl;
		return NULL;
	}

	// Used by caller of this function to free up outAgrs
	*m = method;

	cout << "Executing" << endl;
	switch (method->GetType())
	{
		// for now assume that all out args are strings
		case METHOD_TYPE_NONE_IN_NONE_OUT:
			break;

		case METHOD_TYPE_NONE_IN_ONE_OUT:
			argsOut = new MethodArgs_t;
			dbus_g_proxy_call (GetObj(), 
					           method->GetName().c_str(), 
							   error,  
							   G_TYPE_INVALID,
							   G_TYPE_STRING,
							   &argsOut->m_first.str,
							   G_TYPE_INVALID);
			cout << argsOut->m_first.str << endl;

			break;

		case METHOD_TYPE_NONE_IN_TWO_OUT:
			break;

		case METHOD_TYPE_ONE_IN_NONE_OUT:

			if (NULL == argsIn)
			{
				cout << "Error in input args." << endl;
				break;
			}

			dbus_g_proxy_call (GetObj(), 
					           method->GetName().c_str(), 
							   error, 
							   G_TYPE_INT, 
							   argsIn->m_first.i, 
							   G_TYPE_INVALID, 
							   G_TYPE_INVALID);
			break;

		case METHOD_TYPE_ONE_IN_ONE_OUT:
			break;

		case METHOD_TYPE_ONE_IN_TWO_OUT:
			break;

		case METHOD_TYPE_TWO_IN_NONE_OUT:
			break;

		case METHOD_TYPE_TWO_IN_ONE_OUT:
			break;

		case METHOD_TYPE_TWO_IN_TWO_OUT:
			break;

		case METHOD_TYPE_GENERIC:
		default:
			cout << "Method type not supported" << endl;
			break;
	}

	return argsOut;
}

MethodArgs_t *DeviceObj::PrepareMethodArgs(const char *name, const map<string, string>& keyvals)
{
	return m_devIface->PrepareMethodArgs(name, keyvals);
}

int DeviceObj::GetMethodVec(vector<string>& vec)
{
	vec.clear();
	DeviceIface *iface = GetDevIface();
	
	if (iface)
	{
		map<string, DeviceMethod*>::const_iterator it;
		for (it = iface->GetMethodMap().begin(); it != iface->GetMethodMap().end(); it++)
		{
			vec.push_back(it->second->GetName());
		}
	}

	return vec.size();
}

MethodArgs_t *DeviceIface::PrepareMethodArgs(const char *name, const map<string, string>& keyvals)
{
	MethodArgs_t *args = NULL;

	cout << "Method name: " << name << endl;
	DeviceMethod *m = GetDeviceMethod(name);

	if (NULL == m)
	{
		delete args;
		cout << "Error fetching method" << endl;
		return NULL;
	}
	
	cout << "Got method" << endl;

	switch (m->GetType())
	{
		case METHOD_TYPE_ONE_IN_NONE_OUT:
			{
				args = new MethodArgs_t;
				map<string, string>::const_iterator it;
				for (it = keyvals.begin(); it != keyvals.end(); it++)
				{
					cout << "in keyvals" << endl;
					if (it->first == name)
					{
						cout << "method matchd" << endl;
						/* method matched */
						if ((m->GetInArgTypes())[0] == METHOD_ARG_TYPE_INT)
						{
							cout << "preparing args" << endl;
							/* first input arg is an int */
							args->m_first.i = atoi(it->second.c_str());
						}
						else
						{
							/* first arg is string */
							//TODO
						}
					}
				}
			}
			break;

		case METHOD_TYPE_NONE_IN_ONE_OUT:
			cout << "Method has no input arguments." << endl;
			break;

		default:
			cout << "Not Supported!" << endl;
			break;
	}

	return args;
}

void DeviceObj::PrepareJsonResponseFromArgs(MethodArgs_t *args, string& response)
{
	// for now assume only 1 string-output-arg
	ptree pt;
	std::ostringstream ostr;
	pt.put("value", args->m_first.str);

	write_json(ostr, pt);

	response = ostr.str();
}

void DeviceCatalog::InitNewDeviceDbus(string& iface, string& phy_name)
{
	GError *error = NULL;
	char params[256];
	string retrievalFreq, retrievalMethod, dataStorageLocation, samplingFreq;
		
	GetDataFromOntology(phy_name, "hasDataRetrievalMethod", retrievalMethod);
	GetDataFromOntology(phy_name, "hasDataRetrievalFrequency", retrievalFreq);
	GetDataFromOntology(phy_name, "hasInputSamplingFrequency", samplingFreq);
	GetDataFromOntology(phy_name, "hasDataStorageLocation", dataStorageLocation);

	snprintf(params, 256, "DeviceName=%s&InputSamplingRateMsec=%s&RetrievalFreq=%s&RetrievalMethod=%s&DataStorageLocation=%s", 
					phy_name.c_str(), samplingFreq.c_str(), retrievalFreq.c_str(), retrievalMethod.c_str(), dataStorageLocation.c_str());

	cout << "New interface " << iface << " with params " << params << endl;

	dbus_g_proxy_call (m_deviceAgentObj,
						"new_device",
						&error,
						G_TYPE_STRING,
						iface.c_str(),
						G_TYPE_STRING,
						params,
						G_TYPE_INVALID,
						G_TYPE_INVALID);
}
