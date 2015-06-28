#ifndef __DATAMANAGER_OBJECT_H__
#define __DATAMANAGER_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_DATAMANAGER_OBJECT_SIGNAL_NEW_KEY = 0,
	E_DATAMANAGER_OBJECT_SIGNAL_ERROR,
	E_DATAMANAGER_OBJECT_SIGNAL_COUNT
} DataManagerObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
} DataManagerObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_DATAMANAGER_OBJECT_SIGNAL_COUNT];
} DataManagerObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType dataManager_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define DATAMANAGER_TYPE_OBJECT              (dataManager_object_get_type())

#define DATAMANAGER_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 DATAMANAGER_TYPE_OBJECT, DataManagerObject))
#define DATAMANAGER_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  DATAMANAGER_TYPE_OBJECT, DataManagerObjectClass))
#define DATAMANAGER_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 DATAMANAGER_TYPE_OBJECT))
#define DATAMANAGER_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  DATAMANAGER_TYPE_OBJECT))
#define DATAMANAGER_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								DATAMANAGER_TYPE_OBJECT, DataManagerObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean dataManager_object_new(DataManagerObject* obj, 
							    gchar *key, 
								GError** error);

gboolean dataManager_object_insert(DataManagerObject* obj, 
								   gchar *devname, 
								   gchar *command, 
								   gchar *value, 
								   GError** error);

gboolean dataManager_object_find(DataManagerObject* obj, 
								 gchar *devname, 
								 gchar *command, 
								 gint ts, 
								 gchar **value, 
								 gint *num_elems, 
								 GError** error);

void dataManager_object_emitSignal(DataManagerObject* obj, 
                                DataManagerObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
