#ifndef __LIGHTING_OBJECT_H__
#define __LIGHTING_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_LIGHTING_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_LIGHTING_OBJECT_SIGNAL_ERROR,
	E_LIGHTING_OBJECT_SIGNAL_COUNT
} LightingObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	gchar * m_cur_status;
gint m_new_status;

	void *m_dev_ptr;
} LightingObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_LIGHTING_OBJECT_SIGNAL_COUNT];
} LightingObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType lighting_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define LIGHTING_TYPE_OBJECT              (lighting_object_get_type())

#define LIGHTING_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 LIGHTING_TYPE_OBJECT, LightingObject))
#define LIGHTING_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  LIGHTING_TYPE_OBJECT, LightingObjectClass))
#define LIGHTING_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 LIGHTING_TYPE_OBJECT))
#define LIGHTING_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  LIGHTING_TYPE_OBJECT))
#define LIGHTING_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								LIGHTING_TYPE_OBJECT, LightingObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean lighting_object_get_status(LightingObject* obj, gchar * *cur_status, GError** error); 

gboolean lighting_object_set_status(LightingObject* obj, gint new_status, GError** error); 



void lighting_object_emitSignal(LightingObject* obj, 
                                LightingObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
