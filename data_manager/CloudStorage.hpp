#ifndef __CLOUD_STORAGE_HPP__
#define __CLOUD_STORAGE_HPP__
#include "StorageSlot.hpp"
#include "TimeManager.hpp"

/* Otherwise compilation will fail */
extern "C"
{
#include "thingspeak.h"
#include "ts_http.h"
}

#define TS_USER_KEY "3FP2S5YM5EJ375P8"

template <typename T>
class CloudStorage : public StorageSlot<T>
{
	private:
		
		ts_context_t *m_ctx;
		int m_fieldId;
		CloudStorage() {}

	public:

		CloudStorage(ts_context_t *ctx, const char *name, int fieldId) : m_ctx(ctx), m_fieldId(fieldId),
											     			StorageSlot<T>(name, STORAGE_SLOT_TYPE_CLOUD) 
		{
			Init();
		}

		~CloudStorage() 
		{
		}

		int Init()
		{
			/*
			 * Update the existing channel with this new field_id and 
			 * command name.
			 */
			ts_add_channel_field(m_ctx, StorageSlot<T>::Name().c_str(), m_fieldId);

			return 0;
		}

		int Insert(const char *value)
		{
			T val;
			ts_datapoint_t data;
			char field_str[32];

			memset(&data, 0, sizeof(ts_datapoint_t));

			switch (StorageSlot<T>::DataType())
			{
				case STORAGE_DATA_TYPE_INT:
					val = strtol(value, NULL, 10);
					ts_set_value_i32(&data, val);
					break;

				case STORAGE_DATA_TYPE_FLOAT:
					val = strtof(value, NULL);
					ts_set_value_f32(&data, val);
					break;

				case STORAGE_DATA_TYPE_STRING:
					ts_set_value_str(&data, value);
					break;

				default:
					cout << "ERROR: Unsupported data type\n";
					return -1;
			}
			
			snprintf(field_str, 32, "field%d", m_fieldId);
			ts_datastream_update(m_ctx, 0, field_str, &data);
			return 0;
		}

		int Find(unsigned int ts, char **res, int *num_elems)
		{
#if 0
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
#endif
			return 0;
		}
};

#endif
