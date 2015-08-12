#include "groveSoundSensorDevice.hpp"
#include "DeviceAgent.hpp"
#include <boost/lexical_cast.hpp>
#include <upm/groveloudness.h>

int groveSoundSensorDevice::RecvFromDevice()
{
	upm::GroveLoudness *loudness = (upm::GroveLoudness *)m_handler;

	int val = loudness->value();

	m_prev_value = m_cur_value;

	//cout << "Loudness: " << val << endl;
	
	m_cur_value = boost::lexical_cast<std::string>(val);

	return 0;
}

int groveSoundSensorDevice::SendToDevice()
{
	return 0;
}

int groveSoundSensorDevice::Read(void *mem, int len)
{
	return 0;
}

int groveSoundSensorDevice::Write(void *mem, int len)
{
	return 0;
}

int groveSoundSensorDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void groveSoundSensorDevice::SetDeviceParams()
{
	return;
}

void groveSoundSensorDevice::GetDeviceParams()
{
	return;
}

int groveSoundSensorDevice::OnInit()
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

	upm::GroveLoudness* loudness = new upm::GroveLoudness(pin);

	if (loudness == NULL)
	{
		cout << "Memory Allocation failed for groveSoundSensor" << endl;
		return -1;
	}

	m_handler = (void *)loudness;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool groveSoundSensorDevice::HasChanged()
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

void groveSoundSensorDevice::OnChange()
{
	cout << "groveSoundSensor value changed to " << m_cur_value << endl;

	/* Update the D-Bus SoundObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	// TODO: Sound Sensor is a Time-driven device so no need for signal handler support
	//temperature_object_emitSignal(GetObj(), E_SOUND_OBJECT_SIGNAL_CHANGED_STATUS, "DeviceName=temperature1&Command=get_status");

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=temperature1&Command=get_status");
}

void groveSoundSensorDevice::OnError()
{
	return;
}

void groveSoundSensorDevice::OnSendDone()
{
	return;
}

int groveSoundSensorDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int groveSoundSensorDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
