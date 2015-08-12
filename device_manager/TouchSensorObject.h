#ifndef __TOUCHSENSOR_OBJECT_H__
#define __TOUCHSENSOR_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_TOUCHSENSOR_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_TOUCHSENSOR_OBJECT_SIGNAL_ERROR,
	E_TOUCHSENSOR_OBJECT_SIGNAL_COUNT
} TouchSensorObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	const gchar * m_cur_status;

	void *m_dev_ptr;
} TouchSensorObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_TOUCHSENSOR_OBJECT_SIGNAL_COUNT];
} TouchSensorObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType touchsensor_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define TOUCHSENSOR_TYPE_OBJECT              (touchsensor_object_get_type())

#define TOUCHSENSOR_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 TOUCHSENSOR_TYPE_OBJECT, TouchSensorObject))
#define TOUCHSENSOR_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  TOUCHSENSOR_TYPE_OBJECT, TouchSensorObjectClass))
#define TOUCHSENSOR_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 TOUCHSENSOR_TYPE_OBJECT))
#define TOUCHSENSOR_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  TOUCHSENSOR_TYPE_OBJECT))
#define TOUCHSENSOR_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								TOUCHSENSOR_TYPE_OBJECT, TouchSensorObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean touchsensor_object_get_status(TouchSensorObject* obj, gchar * *cur_status, GError** error); 



void touchsensor_object_emitSignal(TouchSensorObject* obj, 
                                TouchSensorObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
