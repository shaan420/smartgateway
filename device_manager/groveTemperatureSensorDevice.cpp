#include "groveTemperatureSensorDevice.hpp"
#include "DeviceAgent.hpp"
#include <boost/lexical_cast.hpp>
#include <upm/grove.h>

int groveTemperatureSensorDevice::RecvFromDevice()
{
	upm::GroveTemp *temp = (upm::GroveTemp *)m_handler;

	int celsius = temp->value();
	int fahrenheit = (int) (celsius * 9.0/5.0 + 32.0);

	m_prev_value = m_cur_value;
	
	m_cur_value = boost::lexical_cast<std::string>(fahrenheit);

	return 0;
}

int groveTemperatureSensorDevice::SendToDevice()
{
	return 0;
}

int groveTemperatureSensorDevice::Read(void *mem, int len)
{
	return 0;
}

int groveTemperatureSensorDevice::Write(void *mem, int len)
{
	return 0;
}

int groveTemperatureSensorDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void groveTemperatureSensorDevice::SetDeviceParams()
{
	return;
}

void groveTemperatureSensorDevice::GetDeviceParams()
{
	return;
}

int groveTemperatureSensorDevice::OnInit()
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

	upm::GroveTemp* temp = new upm::GroveTemp(pin);

	if (temp == NULL)
	{
		cout << "Memory Allocation failed for groveTemperatureSensor" << endl;
		return -1;
	}

	m_handler = (void *)temp;
	return 0;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool groveTemperatureSensorDevice::HasChanged()
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

void groveTemperatureSensorDevice::OnChange()
{
	cout << "grove Temperature Sensor value changed to " << m_cur_value << endl;

	/* Update the D-Bus TemperatureObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	// TODO: Temperature Sensor is a Time-driven device so no need for signal handler support
	//temperature_object_emitSignal(GetObj(), E_TEMPERATURE_OBJECT_SIGNAL_CHANGED_STATUS, "DeviceName=temperature1&Command=get_status");

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=temperature1&Command=get_status");
}

void groveTemperatureSensorDevice::OnError()
{
	return;
}

void groveTemperatureSensorDevice::OnSendDone()
{
	return;
}

int groveTemperatureSensorDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int groveTemperatureSensorDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
