#ifndef __RDF_STORAGE_HPP__
#define __RDF_STORAGE_HPP__
#include <redland.h>
#include "StorageSlot.hpp"

class RdfStorage : public StorageSlot<librdf_stream>
{
	private:
		librdf_model *m_model;
		librdf_storage *m_storage;

		RdfStorage() {}

		int GetCurBucketNumber(int ts);
		int Init();

	public:
		static librdf_world *sm_world;
		static librdf_parser *sm_parser;

		RdfStorage(const char *name) : StorageSlot(name, STORAGE_SLOT_TYPE_RDF), 
				m_model(NULL), 
				m_storage(NULL) 
		{
			Init();
		}

		~RdfStorage() 
		{
			librdf_free_model(m_model);
			librdf_free_storage(m_storage);
		}
	
		int Insert(const char *data, int data_len);
		Iterator Find(const char *key);
};
#endif
