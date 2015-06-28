#ifndef __RING_BUFFER_STORAGE_HPP__
#define __RING_BUFFER_STORAGE_HPP__
#include "StorageSlot.hpp"
#include "TimeManager.hpp"
#include <boost/circular_buffer.hpp>
#include <boost/lexical_cast.hpp>
/*
 * Backup streaming data for the past 1 hour per device
 * per command.
 */
#define MAX_ARCHIVE_SECONDS 3600

template <typename T>
class RingBufferStorage : public StorageSlot<T>
{
	private:

		boost::circular_buffer<T> m_rb;

		RingBufferStorage() {}

		int GetCurBucketNumber(int ts);

	public:

		RingBufferStorage(const char *name) : StorageSlot<T>(name, STORAGE_SLOT_TYPE_RING_BUFFER), 
										   	  m_rb(MAX_ARCHIVE_SECONDS)
		{
			Init();
		}

		~RingBufferStorage() 
		{
		}

		int Init()
		{
			return 0;
		}

		int Insert(const char *value)
		{
			T val;
			switch (StorageSlot<T>::DataType())
			{
				case STORAGE_DATA_TYPE_INT:
					val = strtol(value, NULL, 10);
					m_rb.push_back(val);
					break;

				case STORAGE_DATA_TYPE_FLOAT:
					val = strtof(value, NULL);
					m_rb.push_back(val);
					break;

				case STORAGE_DATA_TYPE_STRING:
					m_rb.push_back(val);
					break;

				default:
					cout << "ERROR: Unsupported data type\n";
					return -1;
			}

			return 0;
		}

		int Find(unsigned int ts, char **res, int *num_elems)
		{
			unsigned int n = ts, i = 0;
			*num_elems = 0;
			string response;
			/*
			 * ts = 0 means get the current data
			 * ts = x means get the previous x entries (limited to MAX)
			 */
			typename boost::circular_buffer<T>::reverse_iterator rit;

			if (n > MAX_ARCHIVE_SECONDS-1)
			{
				n = MAX_ARCHIVE_SECONDS-1;
			}

			if (n > m_rb.size()-1)
			{
				n = m_rb.size()-1;
			}

			/* set the number of elements */
			*num_elems = n+1;

			/* allocate enough memory for holding results */
			//*res = (T *)malloc(sizeof(T) * n);

			rit = m_rb.rbegin();

			response.assign("NumElems=");
			response += boost::lexical_cast<string>(*num_elems) + "&";
			response += "Values=";
			while (i <= n)
			{
				response += boost::lexical_cast<string>(*rit);
				if (i < n)
				{
					response += ",";
				}
				i++;
				rit++;
			}

			response += "&";
			char ts_str[64];
			snprintf(ts_str, 64, "%llu", TIME_MANAGER->GetCurTimeSec());
			response += "Ts=";
			response += ts_str;

			*res = strdup(response.c_str());

			return 0;
		}
};

#endif
