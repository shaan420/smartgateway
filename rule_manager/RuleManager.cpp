#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include "RuleManager.hpp"
#include "url_utils.hpp"

RuleManager *s_rule_manager = NULL;
int g_ruleIdx;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
static bool sig_alarm_flag = 0;

pthread_mutex_t g_dbus_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_dbus_cond = PTHREAD_COND_INITIALIZER;

void sig_alarm_handler(int sig)
{
	alarm(1);
	signal(SIGALRM, sig_alarm_handler); 
	sig_alarm_flag = 1;
	pthread_cond_signal(&g_cond);
}

static void QueryXsbPrologEngine(th_context *th_ctx)
{
	int rc;
	vector<string> delRuleVec;
	vector<string> delEventVec;
	vector<string> assertEventVec;
	XSB_StrDefine(return_string);
	char xsb_str[512];

	/*
	 * Now query the Prolog engine to find conditions that have been satisfied
	 */
	cout << "Querying the XSB Prolog engine.\n";

	rc = xsb_query_string_string(th_ctx, "do(Dev,Command,Value,Ts,RuleId), lastSeenTs(TsPrev), Ts >= TsPrev.", &return_string, "|");
	while (rc == XSB_SUCCESS) 
	{
		printf("Rule Satisfied: %s\n", (return_string.string));
		string text(return_string.string);

		boost::char_separator<char> sep("|");
		boost::tokenizer< boost::char_separator<char> > tokens(text, sep);
		boost::tokenizer< boost::char_separator<char> >::iterator it_tok = tokens.begin();

		string dev = *it_tok;
		it_tok++;

		string com = *it_tok;
		it_tok++;

		string val = *it_tok;
		it_tok++;

		string ts = *it_tok;
		it_tok++;

		string ruleId = *it_tok;

		/* TODO: Can make this async if there are performance issues */
		RULE_MANAGER->ExecuteCommand(dev, com, val);

		/*
		 * Now mark this rule for deletion
		 */
		delRuleVec.push_back(ruleId);

		/* Get the next rule that was satisfied (if at all) */
		rc = xsb_next_string(th_ctx, &return_string, "|");
	}

	rc = xsb_query_string_string(th_ctx, "event(EventName,Zero1,Zero2,Ts,RuleId), lastSeenTs(TsPrev), Ts >= TsPrev.", &return_string, "|");
	while (rc == XSB_SUCCESS) 
	{
		printf("Rule Satisfied: %s\n", (return_string.string));
		string text(return_string.string);

		boost::char_separator<char> sep("|");
		boost::tokenizer< boost::char_separator<char> > tokens(text, sep);
		boost::tokenizer< boost::char_separator<char> >::iterator it_tok = tokens.begin();

		string eventName = *it_tok;
		it_tok++;

		//string com = *it_tok;
		it_tok++;

		//string val = *it_tok;
		it_tok++;

		string ts = *it_tok;
		it_tok++;

		string ruleId = *it_tok;

		/* TODO: Can make this async if there are performance issues */
		RULE_MANAGER->PublishEvent(eventName, ts);

		/*
		 * This event needs to be asserted back to the Prolog Database, so 
		 * add it to the asserEventVec. But, the rule that caused the event needs 
		 * to be deleted (in case its a "OneShot" rule otherwise just delete its 
		 * get conditions just like in the case of normal rules).
		 */
		char event_str[256];
		snprintf(event_str, 256, "event(%s, 0, 0, %s, %s)", eventName.c_str(), ts.c_str(), ruleId.c_str());
		assertEventVec.push_back(string(event_str));

		delEventVec.push_back(ruleId);

		/* Get the next rule that was satisfied (if at all) */
		rc = xsb_next_string(th_ctx, &return_string, "|");
	}	

	BOOST_FOREACH(string& s, assertEventVec)
	{
		/*
		 * Assert the events into the Prolog Database
		 */
		snprintf(xsb_str, 512, "assert( (%s) ).", s.c_str());
		cout << "Asserting event " << xsb_str << endl;
		if (xsb_command_string(th_ctx, xsb_str) == XSB_ERROR)
		{
			fprintf(stderr,"++Error asserting condition %s: %s/%s\n", xsb_str,
					xsb_get_error_type(th_ctx),
					xsb_get_error_message(th_ctx));
		}
	}

	BOOST_FOREACH(string& s, delEventVec)
	{
		/*
		 * Now remove this rule and all its conditions if it was "OneShot"
		 * This needs to be done outside the prev loop because XSB can only issue
		 * a new command/query when the prev one is closed (either when all results 
		 * of a query are read or explicitly called xsb_query_close() ).
		 * Also, mark this call as "true" as it originates from the workerThread.
		 */
		RULE_MANAGER->DeleteRule(s.c_str(), true);
	}


	BOOST_FOREACH(string& s, delRuleVec)
	{
		/*
		 * Now remove this rule and all its conditions if it was "OneShot"
		 * This needs to be done outside the prev loop because XSB can only issue
		 * a new command/query when the prev one is closed (either when all results 
		 * of a query are read or explicitly called xsb_query_close() ).
		 * Also, mark this call as "true" as it originates from the workerThread.
		 */
		RULE_MANAGER->DeleteRule(s.c_str(), true);
	}

	/*
	 * Now empty the vectors
	 */
	delRuleVec.clear();
	assertEventVec.clear();
	delEventVec.clear();
	return;
}

void *time_driven_worker_func(void *arg)
{
	th_context *th_ctx = (th_context *)arg;
	sigset_t mask;
	string xsbCommandStr;
	char xsb_str[512];

	/* setup SIGALRM to wake up every second */
	sigfillset(&mask);
	sigdelset(&mask, SIGALRM);
	alarm(1);
	signal(SIGALRM, sig_alarm_handler);

	while (1)
	{
		cout << "Suspending the thread until next second.\n";
		//sigsuspend(&mask);
		pthread_cond_wait(&g_cond, &g_mutex);

		cout << "Worker thread invoked\n";

		if (RULE_MANAGER->IsRuleMapEmpty())
		{
			cout << "Rule map is empty...nothing to do...\n";
			continue;
		}

		/*
		 * Check for satisfiability of the Rules in the RuleVec
		 */
		RuleManager::DataReqMap_t::iterator it = RULE_MANAGER->TimeDrivenMap().begin();

		for (it = RULE_MANAGER->TimeDrivenMap().begin(); it != RULE_MANAGER->TimeDrivenMap().end(); it++)
		{
			Condition *c = it->first;
			xsbCommandStr = RULE_MANAGER->GetData(c->DevStr(), c->CommandStr(), c->TsStr());
			cout << xsbCommandStr << endl;
			/*
			 * assert the condition into the Prolog database
			 */
			strncpy(xsb_str, xsbCommandStr.c_str(), 511);
			xsb_str[511] = '\0';

			if (xsb_command_string(th_ctx, xsb_str) == XSB_ERROR)
				fprintf(stderr,"++Error asserting condition %s: %s/%s\n", xsb_str,
																		  xsb_get_error_type(th_ctx),
																		  xsb_get_error_message(th_ctx));

			/*
			 * Update the last seen timestamp in the Prolog Database.
			 */
			RULE_MANAGER->UpdateTs(c->TsStr());
		}

		/*
		 * Now query the Prolog engine to find conditions that have been satisfied
		 */
		QueryXsbPrologEngine(RULE_MANAGER->XsbWorkerThreadCtx());
	}

	return NULL;
}

static void changedStatusSignalHandler(DBusGProxy* proxy,
									 const char* value,
									 gpointer userData) 
{
	map<string, string> m;
	map<string, string>::iterator it;
	string devStr, commandStr, tsStr("0"), xsbCommandStr;
	char xsb_str[512];
	th_context *th_ctx = RULE_MANAGER->XsbWorkerThreadCtx();

	cout << "Signal handler invoked with value " << value << endl;

	if (url_key_value_to_map(value, m))
	{
		cout << "Parsing signal value failed " << value << endl;
		return;
	}

	it = m.find("DeviceName");
	if (it == m.end())
	{
		cout << "DeviceName not found!\n";
		return;
	}
	
	devStr = it->second;

	it = m.find("Command");
	if (it == m.end())
	{
		cout << "Command not found!\n";
		return;
	}
	
	commandStr = it->second;

	cout << "Signal arrived for " << devStr << " " << commandStr << endl;

	xsbCommandStr = RULE_MANAGER->GetData(devStr, commandStr, tsStr);
	cout << xsbCommandStr << endl;

	/*
	 * assert the condition into the Prolog database
	 */
	strncpy(xsb_str, xsbCommandStr.c_str(), 511);
	xsb_str[511] = '\0';

	if (xsb_command_string(th_ctx, xsb_str) == XSB_ERROR)
		fprintf(stderr,"++Error asserting condition %s: %s/%s\n", xsb_str,
				xsb_get_error_type(th_ctx),
				xsb_get_error_message(th_ctx));

	/*
	 * Update the last seen timestamp in the Prolog Database.
	 */
	RULE_MANAGER->UpdateTs(tsStr);

	/* Now check if any action or event is triggered */
	QueryXsbPrologEngine(th_ctx);

	return;
}


int RuleManager::UpdateTs(string& tsStr)
{
	/*
	 * First retract the predicate lastSeenTs(X).
	 * Then update it with the latest.
	 */
	char xsb_cmd[128];

	snprintf(xsb_cmd, 128, "retractall( lastSeenTs(_) ). ");

	cout << "Retract Cmd string: " << xsb_cmd << endl;
	if (xsb_command_string(m_worker_th_ctx, xsb_cmd) == XSB_ERROR)
	{
		fprintf(stderr,"++Error retracting conditions. %s/%s\n",	xsb_get_error_type(m_worker_th_ctx),
				xsb_get_error_message(m_worker_th_ctx));

		return -1;
	}

	snprintf(xsb_cmd, 128, "assert( lastSeenTs(%s) ). ", tsStr.c_str());

	cout << "Asserting Cmd string: " << xsb_cmd << endl;
	if (xsb_command_string(m_worker_th_ctx, xsb_cmd) == XSB_ERROR)
	{
		fprintf(stderr,"++Error asserting conditions. %s/%s\n",	xsb_get_error_type(m_worker_th_ctx),
				xsb_get_error_message(m_worker_th_ctx));
		
		return -1;
	}

	cout << "Successfully updated the timestamp in the XSB Prolog database\n";
	return 0;
}

int RuleManager::ExecuteCommand(string& devStr, string& commandStr, string& valueStr)
{
	char command[512];
	char *response = NULL;
	GError *error = NULL;
	snprintf(command, 511, "DeviceName=%s&%s=%s", devStr.c_str(), commandStr.c_str(), valueStr.c_str());

	dbus_g_proxy_call(m_contextManagerObj,
					  "dev_command",
					  &error,
					  G_TYPE_STRING,
					  command,
					  G_TYPE_INVALID,
					  G_TYPE_STRING,
					  response,
					  G_TYPE_INVALID);

	/* TODO: What to do with the response?? */
	if (response) free (response);

	return 0;
}

int RuleManager::PublishEvent(string& eventStr, string& tsStr)
{
	//char event[512];
	char *response = NULL;
	GError *error = NULL;
	//snprintf(event, 512, "EventName=%s&Ts=%s", eventStr.c_str(), tsStr.c_str());

	dbus_g_proxy_call(m_notificationAgentObj,
					  "publish_event",
					  &error,
					  G_TYPE_STRING,
					  eventStr.c_str(),
					  G_TYPE_INVALID,
					  G_TYPE_STRING,
					  response,
					  G_TYPE_INVALID);

	/* TODO: What to do with the response?? */
	if (response) free (response);

	return 0;
}

RuleManager *RuleManager::getInstance()
{
	if (NULL == s_rule_manager)
	{
		s_rule_manager = new RuleManager;
	}

	return s_rule_manager;
}

string RuleManager::GetData(string& devStr, string& commandStr, string& tsStr)
{
	GError *error = NULL;
	char *response = NULL;
	map<string, string> valueMap;
	string condStr;
	string valueStr;
	//int ts; 
	int num_elems = 0;

	//ts = strtol(tsStr.c_str(), NULL, 10);

	dbus_g_proxy_call(m_dataManagerObj,
					  "find",
					  &error,
					  G_TYPE_STRING,
					  devStr.c_str(),
				 	  G_TYPE_STRING,
					  commandStr.c_str(),
					  G_TYPE_INT,
					  0,
					  G_TYPE_INVALID,
					  G_TYPE_STRING,
					  &response,
					  G_TYPE_INT,
					  &num_elems,
					  G_TYPE_INVALID);
	
	if (url_key_value_to_map(response, valueMap))
	{
		cout << "Response from DataManager is malformed." << response << endl;
		return condStr;
	}

	/*
	 * Get value and ts from the response
	 */
	map<string, string>::iterator it;
	it = valueMap.find("Values");
	if (it != valueMap.end())
	{
		valueStr = it->second;
	}

	it = valueMap.find("Ts");
	if (it != valueMap.end())
	{
		tsStr = it->second;
	}

	/*
	 * Now prepare the condition string that can be 
	 * fed to the Prolog engine.
	 */
	char cond[256];
	snprintf(cond, 255, "assert( get(%s, %s, %s, %s) ).", devStr.c_str(),
											    commandStr.c_str(),
												valueStr.c_str(),
												tsStr.c_str());

	if (response) free (response);
	return string(cond);
}

int RuleManager::Init_DBUS()
{
	string name("ruleManager");
	GError* error = NULL;
	guint result;
	DBusGProxy *busProxy;

	g_type_init();

	m_bus = dbus_g_bus_get(DBUS_BUS_SYSTEM, &error);
	if (error != NULL) 
	{
		printf("Error: could not create dbus system-bus\n");
		return -1;
	}

	g_print("got dbus-session-bus successfully.\n");

	busProxy = dbus_g_proxy_new_for_name(m_bus,
			DBUS_SERVICE_DBUS,
			DBUS_PATH_DBUS,
			DBUS_INTERFACE_DBUS);

	if (busProxy == NULL) 
	{
		printf("Error: could not create dbus proxy for name registration\n");
		return -1;
	}
	
	string service_path(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "ruleManager";

	if (!dbus_g_proxy_call(busProxy,
				/* Method name to call. */
				"RequestName",
				/* Where to store the GError. */
				&error,
				/* Parameter type of argument 0. Note that
				   since we're dealing with GLib/D-Bus
				   wrappers, you will need to find a suitable
				   GType instead of using the "native" D-Bus
				   type codes. */
				G_TYPE_STRING,
				/* Data of argument 0. In our case, this is
				   the well-known name for our server
				   example ("org.asu.smartgateway"). */
				service_path.c_str(),
				/* Parameter type of argument 1. */
				G_TYPE_UINT,
				/* Data of argument 0. This is the "flags"
				   argument of the "RequestName" method which
				   can be use to specify what the bus service
				   should do when the name already exists on
				   the bus. We'll go with defaults. */
				0,
				/* Input arguments are terminated with a
				   special GType marker. */
				G_TYPE_INVALID,
				/* Parameter type of return lighting 0.
				   For "RequestName" it is UINT32 so we pick
				   the GType that maps into UINT32 in the
				   wrappers. */
				G_TYPE_UINT,
				/* Data of return lighting 0. These will always
				   be pointers to the locations where the
				   proxy can copy the results. */
				&result,
				/* Terminate list of return lightings. */
				G_TYPE_INVALID)) {
					printf("D-Bus.RequestName RPC failed: %s\n", error->message);
					/* Note that the whole call failed, not "just" the name
					   registration (we deal with that below). This means that
					   something bad probably has happened and there's not much we can
					   do (hence program termination). */
					return -1;
				}

	/* Check the result code of the registration RPC. */
	printf("RequestName returned %d.\n", result);
	g_print("got dbus-proxy-object for name registration.\n");

	m_obj = (RuleManagerObject *)g_object_new(RULEMANAGER_TYPE_OBJECT, NULL);
	if (m_obj == NULL)
	{
		g_print("Failed to create ruleManager gobj instance.\n");
	}

	g_print("Registering RuleManager DbusObject on the D-Bus.\n");

	/* 
	 * The function does not return any status, so can't check for
	 * errors here. 
	 */
	dbus_g_connection_register_g_object(m_bus,
			(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+name).c_str(),
			G_OBJECT(m_obj));

	g_print("%s ready to serve requests\n", name.c_str());

	/*
	 * Now create a proxy object so that we can communicate with 
	 * the DataManager.
	 */
	service_path.assign(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "dataManager";
	string obj_path(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "dataManager";
	string iface_path(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "DataManager";

	DBusGProxy *obj = dbus_g_proxy_new_for_name(m_bus,
									service_path.c_str(), /* name */
									obj_path.c_str(), /* obj path */
									iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	cout << "DBUS Proxy for DataManager created successfully\n";
	m_dataManagerObj = obj;

	/*
	 * We also need to communicate with the ContextManager to execute 
	 * an action on a particular device. Basically the contextManager
	 * will communicate with the deviceAgent to execute the actual 
	 * command as it(contextManager) has access to the underlying details of the 
	 * various device methods.
	 */
	service_path.assign(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "contextManager";
	obj_path.assign(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "contextManager";
	iface_path.assign(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "ContextManager";

	obj = dbus_g_proxy_new_for_name(m_bus,
							service_path.c_str(), /* name */
							obj_path.c_str(), /* obj path */
							iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	cout << "DBUS Proxy for ContextManager created successfully\n";
	m_contextManagerObj = obj;

	/*
	 * Now create a proxy object so that we can communicate with 
	 * the NotificationAgent.
	 */
	service_path.assign(SMARTGATEWAY_SERVICE_PATH_PREFIX);
	service_path += "notificationAgent";
	obj_path.assign(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX);
	obj_path += "notificationAgent";
	iface_path.assign(SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX);
	iface_path += "NotificationAgent";

	obj = dbus_g_proxy_new_for_name(m_bus,
			service_path.c_str(), /* name */
			obj_path.c_str(), /* obj path */
			iface_path.c_str() /* interface path */);

	if (NULL == obj)
	{
		return -1;
	}

	cout << "DBUS Proxy for NotificationAgent created successfully\n";
	m_notificationAgentObj = obj;

	return 0;
}

static void *dbus_offload_thread_func(void *data)
{
	char *params = NULL;
	RuleManager::DbusOffloadQueue_t *q = (RuleManager::DbusOffloadQueue_t *)data;
	if (NULL == q)
	{
		cout << "ERROR: invalid dbus offload queue\n";
		return NULL;
	}

	while(1)
	{
		pthread_cond_wait(&g_dbus_cond, &g_dbus_mutex);

		while (q->pop(params))
		{
			/* 
			 * TODO: We can perform different functionality based on the "params"
			 * but for now just...
			 */
			cout << "Dbus Offload Thread got rule/event: " << params << endl;
			RULE_MANAGER->HandleInsertRule(params);

			free (params);
		}
	}

	return NULL;
}


int RuleManager::Init_XSB()
{
	//int ret;
	char init_string[512];

	/* 
	 * xsb_init_string relies on the calling program to pass the absolute or relative
	 * path name of the XSB installation directory. We assume that the current
	 * program is sitting in the directory ../examples/c_calling_xsb/
	 * To get installation directory, we strip 3 file names from the path. 
	 */

	strcpy(init_string, "/home/root/XSB/");

	if (xsb_init_string(init_string)) 
	{
		fprintf(stderr,"%s initializing XSB: %s\n", xsb_get_init_error_type(),
													xsb_get_init_error_message());
		exit(XSB_ERROR);
	}

	/* get xsb main thread context */
	m_main_th_ctx = xsb_get_main_thread();

	/* create xsb worker thread context */
	xsb_ccall_thread_create(m_main_th_ctx, &m_worker_th_ctx);

	/* create worker thread */
	m_worker_th = g_thread_new("XSB-TimeDrivenWorkerThread", &time_driven_worker_func, (gpointer)m_worker_th_ctx);
	m_dbus_offload_th = g_thread_new("XSB-DbusOffloadThread", &dbus_offload_thread_func, (gpointer)&m_dbus_offload_queue);
	//ret = pthread_create(&m_worker_th, NULL, worker_func, m_worker_th_ctx);
	if (NULL == m_worker_th)
	//if (-1 == ret)
	{
		printf("g_thread_new failed\n");
		return -1;
	}

	return 0;
}

int RuleManager::InsertRule(const char *rule_params)
{
	char *params = NULL;

	params = strdup(rule_params);

	m_dbus_offload_queue.push(params);

	pthread_cond_signal(&g_dbus_cond);

	return 0;
}

int RuleManager::HandleInsertRule(const char *rule_params)
{
	Rule *r = NULL;
	const char *rule_str = NULL;
	map<string, string> params;
	map<string, string>::iterator it;

	if (url_key_value_to_map(rule_params, params))
	{
		cout << "ERROR: parsing rule_params failed\n";
		return -1;
	}

	it = params.find("Rule");
	if (it == params.end())
	{
		it = params.find("Event");
		if (it == params.end())
		{
			cout << "ERROR: Rule/Event string not present in rule_params\n";
			return -1;
		}
	}
	
	rule_str = it->second.c_str();
	r = new Rule(rule_str);

	it = params.find("IsOneShot");
	if (it == params.end())
	{
		/* By default consider it is oneShot */
		r->IsOneShot(true);
	}
	else
	{
		r->IsOneShot((it->second == "false") ? false : true);
	}

	if (false == parse_condition(r->Head()->CondStr(), r->Head()))
	{
		cout << "Malformed head condition in rule\n";
		delete r;
		return -1;
	}

	BOOST_FOREACH(Condition *c, r->CondVec())
	{
		/* 
		 * for each condition that is a "get" i.e. a data-requestor,
		 * insert the dbus name (i.e. the first argument which is also the 
		 * the device instance name) into the dataReqMap. Also increment its 
		 * consumer count (which is the value of that entry in the map).
		 */
		switch(c->type())
		{
			case CONDITION_TYPE_GET:
				{
					/* extract the dev name and command from the condition */
					if (parse_condition(c->CondStr(), c))
					{
						/* 
						 * got a valid name.
						 * check if this data requestor is already present 
						 * in the map. If yes, then just increment its consumer 
						 * count. If not, then add a new entry with count 1.
						 * This DataRequestor string is formed by concatenating
						 * the device and command strings seperated by a "|".
						 * TODO: take lock to update map by making AddDataRequestor function
						 */
						cout << c->DevStr() << " " << c->CommandStr() << endl;

						RULE_MANAGER->SetupConditionPolling(c);
						
						cout << "Got valid condition in rule: " << c->CondStr() << endl;
					}
					else
					{
						cout << "Malformed condition in rule" << endl;
						delete r;
						return -1;
					}
				}
				break;

			case CONDITION_TYPE_EVENT:
				{
					if (false == parse_condition(c->CondStr(), c))
					{
						cout << "Malformed condition in rule" << endl;
						delete r;
						return -1;
					}
				}
				break;

			default:
				break;
		}
	}

	r->id(g_ruleIdx);

	/*
	 * Also insert the rule into the running Prolog database.
	 */
	char head[256];
	if (r->Head()->type() == CONDITION_TYPE_DO)
	{
		snprintf(head, 256, "do(%s, %s, %s, %s, %d) ", r->Head()->DevStr().c_str(),
													   r->Head()->CommandStr().c_str(),
													   r->Head()->ValueStr().c_str(),
													   "Ts",
													   r->id());
	}
	else if (r->Head()->type() == CONDITION_TYPE_EVENT)
	{
		snprintf(head, 256, "event(%s, %s, %s, %s, %d) ", r->Head()->DevStr().c_str(),
													      r->Head()->CommandStr().c_str(),
													      r->Head()->ValueStr().c_str(),
													      "Ts",
													      r->id());
	}
	else
	{
		cout << "ERROR: unsupported condition type in head\n";
		delete r;
		return -1;
	}

	r->AssertedRuleStr().assign(head);
	const char *conditions_start = strstr(rule_str, ":-");
	if (conditions_start)
	{
		r->AssertedRuleStr() += conditions_start;
	}

	char xsb_str[512];
	snprintf(xsb_str, 512, "assert( (%s) ).", r->AssertedRuleStr().c_str());

	cout << "Inserting rule string: " << xsb_str << endl;
	if (xsb_command_string(m_worker_th_ctx, xsb_str) == XSB_ERROR)
		fprintf(stderr,"++Error inserting rule. %s/%s\n",	xsb_get_error_type(m_worker_th_ctx),
															xsb_get_error_message(m_worker_th_ctx));

	cout << "Rule inserted successfully into the XSB Prolog database\n";

	return RULE_MANAGER->RuleMapInsert(r);
}

int RuleManager::RuleMapInsert(Rule *r)
{
	/* TODO: Take write lock */
	m_ruleMap.insert(make_pair<int, Rule*>(g_ruleIdx, r));

	g_ruleIdx = (g_ruleIdx == INT_MAX) ? 0 : g_ruleIdx+1;

	return g_ruleIdx;
}

int RuleManager::GetDeviceInfo(Condition *c)
{
	GError *error = NULL;
	char *response = NULL;
	char command[256];
	map<string, string> m;
	map<string, string>::iterator it;
	string d, commandStr, tsStr("0"), xsbCommandStr;

	snprintf(command, 256, "DeviceName=%s&Command=%s", c->DevStr().c_str(), c->CommandStr().c_str());

	dbus_g_proxy_call(m_contextManagerObj,
			"device_info",
			&error,
			G_TYPE_STRING,
			command,
			G_TYPE_INVALID,
			G_TYPE_STRING,
			&response,
			G_TYPE_INVALID);

	if (NULL == response)
	{
		cout << "ERROR: GetDeviceInfo Failed\n";
		return -1;
	}

	cout << "Response device info: " << response << endl;

	if (url_key_value_to_map(response, m))
	{
		cout << "ERROR: Parsing device info failed " << endl;
		return -1;
	}

	it = m.find("ServicePath");
	if (it == m.end())
	{
		cout << "ServicePath not found!\n";
		return -1;
	}

	c->DbusServiceName().assign(it->second);

	it = m.find("ObjPath");
	if (it == m.end())
	{
		cout << "ObjPath not found!\n";
		return -1;
	}
	
	//c->DbusServiceName().assign(it->second);
	cout << "Obj path: " << it->second << endl;

	it = m.find("IfacePath");
	if (it == m.end())
	{
		cout << "IfacePath not found!\n";
		return -1;
	}

	c->DbusInterfaceName().assign(it->second);

	it = m.find("PollType");
	if (it == m.end())
	{
		cout << "PollType not found!\n";
		return -1;
	}

	if (string::npos != it->second.find("event-driven"))
	{
		c->PollType() = CONDITION_POLL_TYPE_EVENT_DRIVEN;
	}
	else
	{
		c->PollType() = CONDITION_POLL_TYPE_TIME_DRIVEN;
	}

	free (response);

	return 0;
}

int RuleManager::SetupConditionPolling(Condition *c)
{
	/* Get details of device from ContextManager */
	GetDeviceInfo(c);

	switch (c->PollType())
	{
		case CONDITION_POLL_TYPE_TIME_DRIVEN:
			SetupTimeDrivenCondition(c);
			break;

		case CONDITION_POLL_TYPE_EVENT_DRIVEN:
			SetupEventDrivenCondition(c);
			break;

		default:
			cout << "ERROR: Unsupported Condition Poll Type\n";
			return -1;
	}

	return 0;
}

int RuleManager::SetupTimeDrivenCondition(Condition *c)
{
	DataReqMap_t::iterator it;
	it = TimeDrivenMap().find(c);
	if (it != TimeDrivenMap().end())
	{
		/* already exists, increment consumer count */
		it->second++;
		cout << "Consumer count for " << c->CondStr() << " incremented to " << it->second << endl;
	}
	else
	{
		/* add new entry with count 1 */
		TimeDrivenMap().insert(make_pair<Condition*, int>(new Condition(c), 1));
		cout << "Consumer count set to 1\n";
	}

	return 0;
}

int RuleManager::SetupEventDrivenCondition(Condition *c)
{
	DataReqMap_t::iterator it;
	it = EventDrivenMap().find(c);
	if (it != EventDrivenMap().end())
	{
		/* already exists, increment consumer count */
		it->second++;
		cout << "Consumer count for " << c->CondStr() << " incremented to " << it->second << endl;
	}
	else
	{
		/* Setup the dbus proxy for the device */
		DBusGProxy *remoteValue =
							dbus_g_proxy_new_for_name(m_bus,
							c->DbusServiceName().c_str(), /* name */
							c->DbusObjPath().c_str(), /* obj path */
							c->DbusInterfaceName().c_str() /* interface */);

		if (remoteValue == NULL) {
			printf("Couldn't create the proxy object unknown(dbus_g_proxy_new_for_name)\n");
			return -1;
		}

		c->DbusObj(remoteValue);

		/* Now register the signal */
		dbus_g_proxy_add_signal(/* Proxy to use */
				remoteValue,
				/* Signal name */
				SIGNAL_CHANGED_STATUS,
				/* Will receive one string argument */
				G_TYPE_STRING,
				/* Termination of the argument list */
				G_TYPE_INVALID);

		/* Now connect the signal to register the callback */
		dbus_g_proxy_connect_signal(remoteValue, SIGNAL_CHANGED_STATUS,
				G_CALLBACK(changedStatusSignalHandler),
				NULL, NULL);

		/* add new entry with count 1 */
		EventDrivenMap().insert(make_pair<Condition*, int>(new Condition(c), 1));
		cout << "Consumer count set to 1\n";
	}

	return 0;
}


int RuleManager::DeleteRule(const char *ruleId, bool fromWorkerThread)
{
	int id = atoi(ruleId);

	RuleMap_t::iterator it;
 	DataReqMap_t::iterator it_cond;

	it = m_ruleMap.find(id);

	if (it == m_ruleMap.end())
	{
		cout << "Rule not found. ID=" << ruleId << endl;
		return -1;
	}

	Rule *r = it->second;

	BOOST_FOREACH(Condition *c, r->CondVec())
	{
		if (c->type() == CONDITION_TYPE_GET)
		{
			DataReqMap_t *m;
			if (c->PollType() == CONDITION_POLL_TYPE_TIME_DRIVEN)
			{
				m = &TimeDrivenMap();
			}
			else if (c->PollType() == CONDITION_POLL_TYPE_EVENT_DRIVEN)
			{
				m = &EventDrivenMap();
			}
			else
			{
				cout << "ERROR: Unknown condition poll type during delete rule\n";
				return-1;
			}

			it_cond = m->find(c);
			if (it_cond != m->end())
			{
				if (r->IsOneShot() || !fromWorkerThread)
				{
					/* should exist, decrement consumer count */
					it_cond->second--;
				}

				if (it_cond->second <= 0)
				{
					/*
					 * No more consumers
					 */
					cout << "No more consumers so erasing from DataReqMap: " << c->DevStr() << "," << c->CommandStr() << endl;
					delete it_cond->first;
					m->erase(it_cond);

					/* retract() this condition from the Prolog database */
					char retractCmd[256];
					snprintf(retractCmd, 256, "retractall( get(%s, %s, _, _) ). ", c->DevStr().c_str(),
							c->CommandStr().c_str());

					cout << "Retract Cmd string: " << retractCmd << endl;
					if (xsb_command_string(m_worker_th_ctx, retractCmd) == XSB_ERROR)
						fprintf(stderr,"++Error retracting conditions. %s/%s\n",	xsb_get_error_type(m_worker_th_ctx),
								xsb_get_error_message(m_worker_th_ctx));

					cout << "Successfully retracted conditions from the XSB Prolog database\n";
				}

				if (c->PollType() == CONDITION_POLL_TYPE_EVENT_DRIVEN)
				{
					/* Unsubscribe for the signal from this device */
					dbus_g_proxy_disconnect_signal(c->DbusObj(), SIGNAL_CHANGED_STATUS,
							G_CALLBACK(changedStatusSignalHandler),
							NULL);
				}
			}
			else
			{
				/* How the hell did this happen?! Threading issue, race conditions??! */
				cout << "ERROR: Did not find data requestor in map!" << endl;
			}
		}
	}
	
	if ((false == r->IsOneShot()) && fromWorkerThread)
	{
		cout << "Recurring rule not deleting\n";
		return 0;
	}

	/*
	 * Now retract this rule from the XSB Prolog database
	 */
	char xsb_str[512];
	snprintf(xsb_str, 512, "retract( (%s) ).", r->AssertedRuleStr().c_str());

	cout << "Retracting rule string: " << xsb_str << endl;
	if (xsb_command_string(m_worker_th_ctx, xsb_str) == XSB_ERROR)
		fprintf(stderr,"++Error retracting rule. %s/%s\n",	xsb_get_error_type(m_worker_th_ctx),
															xsb_get_error_message(m_worker_th_ctx));

	cout << "Rule retracted successfully from the XSB Prolog database\n";

	delete r;
	m_ruleMap.erase(it);

	return 0;
}

int RuleManager::FetchAllRules(char **response)
{
	char buf[33];
	string resp;

	RuleMap_t::iterator it = m_ruleMap.begin();
	while (it != m_ruleMap.end())
	{
		Rule *r = it->second;
		snprintf(buf, sizeof(buf), "%d", r->id());
		resp += buf;
		resp += "\t";
		resp += r->String();
		resp += "\n";
		it++;
	}

	*response = strdup(resp.c_str());
	return 0;
}

bool RuleManager::IsRuleMapEmpty()
{
	return m_ruleMap.empty();
}

bool parse_condition(std::string& condStr, Condition *c)
{
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	using boost::spirit::qi::_1;
	using boost::spirit::qi::phrase_parse;
	using boost::spirit::ascii::space;
	using boost::phoenix::ref;
	using qi::lit;
	using qi::lexeme;
	using ascii::char_;

	std::cout << "Parsing condition: " << condStr << std::endl;
	qi::rule<std::string::iterator, std::string(), ascii::space_type> csv_string;
	csv_string %= lexeme[+(char_ - ')' - ',')];
	bool r = phrase_parse(condStr.begin(), condStr.end(),

			//  Begin grammar
			(
			 (lit("get") | lit("do") | lit("event")) >> 
			 '(' >> 
			 csv_string[ref(c->m_devStr) = _1] >> ',' >>
			 csv_string[ref(c->m_commandStr) = _1] >> ',' >>
			 csv_string[ref(c->m_valueStr) = _1] >> ',' >>
			 csv_string[ref(c->m_tsStr) = _1] >> 
			 -(',' >> csv_string[ref(c->m_idStr) = _1]) >> 
			 ')'
			),
			//  End grammar

			space);

	return r;
}


