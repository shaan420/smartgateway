#ifndef __DEVICE_HPP__
#define __DEVICE_HPP__
#include <iostream>
#include <stdio.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>

#include "DeviceBase.hpp"

using namespace std;

template <class DevHandlerType>
class Device : public DeviceBase
{
	protected:
		DevHandlerType *m_handler;

		void CreateDevHandler(void *params);
		void DestroyDevHandler();
	
	public:
		Device()
		{
		}

		Device(const string& name, const string& desc, DeviceConf_t &conf)
			: DeviceBase(name, desc, conf)
		{
		}

		/* Get device handler */
		virtual DevHandlerType *GetDeviceHandler() const 
		{
			return m_handler;
		}
};
#endif
