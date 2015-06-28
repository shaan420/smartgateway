#ifndef __EVENT_HPP__
#define __EVENT_HPP__
#include <map>
#include "Subscriber.hpp"
using namespace std;

class Event
{
	
	public:
		typedef map<string, Subscriber*> SubscriberMap_t;

	private:
		string m_name;
		SubscriberMap_t m_map;
		
	public:

		Event() {}
		Event(const char *name) : m_name(name) {}

		int AddSubscriber(const char *subscr_params)
		{
			Subscriber *s = new Subscriber(subscr_params);
			m_map.insert(make_pair<string, Subscriber*>(s->Name(), s));
			return 0;
		}

		int Publish();

		~Event() {}
};
#endif
