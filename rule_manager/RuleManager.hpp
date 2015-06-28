#ifndef __RULE_MANAGER_HPP__
#define __RULE_MANAGER_HPP__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstdlib>
#include <vector>
#include <glib.h>
#include <map>
#include <string>
#include <pthread.h>
#include <dbus/dbus-glib.h>
#include "../common/include/common-defs.h"
#include <boost/lockfree/spsc_queue.hpp>

#define MAX_STRING_LEN 128

using namespace std;
#include "Rule.hpp"

extern "C" {

#include "RuleManagerObject.h"

/* cinterf.h is necessary for the XSB API, as well as the path manipulation routines*/
#include "cinterf.h"
extern char *xsb_executable_full_path(char *);
extern char *strip_names_from_path(char*, int);

/* context.h is necessary for the type of a thread context. */
#include "context.h"
#include "thread_xsb.h"

}
#define RULE_MANAGER s_rule_manager->getInstance()

class RuleManager
{
	public:
		//typedef vector<Rule*> RuleVec_t;
		/*
		 * The RuleMap stores the rules with an associated ID.
		 * This ID is used when we get a DeleteRule request from
		 * the ContextManager.
		 */
		typedef map<int, Rule*> RuleMap_t;

		/*
		 * The Data Requestor Map stores the list of all the 
		 * devices that need to fetch their data from the 
		 * Data Manager and it also stores their associated 
		 * consumer count. This count signifies the number of rules that request 
		 * for data from the same device. This therefore helps in avoiding 
		 * a redundant lookup to the DataManager.
		 */
		typedef map<Condition*, int, CmpConditionType> DataReqMap_t;
		
		typedef boost::lockfree::spsc_queue<char *, boost::lockfree::capacity<1024> > DbusOffloadQueue_t; 

	private:
		/* The GObject representing a D-Bus connection. */
		DBusGConnection *m_bus;
		RuleManagerObject *m_obj;

		/* DBUS Proxy objects to communicate with ContextManager and DataManager*/
		DBusGProxy *m_contextManagerObj;
		DBusGProxy *m_dataManagerObj;
		DBusGProxy *m_notificationAgentObj;

		/* main thread to listen for incoming rule registrations */
		struct th_context *m_main_th_ctx;

		/* worker thread to check if any of the rules have been satisfied */
		//pthread_t m_worker_th;
		GThread *m_worker_th;
		GThread *m_dbus_offload_th;
		struct th_context *m_worker_th_ctx;

		/* Rule Map to hold the registered rules */
		RuleMap_t m_ruleMap;

		/*
		 * Map to hold the data requestors present within a rule
		 * It basically maps the particular data requestor to its 
		 * consumer count.
		 */
		DataReqMap_t m_timeDrivenMap;
		DataReqMap_t m_eventDrivenMap;

		/*
		 * This queue is used to process DBUS messages within a separate 
		 * OffloadThread context so that the Main thread returns quickly.
		 * The main motivation for introducing this was because when the
		 * ContextManager sent an "InsertRule" message, the RuleManager needed
		 * to call the ContextManager again to resolve the PollType of the
		 * "get" conditions in the Rule. Thus creating a deadlock condition.
		 * To avoid this, a separate thread is created.
		 */
		DbusOffloadQueue_t m_dbus_offload_queue;

	public:
		
		RuleManager()
		{
		}

		~RuleManager()
		{
		}

		RuleManager *getInstance();

		DataReqMap_t& TimeDrivenMap()
		{
			return m_timeDrivenMap;
		}

		DataReqMap_t& EventDrivenMap()
		{
			return m_eventDrivenMap;
		}

		int Init_XSB();
		int Init_DBUS();
		int UpdateTs(string& tsStr);

		int Init()
		{
			Init_DBUS();
			Init_XSB();

			return 0;
		}

		/*
		 * Helper functions
		 */
		bool IsRuleMapEmpty();
		int RuleMapInsert(Rule *r);
		th_context *XsbWorkerThreadCtx()
		{
			return m_worker_th_ctx;
		}

		/*
		 * DBUS message handlers
		 */
		int InsertRule(const char *rule_params);
		int DeleteRule(const char *ruleId, bool fromWorkerThread);
		int FetchAllRules(char **response);

		/*
		 * Handle external communication
		 */
		/* This is used to resolve the get conditions in the rules / events via the DataManager */
		string GetData(string& devStr, string& commandStr, string& tsStr);

		/* This is used to execute the device actions via the ContextManager */
		int ExecuteCommand(string& devStr, string& commandStr, string& valueStr);

		/* 
		 * This is used to provide notifications to those who have subscribed 
		 * for an event via the NotificationAgent.
		 */
		int PublishEvent(string& eventStr, string& tsStr);

		/*
		 * Event Driven Status Changes
		 */
		int SetupConditionPolling(Condition *c);
		int SetupTimeDrivenCondition(Condition *c);
		int SetupEventDrivenCondition(Condition *c);

		int GetDeviceInfo(Condition *c);

		/*
		 * DBUS Offload helpers
		 */
		int HandleInsertRule(const char *rule_params);
};

extern RuleManager *s_rule_manager;

#endif
