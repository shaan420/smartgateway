#ifndef __CONTEXT_MANAGER_HPP__
#define __CONTEXT_MANAGER_HPP__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include <dbus/dbus-glib.h>
#include "../common/include/common-defs.h"
#include "ContextManagerObject.h"
#include "DeviceCatalog.hpp"
#include "url_utils.hpp"

#define CONTEXT_MANAGER s_context_manager->getInstance()

class ContextManager
{
	private:
		/* The GObject representing a D-Bus connection. */
		DBusGConnection *m_bus;
		ContextManagerObject *m_obj;

		/* Proxy object to communicate with the RuleManager and DataManager */
		DBusGProxy *m_ruleManagerObj;
		DBusGProxy *m_dataManagerObj;
		DBusGProxy *m_notificationAgentObj;

	public:
		ContextManager()
		{
		}

		~ContextManager()
		{
		}

		ContextManager *getInstance();

		int Init();

		int HandleQueryUrl(const char *query_type, const char *params, char **response);
		int HandleRule(const char *params, char **response);
		int HandleEventRequest(const char *params, char **response);
		int ResolveReferences(const char *params, string& ruleStr);
		int HandleDevCommand(const char *params, char **response);
		int HandleOntUpdate(const char *params, char **resp);
		int HandleDeviceInfo(const char *params, char **response);
		int HandleQuery(const char *params, char **response);
		int HandleDevDataRequest(const char *params, char **response);
};

extern ContextManager *s_context_manager;

#endif
