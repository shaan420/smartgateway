#ifndef __DEVICEAGENT_OBJECT_H__
#define __DEVICEAGENT_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_DEVICEAGENT_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_DEVICEAGENT_OBJECT_SIGNAL_ERROR,
	E_DEVICEAGENT_OBJECT_SIGNAL_COUNT
} DeviceAgentObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
} DeviceAgentObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_DEVICEAGENT_OBJECT_SIGNAL_COUNT];
} DeviceAgentObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType deviceAgent_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define DEVICEAGENT_TYPE_OBJECT              (deviceAgent_object_get_type())

#define DEVICEAGENT_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 DEVICEAGENT_TYPE_OBJECT, DeviceAgentObject))
#define DEVICEAGENT_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  DEVICEAGENT_TYPE_OBJECT, DeviceAgentObjectClass))
#define DEVICEAGENT_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 DEVICEAGENT_TYPE_OBJECT))
#define DEVICEAGENT_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  DEVICEAGENT_TYPE_OBJECT))
#define DEVICEAGENT_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								DEVICEAGENT_TYPE_OBJECT, DeviceAgentObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean deviceAgent_object_new_device(DeviceAgentObject* obj, gchar *iface_name, gchar *devname, GError** error);

void deviceAgent_object_emitSignal(DeviceAgentObject* obj, 
                                DeviceAgentObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
