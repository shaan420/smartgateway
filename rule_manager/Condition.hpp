#ifndef __CONDITION_HPP__
#define __CONDITION_HPP__
#include <vector>
#include <string>
#include <iostream>
#include <dbus/dbus-glib.h>
#include "../common/include/common-defs.h"

using namespace std;

typedef enum ConditionType
{
	CONDITION_TYPE_GET = 0,
	CONDITION_TYPE_DO,
	CONDITION_TYPE_EVENT,
	CONDITION_TYPE_ONT,
	CONDITION_TYPE_GENERAL
} ConditionType_t;

typedef enum ConditionPollType
{
	CONDITION_POLL_TYPE_TIME_DRIVEN = 0,
	CONDITION_POLL_TYPE_EVENT_DRIVEN
} ConditionPollType_t;

class Condition
{
	private:
		ConditionType_t m_type;
		/* 
		 * a condition string
		 * eg: get(lighting1, get_status, X, 0) OR 
		 * do(lighting1//dev//, set_status//command//, 1//val//, 0//ts//, 43//ruleId//)
		 * here, this cond is a "get" cond and requests 
		 * the data for lighting1 from the DataManager
		 */
		string m_condStr;

		/*
		 * data-requestor.
		 * Every "get" condition will have a dev name and command
		 * which would be the data-requestor and request 
		 * data from the DataManager.
		 * In the above example "lighting1|get_status" is the dataRequestor
		 */
		string m_devStr;
		string m_commandStr;
		string m_valueStr;
		string m_tsStr;

		/* 
		 * In case this represents a Head condition i.e. a "do()/event()" predicate
		 * we would have an extra 5th arg stating which Rule it reqresents.
		 */
		string m_idStr;

		/*
		 * A get condition needs to be resolved via the dataManager.
		 * This can be Time-driven (periodic polling) or Event-Driven
		 */
		ConditionPollType_t m_pollType;

		/*
		 * In case the condition is Event Driven, we need access to 
		 * its DBusProxy object when registering the signal handler
		 */
		DBusGProxy *m_dbus_obj;

		/*
		 * To get the proxy obj we need the Interface and Service name
		 * of the Dbus Obj. We also need the obj path name, but that can be
		 * corresponds to the devStr itself.
		 */
		string m_dbus_iface_name;
		string m_dbus_service_name;
	
	public:
		Condition()
		{
		}

		Condition(Condition *c) : m_condStr(c->CondStr()), 
								  m_devStr(c->DevStr()),
								  m_commandStr(c->CommandStr()),
								  m_valueStr(c->ValueStr()),
								  m_tsStr(c->TsStr()),
								  m_idStr(c->IdStr()),
								  m_dbus_obj(c->DbusObj())
		{
			m_type = c->type();
		}

		Condition(string& str) : m_type(CONDITION_TYPE_GENERAL), m_condStr(str)
		{
			cout << "New Condition: " << m_condStr << endl;
		}

		~Condition()
		{
		}

		void type(ConditionType_t type)
		{
			m_type = type;
		}

		ConditionType_t type()
		{
			return m_type;
		}

		string& CondStr()
		{
			return m_condStr;
		}
		
		void CondStr(string s)
		{
			m_condStr.assign(s);
		}

		void DevStr(const char *devname)
		{
			m_devStr.assign(devname);
		}

		string& DevStr()
		{
			return m_devStr;
		}

		void CommandStr(const char *commandStr)
		{
			m_commandStr.assign(commandStr);
		}

		string& CommandStr()
		{
			return m_commandStr;
		}

		string& ValueStr()
		{
			return m_valueStr;
		}

		string& TsStr()
		{
			return m_tsStr;
		}
		
		string& IdStr()
		{
			return m_idStr;
		}

		string& DbusServiceName()
		{
			return m_dbus_service_name;
		}

		string& DbusInterfaceName()
		{
			return m_dbus_iface_name;
		}

		string DbusObjPath()
		{
			return (SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+m_devStr);
		}

		void DbusObj(DBusGProxy *obj)
		{
			m_dbus_obj = obj;
		}

		DBusGProxy *DbusObj()
		{
			return m_dbus_obj;
		}

		ConditionPollType_t& PollType()
		{
			return m_pollType;
		}

		friend bool parse_condition(string& s, Condition *c);
		friend struct CmpConditionType;
};

struct CmpConditionType
{
	bool operator()(const Condition *c1, const Condition *c2) const
	{
		string s1(c1->m_devStr + c1->m_commandStr);
		string s2(c2->m_devStr + c2->m_commandStr);
		
		if (s1 < s2)
			return true;

		return false;
	}
};

#endif
