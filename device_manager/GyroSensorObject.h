#ifndef __GYROSENSOR_OBJECT_H__
#define __GYROSENSOR_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_GYROSENSOR_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_GYROSENSOR_OBJECT_SIGNAL_ERROR,
	E_GYROSENSOR_OBJECT_SIGNAL_COUNT
} GyroSensorObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	const gchar * m_cur_status;

	void *m_dev_ptr;
} GyroSensorObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_GYROSENSOR_OBJECT_SIGNAL_COUNT];
} GyroSensorObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType gyrosensor_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define GYROSENSOR_TYPE_OBJECT              (gyrosensor_object_get_type())

#define GYROSENSOR_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 GYROSENSOR_TYPE_OBJECT, GyroSensorObject))
#define GYROSENSOR_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  GYROSENSOR_TYPE_OBJECT, GyroSensorObjectClass))
#define GYROSENSOR_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 GYROSENSOR_TYPE_OBJECT))
#define GYROSENSOR_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  GYROSENSOR_TYPE_OBJECT))
#define GYROSENSOR_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								GYROSENSOR_TYPE_OBJECT, GyroSensorObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean gyrosensor_object_get_status(GyroSensorObject* obj, gchar * *cur_status, GError** error); 



void gyrosensor_object_emitSignal(GyroSensorObject* obj, 
                                GyroSensorObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
