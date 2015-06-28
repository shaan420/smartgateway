#ifndef __DEVICE_FACTORY_HPP__
#define __DEVICE_FACTORY_HPP__
#include <map>
#include <stdio.h>
#include <string>
#include "Device.hpp"

using namespace std;

template <class T> 
void *constructor(const string& name, const string& desc, DeviceConf_t& conf) 
{ 
	return ((void *) new T(name, desc, conf)); 
}

struct DeviceFactory
{
	typedef void*(*constructor_t)(const string&, const string&, DeviceConf_t&);
	typedef std::map<std::string, constructor_t> map_type;
	map_type m_classes;

	template <class T>
	void register_class(std::string const& n)
	{ 
		m_classes.insert(std::make_pair(n, &constructor<T>)); 
	}

	void *construct(string const& n, 
					string const& devname, 
					string const& desc, 
					DeviceConf_t conf)
	{
		map_type::iterator i = m_classes.find(n);
		if (i == m_classes.end()) return NULL;
		return i->second(devname, desc, conf);
	}
};

#define DEVICE_FACTORY_REGISTER_DEVICE(n) g_factory.register_class<n>(#n)
#endif
