#include "testLightingDevice.hpp"
#include "DeviceAgent.hpp"
#include <cstdlib>
#include <time.h>

#define GPIO 3

int testLightingDevice::RecvFromDevice()
{
	//char value[2];
	m_prev_value = m_cur_value;

	//snprintf(value, 2, "%d", rand()%10);
	//m_cur_value = rand()%10;
	m_cur_value = (m_cur_value >= 9) ? 0 :(m_cur_value+1);
	return 0;
}

int testLightingDevice::SendToDevice()
{
	cout << *(int *)m_writeBuf << endl;

	delete (int *)m_writeBuf;
	return 0;
}

int testLightingDevice::Read(void *mem, int len)
{
	return 0;
}

int testLightingDevice::Write(void *mem, int len)
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

int testLightingDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void testLightingDevice::SetDeviceParams()
{
	return;
}

void testLightingDevice::GetDeviceParams()
{
	return;
}

int testLightingDevice::OnInit()
{

//	srand (time(NULL));

	/* Set the device handler */
	m_handler = (int *)malloc(sizeof(int));
	*(int *)m_handler = 0;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool testLightingDevice::HasChanged()
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

void testLightingDevice::OnChange()
{
	//string changedStr(GetDeviceName());

	//changedStr = "DeviceName=" + changedStr + "&Command=get_status";

	//cout << "New Value: " << m_cur_value << endl;

	/* Update the D-Bus LightingObject */
	//GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	//lighting_object_emitSignal(GetObj(), E_LIGHTING_OBJECT_SIGNAL_CHANGED_STATUS, changedStr.c_str());

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=lighting1&Command=get_status");
}

void testLightingDevice::OnError()
{
	return;
}

void testLightingDevice::OnSendDone()
{
	return;
}

int testLightingDevice::PrepareOutputData(string& output)
{
	char value[4];
	snprintf(value, 4, "%d", m_cur_value);
	output.assign(value);
	return 0;
}

int testLightingDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
