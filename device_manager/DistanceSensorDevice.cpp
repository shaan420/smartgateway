#include "DistanceSensorDevice.hpp"
#include <boost/lexical_cast.hpp>
#include <stdlib.h>

#define OUTPUT "0"
#define INPUT "1"
#define TRIGGER_PIN "/sys/class/gpio/gpio14/value"
#define ECHO_PIN "/sys/class/gpio/gpio15/value"
#define ECHO_EDGE "/sys/class/gpio/gpio15/edge"
#define RISING_EDGE "rising"
#define FALLING_EDGE "falling"

// Inline function to read the RDTSC value - start
// source: http://kernel.ubuntu.com/git?p=cking/debug-code/.git;a=commit;h=c68c75b468f23ab4b9aa7f33e0a3d2ebf2b0252f
typedef unsigned long long t64;
typedef unsigned long      t32;

static inline unsigned long long rdtsc(void)
{
	if (sizeof(long) == sizeof(t64)) {
		t32 lo, hi;
		asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
		return ((t64)(hi) << 32) | lo;
	}
	else {
		t64 tsc;
		asm volatile("rdtsc" : "=A" (tsc));
		return tsc;
	}
}
// Inline function to read the RDTSC value - end

//Configure the output pins for multiplexer
static void gpio_init_output()
{
	const char *gpio_pins_out[]	= { "14","31","30" };
	int exp,direction,value,i;
	char path[256];

	exp = open("/sys/class/gpio/export",O_WRONLY);

	for(i=0;i<3;i++) 
	{
		pwrite(exp,gpio_pins_out[i],2,0);
		sprintf(path, "/sys/class/gpio/gpio%s/direction", gpio_pins_out[i]);
		direction = open(path,O_WRONLY);
		pwrite(direction,"out",sizeof("out"),0);
		sprintf(path, "/sys/class/gpio/gpio%s/value", gpio_pins_out[i]);
		value = open(path,O_WRONLY);
		pwrite(value,OUTPUT,2,0);
		close(direction);
		close(value);
	}

	close(exp);
}

//COnfigure the input pin (echo) of the sensor
static void gpio_init_input(void)
{
	int exp,direction;
	exp = open("/sys/class/gpio/export",O_WRONLY);
	pwrite(exp,"15",2,0);
	direction = open("/sys/class/gpio/gpio15/direction",O_WRONLY);
	pwrite(direction,"in",sizeof("in"),0);
	close(direction);
	close(exp);
}

int DistanceSensorDevice::RecvFromDevice()
{
	unsigned char tx_buf;
	unsigned long long tsc_value1, tsc_value2;

	m_prev_value = m_cur_value;

	cout << "DistanceSensor RecvFromDevice called." << endl;

	if((-1 == m_echo_fd) || (-1 == m_trigger_fd))
	{
		printf("Unable to open file descriptors:");
		return -1;
	}

	pwrite(m_echo_fd, RISING_EDGE, 6, 0);
	pwrite(m_trigger_fd, "1", 2, 0); //send 10 us trigger

	usleep(10);

	pwrite(m_trigger_fd,"0",2,0);

	if (poll(&m_poll_sensor, 1, 50) != 0) //Wait for rising edge
	{
		tsc_value1 = rdtsc(); //get tsc value
		pread(m_poll_sensor.fd,&tx_buf,sizeof(tx_buf),0);
		if (m_poll_sensor.revents & POLLPRI)
		{
			pwrite(m_echo_fd,FALLING_EDGE,7,0);
			if ( poll(&m_poll_sensor,1,2000) != 0) //wait for falling edge
			{
				tsc_value2 = rdtsc(); //get tsc value
				pread(m_poll_sensor.fd,&tx_buf,sizeof(tx_buf),0);
				if (m_poll_sensor.revents & POLLPRI)
				{
					//m_cur_value.assign(itoa(((((tsc_value2-tsc_value1)/400)*34000)/2000000), buf, 10));
					m_cur_value = boost::lexical_cast<std::string>(((((tsc_value2-tsc_value1)/400)*34000)/2000000));
					cout << "Distance: " << m_cur_value << "cm\n" << endl;
				}
			}
		}
	}

	return 0;
}

int DistanceSensorDevice::SendToDevice()
{
	return 0;
}

int DistanceSensorDevice::Read(void *mem, int len)
{
	return 0;
}

int DistanceSensorDevice::Write(void *mem, int len)
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

int DistanceSensorDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void DistanceSensorDevice::SetDeviceParams()
{
	return;
}

void DistanceSensorDevice::GetDeviceParams()
{
	return;
}

int DistanceSensorDevice::OnInit()
{
	gpio_init_input();
	gpio_init_output();

	m_echo_fd = open (ECHO_EDGE,O_RDWR);
	m_trigger_fd = open (TRIGGER_PIN,O_RDWR);
	
	if((-1 == m_echo_fd) || (-1 == m_trigger_fd))
	{
		printf("Unable to open file descriptors:");
		return -1;
	}

	/* Initialize the poll struct */
	m_poll_sensor.fd = open(ECHO_PIN, O_RDWR);
	m_poll_sensor.events = POLLPRI|POLLERR;

	/* perform dummy read to clear stale interrupts */
	char buf;
	pread(m_echo_fd, &buf, sizeof(buf), 0);

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	return 0;
}

bool DistanceSensorDevice::HasChanged()
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

void DistanceSensorDevice::OnChange()
{
	cout << "New Value: " << m_cur_value << endl;

	/* Update the D-Bus DistanceSensorObject */
	GetObj()->m_cur_status = m_cur_value.c_str();

	/* Notify status change */
	distancesensor_object_emitSignal(GetObj(), E_DISTANCESENSOR_OBJECT_SIGNAL_CHANGED_STATUS, "status");
}

void DistanceSensorDevice::OnError()
{
	return;
}

void DistanceSensorDevice::OnSendDone()
{
	return;
}

int DistanceSensorDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int DistanceSensorDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
