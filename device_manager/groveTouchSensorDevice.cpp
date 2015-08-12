#include "groveTouchSensorDevice.hpp"
#include "DeviceAgent.hpp"
#include <boost/lexical_cast.hpp>
#include <upm/ttp223.h>

int groveTouchSensorDevice::RecvFromDevice()
{
	upm::TTP223 *touch = (upm::TTP223 *)m_handler;

	int val = touch->isPressed();

	m_prev_value = m_cur_value;
	
	m_cur_value = boost::lexical_cast<std::string>(val);

	return 0;
}

int groveTouchSensorDevice::SendToDevice()
{
	return 0;
}

int groveTouchSensorDevice::Read(void *mem, int len)
{
	return 0;
}

int groveTouchSensorDevice::Write(void *mem, int len)
{
	return 0;
}

int groveTouchSensorDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void groveTouchSensorDevice::SetDeviceParams()
{
	return;
}

void groveTouchSensorDevice::GetDeviceParams()
{
	return;
}

int groveTouchSensorDevice::OnInit()
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

	upm::TTP223 *touch = new upm::TTP223(pin);

	if (touch == NULL)
	{
		cout << "Memory Allocation failed for groveTouchSensor" << endl;
		return -1;
	}

	m_handler = (void *)touch;
	return 0;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool groveTouchSensorDevice::HasChanged()
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

void groveTouchSensorDevice::OnChange()
{
	char signal_str[128];
	cout << "grove Touch Sensor: " << m_cur_value << endl;

	/* Update the D-Bus TouchObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	snprintf(signal_str, 128, "DeviceName=%s&Command=get_status", m_name.c_str());

	/* Notify status change */
	touchsensor_object_emitSignal(GetObj(), E_TOUCHSENSOR_OBJECT_SIGNAL_CHANGED_STATUS, signal_str);

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=toucherature1&Command=get_status");
}

void groveTouchSensorDevice::OnError()
{
	return;
}

void groveTouchSensorDevice::OnSendDone()
{
	return;
}

int groveTouchSensorDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int groveTouchSensorDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
