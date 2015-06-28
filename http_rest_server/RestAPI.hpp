#ifndef __REST_API__
#define __REST_API__

#include <map>
#include <string>
#include <set>
#include "Executor.hpp"

using std::map;
using std::string;
using std::set;

namespace REST
{
	class RestAPI
	{
		public:
			RestAPI();
			bool executeRestAPI(const string& url, 
								string& url_string, 
								const map<string, string>& argval, 
								const char *upload_data,
								int upload_data_size,
								string& response);
		private:
			Executor _executor;

			bool _validate(const void*  data);

			bool _executeAPI(const string& url, 
							 string& url_string,
							 const set<string>& argvals, 
							 const map<string, string>& keyvals,
							 Executor::outputType type, 
							 string& response);

			void _getInvalidResponse(string& response);

			map<string, set<string> > _apiparams;
	};
} // namespace REST ends

#endif
