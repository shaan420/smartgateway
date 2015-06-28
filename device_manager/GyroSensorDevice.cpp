#include "GyroSensorDevice.hpp"
#include <sys/ioctl.h>
#include <sstream>

#define OUTPUT "0"
#define INPUT "1"

// Inline function to read the RDTSC value - start
// source: http://kernel.ubuntu.com/git?p=cking/debug-code/.git;a=commit;h=c68c75b468f23ab4b9aa7f33e0a3d2ebf2b0252f
typedef unsigned long long t64;
typedef unsigned long      t32;

uint8_t devAddr = MPU6050_DEFAULT_ADDRESS;

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

int MPU6050_OffsetCal();

int8_t readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout) {
    int8_t count = 0;
    int fd = open("/dev/i2c-0", O_RDWR);

    if (fd < 0) {
        printf("Failed to open device: %s\n", strerror(errno));
        return(-1);
    }
    if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
        printf("Failed to select device: %s\n", strerror(errno));
        close(fd);
        return(-1);
    }
    if (write(fd, &regAddr, 1) != 1) {
        printf("Failed to write reg: %s\n", strerror(errno));
        close(fd);
        return(-1);
    }
    count = read(fd, data, length);
    if (count < 0) {
        printf("Failed to read device(%d): %s\n", count, strerror(errno));
        close(fd);
        return(-1);
    } else if (count != length) {
        printf("Short read  from device, expected %d, got %d\n", length, count);
        close(fd);
        return(-1);
    }
    close(fd);

    return count;
}


int8_t readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout) {
    return readBytes(devAddr, regAddr, 1, data, timeout);
}

int8_t readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout) {
    uint8_t count, b;
    if ((count = readByte(devAddr, regAddr, &b, timeout)) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        b &= mask;
        b >>= (bitStart - length + 1);
        *data = b;
    }
    return count;
}
 
bool writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data) {
    int8_t count = 0;
    uint8_t buf[128];
    int fd;

    if (length > 127) {
        printf("Byte write count (%d) > 127\n", length);
        return(FALSE);
    }

    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        printf("Failed to open device: %s\n", strerror(errno));
        return(FALSE);
    }
    if (ioctl(fd, I2C_SLAVE, devAddr) < 0) {
        printf("Failed to select device: %s\n", strerror(errno));
        close(fd);
        return(FALSE);
    }
    buf[0] = regAddr;
    memcpy(buf+1,data,length);
    count = write(fd, buf, length+1);
    if (count < 0) {
        printf("Failed to write device(%d): %s\n", count, strerror(errno));
        close(fd);
        return(FALSE);
    } else if (count != length+1) {
        printf("Short write to device, expected %d, got %d\n", length+1, count);
        close(fd);
        return(FALSE);
    }
    close(fd);

    return TRUE;
}

bool writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    return writeBytes(devAddr, regAddr, 1, &data);
}

bool writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data) {
    uint8_t b;
    readByte(devAddr, regAddr, &b,0);
    b = (data != 0) ? (b | (1 << bitNum)) : (b & ~(1 << bitNum));
    return writeByte(devAddr, regAddr, b);
}


bool writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data) {
    uint8_t b;
    if (readByte(devAddr, regAddr, &b, 0) != 0) {
        uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
        data <<= (bitStart - length + 1); // shift data into correct position
        data &= mask; // zero all non-important bits in data
        b &= ~(mask); // zero all important bits in existing byte
        b |= data; // combine data with existing byte
        return writeByte(devAddr, regAddr, b);
    } else {
        return false;
    }
}

void setClockSource(uint8_t source) {
    writeBits(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source);
}

void setFullScaleGyroRange(uint8_t range) {
    writeBits(devAddr, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range);
}

void setFullScaleAccelRange(uint8_t range) {
    writeBits(devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range);
}

void setSleepEnabled(bool enabled) {
    writeBit(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled);
}

int getMotion6(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) 
{
	int ret;
	uint8_t buffer[14];
    ret = readBytes(devAddr, MPU6050_RA_ACCEL_XOUT_H, 14, buffer, 0);

	if (ret == -1)
	{
		return -1;
	}

    *ax = (((int16_t)buffer[0]) << 8) | buffer[1];
    *ay = (((int16_t)buffer[2]) << 8) | buffer[3];
    *az = (((int16_t)buffer[4]) << 8) | buffer[5];
    *gx = (((int16_t)buffer[8]) << 8) | buffer[9];
    *gy = (((int16_t)buffer[10]) << 8) | buffer[11];
    *gz = (((int16_t)buffer[12]) << 8) | buffer[13];

	return 0;
}

int GyroSensorDevice::MPU6050_OffsetCal()
{
	int i;
	long int x_accel_acc=0,y_accel_acc=0,z_accel_acc=0;
	long int x_gyro_acc=0,y_gyro_acc=0,z_gyro_acc=0;
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	int ret;

	for (i=1;i<=100;i++)
	{
		ret = getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
		if (-1 == ret)
		{
			return -1;
		}

		x_gyro_acc=(x_gyro_acc+gx);
		y_gyro_acc=(y_gyro_acc+gy);
		z_gyro_acc=(z_gyro_acc+gz);
		x_accel_acc=(x_accel_acc+ax);
		y_accel_acc=(y_accel_acc+ay);
		z_accel_acc=(z_accel_acc+az);
	}

	m_gx_offset=(int16_t)(x_gyro_acc/100);
	m_gy_offset=(int16_t)(y_gyro_acc/100);
	m_gz_offset=(int16_t)(z_gyro_acc/100);
	m_ax_offset=(int16_t)(x_accel_acc/100);
	m_ay_offset=(int16_t)(y_accel_acc/100);
	m_az_offset=(int16_t)(z_accel_acc/100);

	return 0;
}


//Configure the output pins for multiplexer
static void gpio_init_output()
{
	const char *gpio_pins_out[]	= { "29" };
	int exp,direction,value,i;
	char path[256];

	exp = open("/sys/class/gpio/export",O_WRONLY);

	for(i=0;i<1;i++) 
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

int GyroSensorDevice::RecvFromDevice()
{
	int ret;
	char *temp = m_prev_value;
	double unity_anglex,unity_angley,unity_anglez;
	double gx_scaled = 0, gy_scaled = 0, gz_scaled = 0;
	double gx_scaled_prev, gy_scaled_prev, gz_scaled_prev;
	unsigned long tsc_value;
	int16_t ax, ay, az, gx, gy, gz;
	double time_diff;

	m_prev_value = m_cur_value;
	m_cur_value = temp;

	tsc_value = rdtsc();

	cout << "GyroSensor RecvFromDevice called." << endl;
	for(int i=0;i<SAMPLE_COUNT;i++)
	{
		usleep(100);
		ret = getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

		if (ret == -1)
		{
			return -1;
		}

		gx_scaled = gx_scaled + (double)((double)(gx))/131.0f;
		gy_scaled = gy_scaled + (double)((double)(gy))/131.0f;
		gz_scaled = gz_scaled + (double)((double)(gz))/131.0f;
	}

	time_diff = (double)((rdtsc()-tsc_value)/CPU_CLOCK);
	time_diff = (double) ((double)time_diff/(double)SAMPLE_COUNT);
	gx_scaled = gx_scaled/SAMPLE_COUNT;
	gy_scaled = gy_scaled/SAMPLE_COUNT;
	gz_scaled = gz_scaled/SAMPLE_COUNT;

	//filtering noise from scaled values of accelerometer and gyro -- based on trial and error.
	if( (gx_scaled <= -2.0) && (gx_scaled >= -2.5))
		gx_scaled = 0;

	if( (gy_scaled <= 0.2) && (gy_scaled >= 0))
		gy_scaled = 0;	

	if( (gz_scaled <= -0.4) && (gz_scaled >= -0.5))
		gz_scaled = 0;

	stringstream prev(m_prev_value);

	// skip the initial 3 zero values
	int i = 0;
	while (i++ < 3)
	{
		double n;
		prev >> n;
	}

	prev >> gx_scaled_prev;
	prev >> gy_scaled_prev;
	prev >> gz_scaled_prev;
	
	//single integration to find angles from gyro values. Idea from Freescale AN3397
	unity_anglex = ((gx_scaled_prev + ((double)(gx_scaled-gx_scaled_prev)/2.0f))*(time_diff/1000.0f));
	unity_angley = ((gy_scaled_prev + ((double)(gy_scaled-gy_scaled_prev)/2.0f))*(time_diff/1000.0f));
	unity_anglez = ((gz_scaled_prev + ((double)(gz_scaled-gz_scaled_prev)/2.0f))*(time_diff/1000.0f));
	//Filter unwanted noise and make tilts smoother -- based on trial and error
	if((unity_anglex <= 0.1f) && (unity_anglex >= -0.1f))
	{
		unity_anglex = 0.0f;
	}
	if((unity_angley <= 0.6f) && (unity_angley >= -0.6f))
	{
		unity_angley = 0.0f;
	}	
	if((unity_anglez <= 3.5f) && (unity_anglez >= -3.5f))
	{
		unity_anglez = 0.0f;
	}

	//differentiating to find the angles
	unity_anglex = (unity_anglex * 0.017453)/(time_diff/1000.0f);
	unity_angley = (unity_angley * 0.017453)/(time_diff/1000.0f);
	unity_anglez = (unity_anglez * 0.017453)/(time_diff/1000.0f);

	//wait till some initial set of angles are calculated for stability
	if(m_initial_skip_count < INITIAL_SKIP_COUNT)
	{
		snprintf(m_cur_value,
				 MAX_STR_LEN,
				 "0.0 0.0 0.0 0.0 0.0 0.0");

		m_initial_skip_count++;
	}
	else
	{
		snprintf(m_cur_value,
				 MAX_STR_LEN,
				 "0.0 0.0 0.0 %f %f %f", 
				 (float)unity_anglex, (float)unity_anglez, (float)unity_angley);
	}

	return 0;
}

int GyroSensorDevice::SendToDevice()
{
	return 0;
}

int GyroSensorDevice::Read(void *mem, int len)
{
	return 0;
}

int GyroSensorDevice::Write(void *mem, int len)
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

int GyroSensorDevice::ReadShared(void **data)
{
	*data = m_readBuf;
	return 0;
}

void GyroSensorDevice::SetDeviceParams()
{
	return;
}

void GyroSensorDevice::GetDeviceParams()
{
	return;
}

int GyroSensorDevice::OnInit()
{
	//gpio_init_input();
	gpio_init_output();
	
    setClockSource(MPU6050_CLOCK_PLL_XGYRO);
    setFullScaleGyroRange(MPU6050_GYRO_FS_250);
    setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
    setSleepEnabled(false);
	MPU6050_OffsetCal();

	/* Clear the data holders */
	m_readBuf = NULL;
	m_writeBuf = NULL;

	/* Init prev and cur value to zeros */
	snprintf(m_cur_value, MAX_STR_LEN, "0.0 0.0 0.0 0.0 0.0 0.0");
	snprintf(m_prev_value, MAX_STR_LEN, "0.0 0.0 0.0 0.0 0.0 0.0");

	return 0;
}

bool GyroSensorDevice::HasChanged()
{
	if (g_strcmp0(m_prev_value, m_cur_value))
	{
		return true;
	}
	else
	{
		return false;
	}

	return true;
}

void GyroSensorDevice::OnChange()
{
	cout << "New Value: " << m_cur_value << endl;

	/* Update the D-Bus GyroSensorObject */
	GetObj()->m_cur_status = m_cur_value;
	//GetObj()->m_cur_value = g_strndup(m_cur_value, MAX_STR_LEN);

	/* Notify status change */
	gyrosensor_object_emitSignal(GetObj(), E_GYROSENSOR_OBJECT_SIGNAL_CHANGED_STATUS, "status");
}

void GyroSensorDevice::OnError()
{
	return;
}

void GyroSensorDevice::OnSendDone()
{
	return;
}

int GyroSensorDevice::PrepareOutputData(string& output)
{
	output.assign(m_cur_value);
	return 0;
}

int GyroSensorDevice::UpdateDataManager()
{
	DeviceBase::UpdateDataManager("get_status");
	return 0;
}
