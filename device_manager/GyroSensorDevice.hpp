#ifndef __GYRO_SENSOR_DEVICE_HPP__
#define __GYRO_SENSOR_DEVICE_HPP__
#include "DeviceDbus.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/i2c-dev.h>

#include "MPU6050.h"
#include "GyroSensorObject.h"

#define SAMPLE_COUNT 10
#define CPU_CLOCK 400000000.0f
#define INITIAL_SKIP_COUNT 5

class GyroSensorDevice : public DeviceDbus<GyroSensorObject, int>
{
	protected:
		int RecvFromDevice();
		int SendToDevice();

	private:
		char temp_str1[MAX_STR_LEN];
		char temp_str2[MAX_STR_LEN];
		char *m_cur_value;
		char *m_prev_value;
		int16_t m_ax_offset, m_ay_offset, m_az_offset;
		int16_t m_gx_offset, m_gy_offset, m_gz_offset;
		uint8_t m_dev_addr;
		int m_initial_skip_count;

		GyroSensorDevice() {}

		int MPU6050_OffsetCal();
	public:
		GyroSensorDevice(const string name, const string desc, DeviceConf_t conf) 
				: DeviceDbus(GYROSENSOR_TYPE_OBJECT, name, desc, conf)
		{ 
			m_cur_value = temp_str1;
			m_prev_value = temp_str2;
			m_initial_skip_count = 0;
			m_ax_offset = m_ay_offset = m_az_offset = m_gx_offset = m_gy_offset = m_gz_offset = 0;
		}

		int OnInit();
		void OnError();
		void OnChange();
		void OnSendDone();
		bool HasChanged();
		void SetDeviceParams();
		void GetDeviceParams();
		int ReadShared(void **mem);
		int Write(void *mem, int len);
		int Read(void *mem, int len);
		int PrepareOutputData(string& output);
		int UpdateDataManager();
};
#endif
