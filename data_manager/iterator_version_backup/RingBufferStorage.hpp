#ifndef __RING_BUFFER_STORAGE_HPP__
#define __RING_BUFFER_STORAGE_HPP__
#include "StorageSlot.hpp"
#include <boost/circular_buffer.hpp>
/*
 * Backup streaming data for the past 1 hour per device
 * per command.
 */
#define MAX_ARCHIVE_SECONDS 3600

template <typename T>
class RingBufferStorage : public StorageSlot<typename boost::circular_buffer<T>::iterator>
{
	private:

		boost::circular_buffer<T> m_rb;

		RingBufferStorage() {}

		int GetCurBucketNumber(int ts);

	public:

		typedef typename StorageSlot<typename boost::circular_buffer<T>::iterator>::Iterator Iterator;

		RingBufferStorage(const char *name) : StorageSlot<typename boost::circular_buffer<T>::iterator>(name, STORAGE_SLOT_TYPE_RING_BUFFER), 
										   	  m_rb(MAX_ARCHIVE_SECONDS)
		{
			Init();
		}

		~RingBufferStorage() 
		{
		}

		typename RingBufferStorage::Iterator& operator=(const typename RingBufferStorage::Iterator& other)
		{
			RingBufferStorage::Iterator::e_ = other.GetElem();
			return *this;
		}

		bool operator==(const typename RingBufferStorage::Iterator& other)
		{
			return (RingBufferStorage::Iterator::GetElem() == other.GetElem());
		}

		bool operator!=(const typename RingBufferStorage::Iterator& other)
		{
			return (RingBufferStorage::Iterator::GetElem() != other.GetElem());
		}

		T operator*()
		{
			return RingBufferStorage::Iterator::GetElem();
		}

		int Init()
		{
			return 0;
		}

		int Insert(const char *value)
		{
			int val = strtol(value, NULL, 10);
			m_rb.push_back(val);
			return 0;
		}

		typename RingBufferStorage::Iterator Find(unsigned int ts)
		{
			/*
			 * ts = 0 means get the current data
			 * ts = x means get the previous x entries (limited to MAX)
			 */
			RingBufferStorage::Iterator it;

			if (ts > MAX_ARCHIVE_SECONDS-1)
			{
				ts = MAX_ARCHIVE_SECONDS-1;
			}

			if (ts > m_rb.size()-1)
			{
				/* range spans all entries */
				it.e_ = m_rb.begin();
			}
			else
			{
				/* range only spans last "ts" entries */
				//it.e_ = m_rb.at(m_rb.size() - 1 - ts);
				it.e_ = m_rb.begin() + (m_rb.size() - 1 - ts);
			}

			return it;
		}


};

#endif
