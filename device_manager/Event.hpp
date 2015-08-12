#ifndef __EVENT_HPP__
#define __EVENT_HPP__
#include <map>
#include "Subscriber.hpp"
using namespace std;

static int g_fieldId = 1;

class Event
{
	
	public:
		typedef map<string, Subscriber*> SubscriberMap_t;

	private:
		string m_name;
		SubscriberMap_t m_map;

		/* 
		 * Each Event corresponds to a seperate field in the 
		 * "SmartHome Events" channel of the ThingSpeak Platform 
		 */
		int m_fieldId;

	public:

		Event() {}
		Event(const char *name) : m_name(name) 
		{
			if (g_fieldId > 8)
			{
				cout << "Event limit reached in ThingSpeak Platform" << endl;				
				return;
			}
			
			m_fieldId = g_fieldId;
			
			ts_add_channel_field(Subscriber::sm_cloud, m_name.c_str(), m_fieldId);

			g_fieldId++;
		}

		int AddSubscriber(const char *subscr_params)
		{
			Subscriber *s = new Subscriber(subscr_params);
			if (s->Type() == SUBSCRIBER_NOTIFICATION_TYPE_CLOUD)
			{
				/* set the field ID as the params */
				s->SetParams(boost::lexical_cast<std::string>(m_fieldId));
			}

			m_map.insert(make_pair<string, Subscriber*>(s->Name(), s));
			return 0;
		}

		int Publish(string& val);

		~Event() {}
};
#endif
