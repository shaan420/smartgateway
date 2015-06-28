#include "Event.hpp"

using namespace std;

int Event::Publish()
{
	SubscriberMap_t::iterator it;

	cout << "Publishing event " << m_name << endl;

	if (m_map.empty())
	{
		cout << "No subscribers for event: " << m_name << endl;
		return 0;
	}

	for (it = m_map.begin(); it != m_map.end(); it++)
	{
		it->second->Publish(m_name.c_str());
	}
	
	return 0;
}
