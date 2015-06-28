#ifndef __EXECUTOR_FOR_API__
#define __EXECUTOR_FOR_API__

#include <string>
#include <set>
#include <map>
#include <dbus/dbus-glib.h>
#include "../common/include/common-defs.h"

using std::string;
using std::set;
using std::map;

namespace REST
{
	class Executor
	{   
		public:
			enum outputType {
				TYPE_JSON, TYPE_XML   
			};
			Executor()
			{
				Init();
			}

			int Init();

			bool diskinfo(const set<string>& args, outputType type,  string& response);
			bool procinfo(const set<string>& args, outputType type, string& response);
			bool sysinfo(const set<string>& args, outputType type, string& response);
			bool SendToContextManager(const string& url, 
								  string& url_string, 
								  const map<string, string>& keyvals, 
								  outputType type, 
								  string& response);
		private:
			/* DBUS Proxy object to communicate with ContextManager */
			DBusGProxy *m_contextManagerObj;

			void _generateOutput(void *data, outputType type, string& output);

	};
} 

#endif
