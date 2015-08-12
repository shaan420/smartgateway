#include "groveLightSensorDevice.hpp"
#include "DeviceAgent.hpp"
#include <boost/lexical_cast.hpp>
#include <upm/grove.h>

int groveLightSensorDevice::RecvFromDevice()
{
	upm::GroveLight *light = (upm::GroveLight *)m_handler;

	m_prev_value = m_cur_value;
	
	m_cur_value = boost::lexical_cast<std::string>(light->value());

	return 0;
}

int groveLightSensorDevice::SendToDevice()
{
	return 0;
}

int groveLightSensorDevice::Read(void *mem, int len)
{
	return 0;
}

int groveLightSensorDevice::Write(void *mem, int len)
{
	return 0;
}

int groveLightSensorDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void groveLightSensorDevice::SetDeviceParams()
{
	return;
}

void groveLightSensorDevice::GetDeviceParams()
{
	return;
}

int groveLightSensorDevice::OnInit()
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

	upm::GroveLight *light = new upm::GroveLight(pin);

	if (light == NULL)
	{
		cout << "Memory Allocation failed for groveLightSensor" << endl;
		return -1;
	}

	m_handler = (void *)light;
	return 0;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool groveLightSensorDevice::HasChanged()
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

void groveLightSensorDevice::OnChange()
{
	cout << "grove Light Sensor value changed to " << m_cur_value << endl;

	/* Update the D-Bus LightObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	// TODO: Light Sensor is a Time-driven device so no need for signal handler support
	//temperature_object_emitSignal(GetObj(), E_LIGHT_OBJECT_SIGNAL_CHANGED_STATUS, "DeviceName=temperature1&Command=get_status");

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=temperature1&Command=get_status");
}

void groveLightSensorDevice::OnError()
{
	return;
}

void groveLightSensorDevice::OnSendDone()
{
	return;
}

int groveLightSensorDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int groveLightSensorDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
