#ifndef __DEVICE_CATALOG_HPP__
#define __DEVICE_CATALOG_HPP__
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>

using namespace std;

#define DEV_CATALOG (DeviceCatalog::GetInstance())

#define MAX_METHOD_ARG_STR_LEN 256

typedef enum
{
	METHOD_TYPE_NONE_IN_NONE_OUT = 0,
	METHOD_TYPE_NONE_IN_ONE_OUT,
	METHOD_TYPE_NONE_IN_TWO_OUT,
	METHOD_TYPE_ONE_IN_NONE_OUT,
	METHOD_TYPE_ONE_IN_ONE_OUT,
	METHOD_TYPE_ONE_IN_TWO_OUT,
	METHOD_TYPE_TWO_IN_NONE_OUT,
	METHOD_TYPE_TWO_IN_ONE_OUT,
	METHOD_TYPE_TWO_IN_TWO_OUT,
	METHOD_TYPE_GENERIC
} MethodType_t;

typedef enum
{
	METHOD_ARG_TYPE_INT = 0,
	METHOD_ARG_TYPE_FLOAT,
	METHOD_ARG_TYPE_STR,
	METHOD_ARG_TYPE_UNSET
} MethodArgType_t;

typedef enum
{
	METHOD_ARG_DIR_IN = 0,
	METHOD_ARG_DIR_OUT,
	METHOD_ARG_DIR_UNSET
} MethodArgDir_t;

union MethodArgs
{
	int i;
	float f;
	//char str[MAX_METHOD_ARG_STR_LEN];
	char *str;
};

typedef struct
{
	union MethodArgs m_first;
	union MethodArgs m_second;
} MethodArgs_t;

class DeviceMethod
{
	private:
		string m_name;
		MethodType_t m_type;
		vector<MethodArgType_t> m_in_types;
		vector<MethodArgType_t> m_out_types;
	
	public:
		DeviceMethod(const string& name) 
		{
			m_name = name;
			m_type = METHOD_TYPE_NONE_IN_NONE_OUT;
		}
 
		~DeviceMethod() {}

		vector<MethodArgType_t>& GetInArgTypes()
		{
			return m_in_types;
		}
		
		vector<MethodArgType_t>& GetOutArgTypes()
		{
			return m_out_types;
		}

		void InsertNewArg(MethodArgType_t type, MethodArgDir_t dir)
		{
			switch (dir)
			{
				case METHOD_ARG_DIR_IN:
					m_in_types.insert(m_in_types.begin(), type);
					if (m_type < METHOD_TYPE_GENERIC)
					{
						m_type = static_cast<MethodType_t>(static_cast<int>(m_type) + 3);
					}
					else
					{
						m_type = METHOD_TYPE_GENERIC;
					}
					break;

				case METHOD_ARG_DIR_OUT:
					m_out_types.insert(m_out_types.begin(), type);
					if (m_type < METHOD_TYPE_GENERIC)
					{
						m_type = static_cast<MethodType_t>(static_cast<int>(m_type) + 1);
						//m_type += 1;
					}
					else
					{
						m_type = METHOD_TYPE_GENERIC;
					}

					break;

				default:
					cout << "ERROR: Invalid arg direction" << endl;
					break;
			}
		}

		MethodType_t GetType(void)
		{
			return m_type;
		}

		int GetNumArgsIn()
		{
			return m_in_types.size();
		}

		int GetNumArgsOut()
		{
			return m_out_types.size();
		}

		void SetType(MethodType_t type)
		{
			m_type = type;
		}

		string& GetName()
		{
			return m_name;
		}
};

typedef map<string, DeviceMethod*> MethodMapEntryType_t;
typedef map<string, string> SignalMapEntryType_t;

class DeviceIface
{
	private:
		int m_id;

		string m_name;
		/* 
		 * Used to denote the functions that can be 
		 * performed on the device 
		 */
		MethodMapEntryType_t m_methodMap;

		/* 
		 * Used to denote the event notifications that 
		 * can be received from the device 
		 */
		SignalMapEntryType_t m_signalMap;

	public: 
		DeviceIface(const string& name) 
		{
			m_name = name;
		}
		~DeviceIface() {}

		MethodMapEntryType_t& GetMethodMap()
		{
			return m_methodMap;
		}

		SignalMapEntryType_t& GetSignalMap()
		{
			return m_signalMap;
		}

		string& GetName()
		{
			return m_name;
		}

		void InsertNewMethod(DeviceMethod *m)
		{
			m_methodMap.insert(make_pair<string, DeviceMethod*>(m->GetName(), m));
		}

		void SetName(const char *name)
		{
			m_name.assign(name);
		}
	
		DeviceMethod *GetDeviceMethod(const char *name)
		{
			MethodMapEntryType_t::iterator it = m_methodMap.find(name);
			if (it != m_methodMap.end())
			{
				return it->second;
			}

			cout << "Method not found" << endl;
			return NULL;
		}

		MethodArgs_t *PrepareMethodArgs(const char *name, const map<string, string>& keyvals);
};

class DeviceObj 
{
	private:
		/* D-Bus specific */
		string m_name;
		DBusGProxy *m_obj;

		/* Device capabilities */
		DeviceIface *m_devIface;

	public:
		DeviceObj() {}
		~DeviceObj() {}

		DBusGProxy *GetObj()
		{
			return m_obj;
		}

		int Init(string&);

		string& GetName()
		{
			return m_name;
		}

		DeviceIface *GetDevIface()
		{
			return m_devIface;
		}

		void SetName(const char *name)
		{
			m_name.assign(name);
		}
		
		void SetObj(DBusGProxy *obj)
		{
			m_obj = obj;
		}
		
		DeviceIface *LookupIface();

		int CreateDbusObj(string&, string&, string&);
			
		int GetMethodVec(vector<string>& mVec);
		MethodArgs_t *PrepareMethodArgs(const char *name, const map<string, string>& keyvals);
		MethodArgs_t *ExecuteCommand(const char *command, MethodArgs_t *argsIn, DeviceMethod **m);
		
		void PrepareJsonResponseFromArgs(MethodArgs_t *args, string& response);
};

class DeviceCatalogEntry
{
	private:
		int m_id;
		string m_phyName;
		DeviceObj *m_devObj;

	public:
		DeviceCatalogEntry(const string& name) 
		{
			m_phyName = name;
		}

		~DeviceCatalogEntry() {}

		DeviceObj *GetDeviceObj()
		{
			return m_devObj;
		}

		int GetId()
		{
			return m_id;
		}

		void SetId(int id)
		{
			m_id = id;
		}

		string& GetName()
		{
			return m_phyName;
		}

		int CreateDeviceObj();
};

typedef map<string, DeviceCatalogEntry*> DeviceCatalogMap_t;
typedef map<string, DeviceIface*> DeviceIfaceMap_t;

class DeviceCatalog
{
	private:
		string m_serviceName;
		DeviceCatalogMap_t m_deviceCatalogMap;
		DeviceIfaceMap_t m_deviceIfaceMap;
		DBusGProxy *m_deviceAgentObj;

	public:
		DeviceCatalog()
		{}

		~DeviceCatalog()
		{/*TODO: clear all devices in the catalog */}

		int Init();
		static DeviceCatalog *GetInstance();

		int RegisterNewDevice(const string&);

		int InsertNewInterface(DeviceIface *iface)
		{
			m_deviceIfaceMap.insert(make_pair<string, DeviceIface*>(iface->GetName(), iface));
			return 0;
		}

		int InsertNewCatalogEntry(DeviceCatalogEntry *dce)
		{
			m_deviceCatalogMap.insert(make_pair<string, DeviceCatalogEntry*>(dce->GetName(), dce));
			return 0;
		}
		
		DeviceObj *GetDeviceObjByName(const char *name);

		bool GetDeviceObjByFilter(const map<string, string>& keyvals, 
								  vector<DeviceObj *>& devObjVec);

		bool GetInstancesByFilter(const map<string, string>& keyvals, 
								  vector<string>& devObjVec);

		int GetDataFromOntology(const string& subj, const string& pred, string& obj);

		DeviceIface *GetIfaceByName(const string& name);

		int OntologyUpdate(string& action, string& subj, string& pred, string& val);
	
		string& GetServiceName()
		{
			return m_serviceName;
		}
		
		void SetServiceName(const char *name)
		{
			m_serviceName.assign(name);
		}

		void InitNewDeviceDbus(string& iface, string& phy_name);

		int LoadDeviceInterfaces();
		int GetPhysicalDevices(vector<string>&);
};
#endif
