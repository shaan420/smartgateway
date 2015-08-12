#include "groveLightingDevice.hpp"
#include "DeviceAgent.hpp"
#include <upm/grove.h>

int groveLightingDevice::RecvFromDevice()
{
	return 0;
}

int groveLightingDevice::SendToDevice()
{
	upm::GroveLed *light = (upm::GroveLed *)m_handler;

	cout << *(int *)m_writeBuf << endl;

	m_prev_value = m_cur_value;

	if (*(int *)m_writeBuf == 0)
	{
		light->off();
		m_cur_value.assign("0");
	}
	else
	{
		light->on();
		m_cur_value.assign("1");
	}
	
	delete (int *)m_writeBuf;
	return 0;
}

int groveLightingDevice::Read(void *mem, int len)
{
	return 0;
}

int groveLightingDevice::Write(void *mem, int len)
{
	/* Allocate for writeBuf here */
	m_writeBuf = new int;
	*(int *)m_writeBuf = *(int *)mem;

	/* 
	 * The following call is important as it notifies the sender thread
	 * that the write-data is ready to be written
	 */
	DeviceBase::Write(mem, len);
	return 0;
}

int groveLightingDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void groveLightingDevice::SetDeviceParams()
{
	return;
}

void groveLightingDevice::GetDeviceParams()
{
	return;
}

int groveLightingDevice::OnInit()
{
	int pin = -1;

	/* 
	 * TODO: Get the GPIO Pin number from the m_commParams 
	 * The m_commParams should ideally be populated from the ontology.
	 */

	pin = strtol((m_deviceConf.m_commParams.c_str()+1), NULL, 10);

	if (pin == -1)
	{
		cout << "Error: Could not initialize GPIO device\n";
		return -1;
	}

	upm::GroveLed *light = new upm::GroveLed(pin);

	if (light == NULL)
	{
		cout << "Memory Allocation failed for groveLightingDevice" << endl;
		return -1;
	}

	light->off();
	m_cur_value.assign("0");

	m_handler = (void *)light;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool groveLightingDevice::HasChanged()
{
	if (m_prev_value != m_cur_value)
	{
		return true;
	}
	else
	{
		return false;
	}

	return true;
}

void groveLightingDevice::OnChange()
{
	cout << "New Value: " << m_cur_value << endl;

	/* Update the D-Bus LightingObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	lighting_object_emitSignal(GetObj(), E_LIGHTING_OBJECT_SIGNAL_CHANGED_STATUS, "DeviceName=bedroomLighting1&Command=get_status");
	
	m_prev_value = m_cur_value;

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=lighting1&Command=get_status");
}

void groveLightingDevice::OnError()
{
	return;
}

void groveLightingDevice::OnSendDone()
{
	return;
}

int groveLightingDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int groveLightingDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
