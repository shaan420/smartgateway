#ifndef __SUBSCRIBER_HPP__
#define __SUBSCRIBER_HPP__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <curl/curl.h>
#include "url_utils.hpp"

extern "C"
{
#include "thingspeak.h"
#include "ts_http.h"
}

using namespace std;

#define CLOUD_API_KEY "BOOM7YQVSAKY3QMQ"
#define CLOUD_FEED_ID 43433

typedef enum SubscriberNotificationType
{
	SUBSCRIBER_NOTIFICATION_TYPE_SMS = 0,
	SUBSCRIBER_NOTIFICATION_TYPE_EMAIL,
	SUBSCRIBER_NOTIFICATION_TYPE_CLOUD,
	SUBSCRIBER_NOTIFICATION_TYPE_UNSET
} SubscriberNotificationType_t;

class Subscriber
{
	private:
		string m_name;
		SubscriberNotificationType_t m_type;
		string m_params;

		
		int Sendmail(const char *to,
					 const char *message);

		int SendToCloud(const char *message);
	public:
		Subscriber() {}
		Subscriber(const char *name, const char *type, const char *args) : 
			m_name(name), 
			m_params(args)
		{
			if (0 == strncmp(type, "sms", 3))
			{
				m_type = SUBSCRIBER_NOTIFICATION_TYPE_SMS;
			}
			else if (0 == strncmp(type, "email", 5))
			{
				m_type = SUBSCRIBER_NOTIFICATION_TYPE_EMAIL;
			}
			else if (0 == strncmp(type, "cloud", 5))
			{
				m_type = SUBSCRIBER_NOTIFICATION_TYPE_CLOUD;
			}
			else
			{
				m_type = SUBSCRIBER_NOTIFICATION_TYPE_UNSET;
			}
		}

		Subscriber(const char *params)
		{
			map<string, string> m;
			map<string, string>::iterator it;

			url_key_value_to_map(params, m);

			for (it = m.begin(); it != m.end(); it++)
			{
				if (it->first == "Name")
				{
					m_name.assign(it->second);
				}
				else if (it->first == "Type")
				{
					if (it->second == "sms")
					{
						m_type = SUBSCRIBER_NOTIFICATION_TYPE_SMS;
					}
					else if (it->second == "email")
					{
						m_type = SUBSCRIBER_NOTIFICATION_TYPE_EMAIL;
					}
					else if (it->second == "cloud")
					{
						m_type = SUBSCRIBER_NOTIFICATION_TYPE_CLOUD;
					}
					else
					{
						m_type = SUBSCRIBER_NOTIFICATION_TYPE_UNSET;
					}					
				}
				else if (it->first == "Params")
				{
					m_params.assign(it->second);
				}
			}
		}
		
		/*
		 * Notification handlers
		 */
		static CURL *sm_curl;
		static ts_context_t *sm_cloud;

		static int Init()
		{
			/* Initialize Email agent */
			sm_curl = curl_easy_init();

			if (sm_curl == NULL)
			{
				cout << "ERROR: Could not open curl lib\n";
				return -1;
			}

			curl_easy_setopt(sm_curl, CURLOPT_USERNAME, "asu.smarthome@gmail.com");
			curl_easy_setopt(sm_curl, CURLOPT_PASSWORD, "Shankar420");
			curl_easy_setopt(sm_curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
			curl_easy_setopt(sm_curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
			curl_easy_setopt(sm_curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(sm_curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(sm_curl, CURLOPT_MAIL_FROM, "<asu.smarthome@gmail.com>");
			cout << "Email agent initialized successfully\n";

			/* Initialize the Cloud agent */
			sm_cloud = ts_create_context(CLOUD_API_KEY, CLOUD_FEED_ID);

			if (sm_cloud == NULL)
			{
				cout << "ERROR: Could not open thingspeak cloud lib\n";
				return -1;
			}


			return 0;
		}

		int Publish(const char *message);

		const char *Name()
		{
			return m_name.c_str();
		}

		~Subscriber() {}
};
#endif
