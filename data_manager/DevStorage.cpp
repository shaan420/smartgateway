#include "DevStorage.hpp"
#include <iostream>
#include "RingBufferStorage.hpp"
#include "CloudStorage.hpp"
#include "DataManager.hpp"

DevStorage::DevStorage(const char *name, StorageSlotType_t type) : 
					m_devName(name), 
					m_preferred_storage_type(type)
{
	/* Perform type-specific initializations */
	if (m_preferred_storage_type == STORAGE_SLOT_TYPE_CLOUD)
	{
		m_ctx = DATA_MANAGER->GetCloudChannelCtx(name);
		//m_ctx = (void *)ts_create_channel(TS_USER_KEY, name);
	}
}

StorageSlotBase *DevStorage::FindStorageSlot(const char *command, bool create)
{
	string comm(command);
	StorageSlotMap_t::iterator it;
	it = m_slotMap.find(comm);
	ts_context_t *ctx = NULL;

	if (it != m_slotMap.end())
	{
		/* Found storageSlot */
		cout << "Found Storage slot\n";
		return it->second;
	}
	else
	{
		if (false == create) return NULL;

		if (m_slotMap.size() >= MAX_NUM_SLOTS)
		{
			cout << "ERROR: Slot limit reached\n";
			return NULL;
		}

		if (m_preferred_storage_type == STORAGE_SLOT_TYPE_CLOUD)
		{
			/*
			 * Currently the Thingspeak Cloud platform only supports
			 * 8 fields i.e. max 8 commands per device
			 */
			if (m_slotMap.size() >= 8)
			{
				cout << "ERROR: Cloud Slot limit reached\n";
				return NULL;
			}
		}

		StorageSlotBase *s = NULL;
		/* Not found, create based on preferred storage type */
		switch (m_preferred_storage_type)
		{
			case STORAGE_SLOT_TYPE_RDF:
				//s = new RdfStorage(command);
				break;

			case STORAGE_SLOT_TYPE_RING_BUFFER:
				s = new RingBufferStorage<int>(command);
				break;		

			case STORAGE_SLOT_TYPE_CLOUD:
				ctx = static_cast<ts_context_t *>(m_ctx);
				s = new CloudStorage<float>(ctx, command, m_slotMap.size()+1);
				break;

			default:
				cout << "ERROR: Unsupported storage slot type" << endl;
				break;
		}

		if (NULL == s)
		{
			cout << "ERROR: could not create storage slot\n";
			return NULL;
		}

		/* 
		 * Now insert the new storageSlot into the slotMap
		 */
		m_slotMap.insert(make_pair<string, StorageSlotBase*>(comm, s));
		return s;
	}

	return NULL;
}
