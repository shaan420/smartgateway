#ifndef __DEVICE_BASE_HPP__
#define __DEVICE_BASE_HPP__
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <glib.h>

using namespace std;

typedef enum 
{
	DEVICE_DATA_RETRIEVAL_METHOD_PULL = 0,
	DEVICE_DATA_RETRIEVAL_METHOD_PUSH,
	DEVICE_DATA_RETRIEVAL_METHOD_UNSET
} DeviceDataRetrievalMethod_t;

typedef enum
{
	DEVICE_COMM_METHOD_BLUETOOTH = 0,
	DEVICE_COMM_METHOD_WIFI,
	DEVICE_COMM_METHOD_I2C,
	DEVICE_COMM_METHOD_USB,
	DEVICE_COMM_METHOD_SPI,
	DEVICE_COMM_METHOD_GPIO,
	DEVICE_COMM_METHOD_TCP,
	DEVICE_COMM_METHOD_HTTP,
	DEVICE_COMM_METHOD_CUSTOM,
	DEVICE_COMM_METHOD_UNSET
} DeviceCommMethod_t;

typedef enum
{
	DEVICE_DATA_RETRIEVAL_FREQ_PERIODIC = 0,
	DEVICE_DATA_RETRIEVAL_FREQ_ONDEMAND,
	DEVICE_DATA_RETRIEVAL_FREQ_UNSET
} DeviceDataRetrievalFreq_t;

typedef struct
{
	int m_devId;
	double m_inputSamplingRateMsec;
	double m_outputGenRateMsec;
	DeviceDataRetrievalFreq_t m_retrievalFreq;
	DeviceDataRetrievalMethod_t m_retrievalMethod;
	DeviceCommMethod_t m_commMethod;
	/*
	 * commParams denote the parameters that the device uses
	 * to communicate with the actual device instance.
	 * For example, lighting1 is a GPIO LED that uses the 
	 * ledLightingDevice Driver. commParams would be the 
	 * GPIO PIN number. Similarly, for TCP it would be
	 * ip:port or for Bluetooth it would be its address.
	 */
	string m_commParams;

	/*
	 * The driverName must match the registered device class name
	 * that implements all its functions.
	 */
	string m_driverName;

	/*
	 * TODO: The driver for the device could be dynamically selected based on
	 * commMethod and commParams. For example, for commMethod=gpio, we could have 
	 * a dedicated gpio driver (just like ledLightingDevice).
	 */
} DeviceConf_t;

class DeviceBase
{
	protected:
		string m_name;
		string m_desc;
		DeviceConf_t m_deviceConf;
		void *m_readBuf;
		void *m_writeBuf;
		void *m_handler;

	private:
		GThread *m_sendThread;
		GThread *m_recvThread;
		//pthread_t m_sendThread;
		//pthread_t m_recvThread;
		sem_t m_recvSem;
		sem_t m_sendSem;

	public:
		DeviceBase()
		{
			m_name.assign("NA");
			m_desc.assign("NA");
			m_deviceConf.m_devId = 0;
			m_deviceConf.m_inputSamplingRateMsec = 0;
			m_deviceConf.m_outputGenRateMsec = 0;
			m_deviceConf.m_retrievalFreq = DEVICE_DATA_RETRIEVAL_FREQ_UNSET;
			m_deviceConf.m_retrievalMethod = DEVICE_DATA_RETRIEVAL_METHOD_UNSET;
			m_deviceConf.m_commMethod = DEVICE_COMM_METHOD_UNSET;
			m_readBuf = NULL;
			m_writeBuf = NULL;
			m_handler = NULL;
		}

		DeviceBase(const string& name, const string& desc, DeviceConf_t &conf)
		{
			/* Be careful of such assignments */
			m_name = name;
			m_desc = desc;
			m_deviceConf = conf;
			m_readBuf = NULL;
			m_writeBuf = NULL;
		}

		string& GetDeviceName()
		{
			return m_name;
		}

		string& GetDeviceDesc()
		{
			return m_desc;
		}

		DeviceConf_t& GetDeviceConf()
		{
			return m_deviceConf;
		}

		sem_t& GetRecvSem()
		{
			return m_recvSem;
		}
		
		sem_t& GetSendSem()
		{
			return m_sendSem;
		}

		/*******************************************************************/
		/* These functions are called from the user-app */
		/* 
		 * Called by user-app.
		 * This function will spawn a new thread for this particular device
		 * and interact with the device as specified in the DeviceConf.
		 */
		virtual int DeployDevice();

		/* 
		 * Override this to read data from the m_rbuf 
		 */
		virtual int Read(void *mem, int len) = 0;

		/* 
		 * Optionally override this function to prepare for write in the userapp context.
		 * This function *[WILL] / [SHOULD(if overridden)]* notity the sender thread 
		 * that there is something to be written to the device and therefore 
		 * the SendToDevice would be invoked.
		 */
		virtual int Write(void *mem, int len);

		/* 
		 * Override this to return the pointer to rbuf 
		 */
		virtual int ReadShared(void **mem) = 0;

		/******************************************************************/	
		/******************************************************************/	
		/* These functions are called from this library to notify the user-app
		 * of events. These must be overridden/implemented by the userapp */
		/* 
		 * Override this to perform the actual read from the device.
		 * It should typically copy/append the read data into m_rbuf.
		 */
		virtual int RecvFromDevice() = 0;

		/* 
		 * This should typically write the contents of the m_wbuf to the 
		 * device.
		 */
		virtual int SendToDevice() = 0;

		/* Set device params */
		virtual void SetDeviceParams() = 0;
	
		/* Get device params */
		virtual void GetDeviceParams() = 0;

		/* Notify the user-app of device init */
		virtual int OnInit() = 0;

		/* Decide whether a change has occured. */
		virtual bool HasChanged() = 0;

		/* Notify the user-app of a change in the device data */
		virtual void OnChange() = 0;

		/* Notify the app of send done */
		virtual void OnSendDone() = 0;

		/* Notify the user-app when an internal device error/disconnect occurs */
		virtual void OnError() = 0;

		/* Updating Data Manager with streaming data from devices */
		virtual bool ShouldUpdateDataManager();
		virtual int UpdateDataManager() = 0;

		/* 
		 * This should be called from inside the overridden version of UpdateDataManager() above
		 * with the appropriate command name.
		 */
		virtual int UpdateDataManager(const char *command);

		/* Prepare the output for Data Manager */
		virtual int PrepareOutputData(string& value) = 0;
};
#endif
