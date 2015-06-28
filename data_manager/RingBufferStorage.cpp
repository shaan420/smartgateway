#include "RingBufferStorage.hpp"

template <typename T>
int RingBufferStorage<T>::Init()
{
	return 0;
}

template <typename T>
int RingBufferStorage<T>::Insert(const char *value)
{
	int val = strtol(value, NULL, 10);
	m_rb.push_back(val);
	return 0;
}

template <typename T>
typename RingBufferStorage<T>::Iterator RingBufferStorage<T>::Find(int ts)
{
	/*
	 * ts = 0 means get the current data
	 * ts = x means get the previous x entries (limited to MAX)
	 */
	Iterator it;

	if (ts > MAX_ARCHIVE_SECONDS-1)
	{
		ts = MAX_ARCHIVE_SECONDS-1;
	}

	if (ts > m_rb.size()-1)
	{
		/* range spans all entries */
		it.SetElem(m_rb.begin());
	}
	else
	{
		/* range only spans last "ts" entries */
		it.SetElem(m_rb.at(m_rb.size() - 1 - ts));
	}

	return it;
}


