#ifndef __DATA_MANAGER_HPP__
#define __DATA_MANAGER_HPP__
#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <dbus/dbus-glib.h>
#include "RingBufferStorage.hpp"
#include "CloudStorage.hpp"
#include "DataManagerObject.h"
#include "TimeManager.hpp"
#include "DevStorage.hpp"
#include "../common/include/common-defs.h"

using namespace std;

typedef map<string, ts_context_t *> CloudCatalog_t;

class DataManager
{
	private:
		/*
		 * Each Device/Sensor will store its data in its own 
		 * DevStorage container. Each such container will maintain
		 * a storageSlot for each dev command.
		 * Every device/sensor would be identified by its 
		 * DBUS object name.
		 * eg: "lighting1"
		 */
		DevStorageMap_t m_devStorageMap;

		/*
		 * The Thingspeak cloud platform needs to have 
		 * a ts_context for every channel existing in its 
		 * database.
		 * This catalog of channels is populated on startup
		 * of the DataManager and then dynamically updated
		 * based on new device instantiations. This allows reuse 
		 * of the existing channels when restarting the process.
		 */
		CloudCatalog_t m_cloudCatalog;
		

		/* The GObject representing a D-Bus connection. */
		DBusGConnection *m_bus;
		DataManagerObject *m_obj;

		DevStorage *FindDevStorage(const char *s)
		{
			if (NULL == s)
			{
				cout << "find dev storage failed\n";
				return NULL;
			}

			DevStorageMap_t::iterator it = m_devStorageMap.end();
			it = m_devStorageMap.find(s);

			if (m_devStorageMap.end() != it)
			{
				// found 
				return it->second;
			}

			return NULL;
		}

		int CloudCatalogInit();

	public:

		ts_context_t *GetCloudChannelCtx(const char *name);

		DataManager() {}

		~DataManager() 
		{
		}

		DataManager *GetInstance();
		
		int Init();

		int CreateNewDevStorage(const char *params);

		int Insert(const char *params);

		int Find(const char *devname, const char *command, int ts, char **value, int *num_elems);
};

extern DataManager *s_data_manager;
#define DATA_MANAGER s_data_manager->GetInstance()

#endif
