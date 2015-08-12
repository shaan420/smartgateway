#ifndef __LIGHTSENSOR_OBJECT_H__
#define __LIGHTSENSOR_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_LIGHTSENSOR_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_LIGHTSENSOR_OBJECT_SIGNAL_ERROR,
	E_LIGHTSENSOR_OBJECT_SIGNAL_COUNT
} LightSensorObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	const gchar * m_cur_status;

	void *m_dev_ptr;
} LightSensorObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_LIGHTSENSOR_OBJECT_SIGNAL_COUNT];
} LightSensorObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType lightsensor_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define LIGHTSENSOR_TYPE_OBJECT              (lightsensor_object_get_type())

#define LIGHTSENSOR_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 LIGHTSENSOR_TYPE_OBJECT, LightSensorObject))
#define LIGHTSENSOR_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  LIGHTSENSOR_TYPE_OBJECT, LightSensorObjectClass))
#define LIGHTSENSOR_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 LIGHTSENSOR_TYPE_OBJECT))
#define LIGHTSENSOR_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  LIGHTSENSOR_TYPE_OBJECT))
#define LIGHTSENSOR_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								LIGHTSENSOR_TYPE_OBJECT, LightSensorObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean lightsensor_object_get_status(LightSensorObject* obj, gchar * *cur_status, GError** error); 



void lightsensor_object_emitSignal(LightSensorObject* obj, 
                                LightSensorObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
