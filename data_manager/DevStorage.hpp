#ifndef __DEV_STORAGE_HPP__
#define __DEV_STORAGE_HPP__

#include <map>
#include <string>
#include <vector>
#include "StorageSlot.hpp"

using namespace std;

#define MAX_NUM_SLOTS 5

typedef map<string, StorageSlotBase *> StorageSlotMap_t;

class DevStorage
{
	private:

		/* This can be used for any storage specific needs */
		/* For example, cloud storage requires a ts_context_t */
		void *m_ctx;

		string m_devName;

		/*
		 * Every device has one storage slot for each of its supported 
		 * data-producing commands. This map stores list of such commands
		 * and its associated StorageSlot pointers.
		 */
		StorageSlotMap_t m_slotMap;

		StorageSlotType_t m_preferred_storage_type;

	public:
		DevStorage() {}
		DevStorage(const char *name) : m_devName(name), m_preferred_storage_type(STORAGE_SLOT_TYPE_RING_BUFFER)
		{
		}

		DevStorage(const char *name, StorageSlotType_t type);

		~DevStorage() {}

		/* 
		 * Find storage slot for this command in this DevStorage
		 * Create if not present depending on "create" variable.
		 */
		StorageSlotBase *FindStorageSlot(const char *command, bool create);

		int Insert(const char *command, const char *value);
		int Find(const char *command, const char **value);
};

typedef map<string, DevStorage*> DevStorageMap_t;


#endif
