#ifndef __TEMPERATURESENSOR_OBJECT_H__
#define __TEMPERATURESENSOR_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_TEMPERATURESENSOR_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_TEMPERATURESENSOR_OBJECT_SIGNAL_ERROR,
	E_TEMPERATURESENSOR_OBJECT_SIGNAL_COUNT
} TemperatureSensorObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	const gchar * m_cur_status;

	void *m_dev_ptr;
} TemperatureSensorObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_TEMPERATURESENSOR_OBJECT_SIGNAL_COUNT];
} TemperatureSensorObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType temperaturesensor_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define TEMPERATURESENSOR_TYPE_OBJECT              (temperaturesensor_object_get_type())

#define TEMPERATURESENSOR_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 TEMPERATURESENSOR_TYPE_OBJECT, TemperatureSensorObject))
#define TEMPERATURESENSOR_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  TEMPERATURESENSOR_TYPE_OBJECT, TemperatureSensorObjectClass))
#define TEMPERATURESENSOR_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 TEMPERATURESENSOR_TYPE_OBJECT))
#define TEMPERATURESENSOR_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  TEMPERATURESENSOR_TYPE_OBJECT))
#define TEMPERATURESENSOR_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								TEMPERATURESENSOR_TYPE_OBJECT, TemperatureSensorObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean temperaturesensor_object_get_status(TemperatureSensorObject* obj, gchar * *cur_status, GError** error); 



void temperaturesensor_object_emitSignal(TemperatureSensorObject* obj, 
                                TemperatureSensorObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
