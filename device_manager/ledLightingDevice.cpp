#include "ledLightingDevice.hpp"
#include "DeviceAgent.hpp"

#define GPIO 3

int ledLightingDevice::RecvFromDevice()
{
	int ret;
	char value;
	m_prev_value = m_cur_value;
	ret = pread(*(int *)m_handler, &value, 1, 0);

	if (ret <= 0)
	{
		cout << "Lighting Device pread failed\n";
		return -1;
	}

	if('0' == value)
	{
		m_cur_value.assign("0");
	}
	else if ('1' == value)
	{
		m_cur_value.assign("1");
	}
	return 0;
}

int ledLightingDevice::SendToDevice()
{
	cout << *(int *)m_writeBuf << endl;
	if (*(int *)m_writeBuf == 0)
	{
		pwrite(*(int *)m_handler, "0", 2, 0);
	}
	else
	{
		pwrite(*(int *)m_handler, "1", 2, 0);
	}
	
	delete (int *)m_writeBuf;
	return 0;
}

int ledLightingDevice::Read(void *mem, int len)
{
	return 0;
}

int ledLightingDevice::Write(void *mem, int len)
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

int ledLightingDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void ledLightingDevice::SetDeviceParams()
{
	return;
}

void ledLightingDevice::GetDeviceParams()
{
	return;
}

int ledLightingDevice::OnInit()
{
	char buffer[256];
	int fileHandle;
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

	/* Export GPIO */
	fileHandle = open("/sys/class/gpio/export", O_WRONLY);

	if(-1 == fileHandle)

	{
		puts("Error: cannot open /sys/class/gpio/export");
		return(-1);
	}
	sprintf(buffer, "%d", pin);
	write(fileHandle, buffer, strlen(buffer));
	close(fileHandle);

	/* Direction GPIO */
	sprintf(buffer, "/sys/class/gpio/gpio%d/direction", pin);
	fileHandle = open(buffer, O_WRONLY);
	if(-1 == fileHandle)
	{
		puts("Unable to open file:");
		puts(buffer);
		return(-1);
	}

	/* Set out direction */
	write(fileHandle, "out", 3);

	sprintf(buffer, "/sys/class/gpio/gpio%d/value", pin);

	fileHandle = open(buffer, O_RDWR);

	if(-1 == fileHandle)
	{
		puts("Unable to open file:");
		puts(buffer);
		return(-1);
	}

	/* Set the device handler */
	m_handler = (int *)malloc(sizeof(int));
	*(int *)m_handler = fileHandle;

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool ledLightingDevice::HasChanged()
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

void ledLightingDevice::OnChange()
{
	cout << "New Value: " << m_cur_value << endl;

	/* Update the D-Bus LightingObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	lighting_object_emitSignal(GetObj(), E_LIGHTING_OBJECT_SIGNAL_CHANGED_STATUS, "DeviceName=lighting1&Command=get_status");

	//DEV_AGENT->EmitSignalDeviceUpdate("DeviceName=lighting1&Command=get_status");
}

void ledLightingDevice::OnError()
{
	return;
}

void ledLightingDevice::OnSendDone()
{
	return;
}

int ledLightingDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int ledLightingDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
