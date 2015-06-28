#include "DeviceBase.hpp"
#include <dbus/dbus-glib.h>

int g_end = 0;

/* Proxy DBUS object to communicate with DataManager */
DBusGProxy *g_dataManagerObj;

static gpointer PeriodicRetrievalFn(gpointer data)
{
	DeviceBase *dev = (DeviceBase *)data;
	int err = 0;

	while(!g_end)
	{
		/* wait for new period */
		usleep(dev->GetDeviceConf().m_inputSamplingRateMsec*1000);

		/* query device */
		err = dev->RecvFromDevice();

		if (err)
		{
			dev->OnError();
			cout << "Error reading from device" << endl;
			//return NULL;
		}

		/* detect if change */
		if (dev->HasChanged())
		{
			/* notify the app so that it can Read() */
			dev->OnChange();

			if (dev->ShouldUpdateDataManager())
			{
				dev->UpdateDataManager();
			}
		}
	}

	return NULL;
}

static gpointer OnDemandRetrievalFn(gpointer data)
{
	int err = 0;
	DeviceBase *dev = (DeviceBase *)data;

	while(!g_end)
	{
		/* wait for new trigger */
		sem_wait(&dev->GetRecvSem());

		/* query device */
		err = dev->RecvFromDevice();

		if (err)	
		{
			dev->OnError();
			cout << "Error reading from device" << endl;
			return NULL;
		}

		/* detect if change */
		if (dev->HasChanged())
		{
			/* notify the app */
			dev->OnChange();
		}
	}

	return NULL;
}

static gpointer SendFn(gpointer data)
{
	int err = 0;
	DeviceBase *dev = (DeviceBase *)data;

	while (!g_end)
	{
		/* wait for new trigger */
		sem_wait(&dev->GetSendSem());

		err = dev->SendToDevice();

		if (err)	
		{
			dev->OnError();
			cout << "Error sending to device" << endl;
			return NULL;
		}

		/* Notify app that send has successfully been completed. */
		dev->OnSendDone();
	}

	return NULL;
}

int DeviceBase::DeployDevice()
{
	int ret;

	ret = OnInit();
	if (0 != ret)
	{
		printf("OnInit failed.\n");
		return -1;
	}

	/* initialize the recv thread */
	switch (m_deviceConf.m_retrievalFreq)
	{
		case DEVICE_DATA_RETRIEVAL_FREQ_PERIODIC:
			m_recvThread = g_thread_new("PeriodicRetrievalFn", &PeriodicRetrievalFn, (gpointer)this);
			break;

		case DEVICE_DATA_RETRIEVAL_FREQ_ONDEMAND:
			sem_init(&GetRecvSem(), 0, 0);
			m_recvThread = g_thread_new("OnDemandRetrievalFn", &OnDemandRetrievalFn, (gpointer)this);
			break;

		default:
			cout << "Invalid retrieval frequency" << endl;
	}

	/* initialize the send thread */
	sem_init(&GetSendSem(), 0, 0);
	m_sendThread = g_thread_new("SendFn", &SendFn, (gpointer)this);
	printf("%s deployed\n", GetDeviceName().c_str());
	return 0;
}

int DeviceBase::Write(void *mem, int len)
{
	/* 
	 * TODO: copy mem to internal m_wbuf so that 
	 * the sender thread can access it when it performs the
	 * SendToDevice().
	 */
	sem_post(&GetSendSem());
	return 0;
}

int DeviceBase::UpdateDataManager(const char *command)
{
	GError **error;
	string value;
	PrepareOutputData(value);

	dbus_g_proxy_call (g_dataManagerObj,
			"insert",
			error,
			G_TYPE_STRING,
			m_name.c_str(),
			G_TYPE_STRING,
			command,
			G_TYPE_STRING,
			value.c_str(),
			G_TYPE_INVALID,
			G_TYPE_INVALID);

	return 0;
}

bool DeviceBase::ShouldUpdateDataManager()
{
	return true;
}
