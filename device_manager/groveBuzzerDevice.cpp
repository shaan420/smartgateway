#include "groveBuzzerDevice.hpp"
#include "DeviceAgent.hpp"
#include "upm/buzzer.h"

static int chord[] = { DO, RE, MI, FA, SOL, LA, SI, DO, SI };

int groveBuzzerDevice::RecvFromDevice()
{
	m_prev_value = m_cur_value;

	if(false == m_status_on)
	{
		m_cur_value.assign("0");
	}
	else
	{
		m_cur_value.assign("1");
	}

	return 0;
}

int groveBuzzerDevice::SendToDevice()
{
	m_status_on = true;
	int n = *(int *)m_writeBuf;
	cout << "Playing Buzzer " << n << " time(s)." << endl;
	delete (int *)m_writeBuf;

	upm::Buzzer *sound = (upm::Buzzer *)m_handler;
	
	for (int i = 0; i < n; i++)
	{
		sound->playSound(chord[0], 1000000);
		sound->stopSound();
	}
	
	m_status_on = false;
	return 0;
}

int groveBuzzerDevice::Read(void *mem, int len)
{
	return 0;
}

int groveBuzzerDevice::Write(void *mem, int len)
{
	/* Allocate for writeBuf here */
	if (m_status_on) return 0;

	m_writeBuf = new int;
	*(int *)m_writeBuf = *(int *)mem;

	/* 
	 * The following call is important as it notifies the sender thread
	 * that the write-data is ready to be written
	 */
	DeviceBase::Write(mem, len);
	return 0;
}

int groveBuzzerDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void groveBuzzerDevice::SetDeviceParams()
{
	return;
}

void groveBuzzerDevice::GetDeviceParams()
{
	return;
}

int groveBuzzerDevice::OnInit()
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

	upm::Buzzer *sound = new upm::Buzzer(pin);

	if (sound == NULL)
	{
		cout << "Memory Allocation failed for groveSoundSensor" << endl;
		return -1;
	}

	sound->stopSound();
	sound->setVolume(0.02);

	m_handler = (void *)sound;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool groveBuzzerDevice::HasChanged()
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

void groveBuzzerDevice::OnChange()
{
	cout << "New Value: " << m_cur_value << endl;

	/* Update the D-Bus LightingObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	//sound_object_emitSignal(GetObj(), E_SOUND_OBJECT_SIGNAL_CHANGED_STATUS, "DeviceName=lighting1&Command=get_status");

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=lighting1&Command=get_status");
}

void groveBuzzerDevice::OnError()
{
	return;
}

void groveBuzzerDevice::OnSendDone()
{
	return;
}

int groveBuzzerDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int groveBuzzerDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
