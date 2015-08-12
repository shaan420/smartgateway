#ifndef __STORAGE_SLOT_HPP__
#define __STORAGE_SLOT_HPP__
#include <string>

typedef enum
{
	STORAGE_SLOT_TYPE_RDF = 0,
	STORAGE_SLOT_TYPE_RING_BUFFER,
	STORAGE_SLOT_TYPE_UNKNOWN
} StorageSlotType_t;

using namespace std;

class StorageSlotBase
{
	private:	
		string m_name;
		StorageSlotType_t m_type;

		/*
		 * The current tag is used to decide whether a value for a particular timestamp
		 * is valid or from a previous overflow-cycle of the integer range.
		 * For instance, if we get a request from RuleManager asking about lighting1,
		 * command "get_status" with a particular ts, then after a lookup on the storageSlot
		 * we might find an older entry and return that...
		 * TODO: think more on this...
		 */
		static int m_curTag;

	public:
		StorageSlotBase() {}
		StorageSlotBase(const char *name, StorageSlotType_t type) : 
						m_name(name), 
						m_type(type)
		{
		}

		~StorageSlotBase() {}

		string Name()
		{
			return m_name;
		}

		StorageSlotType_t Type()
		{
			return m_type;
		}
};

template <typename T>
class StorageSlot : public StorageSlotBase
{
	public:
		StorageSlot() {}

		StorageSlot(const char *name, StorageSlotType_t type) : StorageSlotBase(name, type) {}
		~StorageSlot() {}

		class Iterator : public std::iterator<std::forward_iterator_tag, T> 
		{
			public:
				Iterator() { }
				Iterator(T e) : e_(e) {}
				~Iterator() {}

				virtual Iterator& operator=(const Iterator& other);

				virtual bool operator==(const Iterator& other);

				virtual bool operator!=(const Iterator& other);

				virtual Iterator& operator++();

				virtual Iterator& operator++(int);

				virtual T operator*();

				virtual void *operator->() { return NULL; }

				T GetElem() const 
				{
					return e_;
				}

		/*		void SetElem(T e)
				{
					e_ = e;
				}
*/
				T e_;
		};

		virtual int Init() = 0;

		virtual int Insert(const char *value) = 0;

		virtual Iterator Find(unsigned int ts) = 0;
};
#endif
