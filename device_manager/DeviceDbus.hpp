#ifndef __DEVICE_DBUS_HPP__
#define __DEVICE_DBUS_HPP__

#include "Device.hpp"
#include "DeviceAgent.hpp"
#include <glib-object.h>
#include "../common/include/common-defs.h"

template <typename DbusObjectType, typename DeviceHandlerType>
class DeviceDbus : public Device<DeviceHandlerType>
{
	private:
		DbusObjectType *m_obj;

	public: 
		DeviceDbus() {}
		DeviceDbus(GType type, const string& name, const string& desc, DeviceConf_t& conf)
			: Device<DeviceHandlerType>(name, desc, conf)
		{
			guint result;
			DBusGProxy *busProxy;
			GError* error = NULL;
			string service_path(SMARTGATEWAY_SERVICE_PATH_PREFIX);
			service_path += name;

			busProxy = dbus_g_proxy_new_for_name(DEV_AGENT->GetDbusSession(),
					DBUS_SERVICE_DBUS,
					DBUS_PATH_DBUS,
					DBUS_INTERFACE_DBUS);

			if (busProxy == NULL)
			{
				printf("Error: could not create dbus proxy for name registration\n");
				return;
			}

			if (!dbus_g_proxy_call(busProxy,
						/* Method name to call. */
						"RequestName",
						/* Where to store the GError. */
						&error,
						/* Parameter type of argument 0. Note that
						   since we're dealing with GLib/D-Bus
						   wrappers, you will need to find a suitable
						   GType instead of using the "native" D-Bus
						   type codes. */
						G_TYPE_STRING,
						/* Data of argument 0. In our case, this is
						   the well-known name for our server
						   example ("org.asu.smartgateway"). */
						service_path.c_str(),
						/* Parameter type of argument 1. */
						G_TYPE_UINT,
						/* Data of argument 0. This is the "flags"
						   argument of the "RequestName" method which
						   can be use to specify what the bus service
						   should do when the name already exists on
						   the bus. We'll go with defaults. */
						0,
						/* Input arguments are terminated with a
						   special GType marker. */
						G_TYPE_INVALID,
						/* Parameter type of return lighting 0.
						   For "RequestName" it is UINT32 so we pick
						   the GType that maps into UINT32 in the
						   wrappers. */
						G_TYPE_UINT,
						/* Data of return lighting 0. These will always
						   be pointers to the locations where the
						   proxy can copy the results. */
						&result,
						/* Terminate list of return lightings. */
						G_TYPE_INVALID)) {
							printf("D-Bus.RequestName RPC failed: %s\n", error->message);
							/* Note that the whole call failed, not "just" the name
							   registration (we deal with that below). This means that
							   something bad probably has happened and there's not much we can
							   do (hence program termination). */
							return;
						}

			/* Check the result code of the registration RPC. */
			printf("RequestName returned %d for %s.\n", result, service_path.c_str());

			m_obj = (DbusObjectType *)g_object_new(type, NULL);
			if (m_obj == NULL) 
			{
				g_print("Failed to create one gobj instance.\n");
				return;
			}

			/* 
			 * Store the back pointer here so that when the D-Bus "set_status"
			 * is invoked, we can use the LightingDevice obj from the LightingObject obj.
			 */
			m_obj->m_dev_ptr = static_cast<void *>(this);
			
			g_print("Registering DbusObject on the D-Bus.\n");

			/* 
			 * The function does not return any status, so can't check for
			 * errors here. 
			 */
			dbus_g_connection_register_g_object(DEV_AGENT->GetDbusSession(),
					(SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX+name).c_str(),
					G_OBJECT(m_obj));

			g_print("%s ready to serve requests\n", name.c_str());
		}

		~DeviceDbus() {}

		DbusObjectType *GetObj() const
		{
			return m_obj;
		}
};

#endif
