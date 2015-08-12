#include "NotificationAgentDevice.hpp"


int NotificationAgentDevice::RecvFromDevice()
{
	return 0;
}

int NotificationAgentDevice::SendToDevice()
{
	// TODO: Handle the actual low level event notification

	Event *e = (Event *)m_writeBuf;

	e->Publish(m_val);

	return 0;
}

int NotificationAgentDevice::Read(void *mem, int len)
{
	return 0;
}

int NotificationAgentDevice::Write(void *mem, int len)
{
	map<string, string> m;
	map<string, string>::iterator mit;

	url_key_value_to_map((const char *)mem, m);

	mit = m.find("EventName");
	if (m.end() == mit)
	{
		cout << "ERROR: EventName not found: " << mem << endl;
		return -1;
	}

	const char *ev_name = mit->second.c_str();

	mit = m.find("Val");
	if (m.end() == mit)
	{
		cout << "ERROR: Value not found: " << mem << endl;
		return -1;
	}

	m_val.assign(mit->second);

	/* 
	 * Set writeBuf here.
	 * The WriteBuf should actually point to the 
	 * appropriate event structure. 
	 */
	cout << "Triggered event: " << ev_name << endl;
	EventMap_t::iterator it = m_map.find(ev_name);
	if (it == m_map.end())
	{
		cout << "ERROR: Event not found in map\n";
		return 0;
	}

	cout << "Found Event Struct for " << ev_name << endl;
	m_writeBuf = (void *)(it->second);

	/* 
	 * The following call is important as it notifies the sender thread
	 * that the write-data is ready to be written
	 */
	DeviceBase::Write(mem, len);
	return 0;
}

int NotificationAgentDevice::ReadShared(void **data)
{
	return 0;
}

void NotificationAgentDevice::SetDeviceParams()
{
	return;
}

void NotificationAgentDevice::GetDeviceParams()
{
	return;
}

int NotificationAgentDevice::OnInit()
{
	/* TODO: initializations for SMS/Email/Cloud */
	Subscriber::Init();

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool NotificationAgentDevice::HasChanged()
{
	return false;
}

void NotificationAgentDevice::OnChange()
{
}

void NotificationAgentDevice::OnError()
{
	return;
}

void NotificationAgentDevice::OnSendDone()
{
	return;
}

int NotificationAgentDevice::PrepareOutputData(string& output)
{
	return 0;
}

int NotificationAgentDevice::AddEvent(const char *event_params)
{
	map<string, string> m;
	map<string, string>::iterator mit;

	url_key_value_to_map(event_params, m);

	mit = m.find("EventName");
	if (m.end() == mit)
	{
		cout << "ERROR: Insert event request malformed\n";
		return 0;
	}

	Event *e = new Event(mit->second.c_str());

	m_map.insert(make_pair<string, Event*>(mit->second, e));

	return 0;
}

int NotificationAgentDevice::AddSubscriberForEvent(const char *sub_params)
{
	map<string, string> m;
	map<string, string>::iterator mit;

	url_key_value_to_map(sub_params, m);

	mit = m.find("EventName");
	if (m.end() == mit)
	{
		cout << "ERROR: Add sub request malformed\n";
		return -1;
	}

	Event *e;
	const char *event = mit->second.c_str();
	EventMap_t::iterator it = m_map.find(event);

	if (it == m_map.end())
	{
		/* Event not found */
		// TODO: create if not present ??!
		cout << "ERROR: event not found\n";
		return -1;
	}

	e = it->second;
	
	e->AddSubscriber(sub_params);

	return 0;
}

int NotificationAgentDevice::UpdateDataManager()
{
	//DeviceBase::UpdateDataManager();
	return 0;
}
