#ifndef __DISTANCESENSOR_OBJECT_H__
#define __DISTANCESENSOR_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_DISTANCESENSOR_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_DISTANCESENSOR_OBJECT_SIGNAL_ERROR,
	E_DISTANCESENSOR_OBJECT_SIGNAL_COUNT
} DistanceSensorObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	gchar * m_cur_status;

	void *m_dev_ptr;
} DistanceSensorObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_DISTANCESENSOR_OBJECT_SIGNAL_COUNT];
} DistanceSensorObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType distancesensor_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define DISTANCESENSOR_TYPE_OBJECT              (distancesensor_object_get_type())

#define DISTANCESENSOR_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 DISTANCESENSOR_TYPE_OBJECT, DistanceSensorObject))
#define DISTANCESENSOR_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  DISTANCESENSOR_TYPE_OBJECT, DistanceSensorObjectClass))
#define DISTANCESENSOR_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 DISTANCESENSOR_TYPE_OBJECT))
#define DISTANCESENSOR_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  DISTANCESENSOR_TYPE_OBJECT))
#define DISTANCESENSOR_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								DISTANCESENSOR_TYPE_OBJECT, DistanceSensorObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean distancesensor_object_get_status(DistanceSensorObject* obj, gchar * *cur_status, GError** error); 



void distancesensor_object_emitSignal(DistanceSensorObject* obj, 
                                DistanceSensorObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
