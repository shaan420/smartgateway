#ifndef __CONTEXTMANAGER_OBJECT_H__
#define __CONTEXTMANAGER_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_CONTEXTMANAGER_OBJECT_SIGNAL_NEW_QUERY = 0,
	E_CONTEXTMANAGER_OBJECT_SIGNAL_ERROR,
	E_CONTEXTMANAGER_OBJECT_SIGNAL_COUNT
} ContextManagerObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
} ContextManagerObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_CONTEXTMANAGER_OBJECT_SIGNAL_COUNT];
} ContextManagerObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType contextManager_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define CONTEXTMANAGER_TYPE_OBJECT              (contextManager_object_get_type())

#define CONTEXTMANAGER_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 CONTEXTMANAGER_TYPE_OBJECT, ContextManagerObject))
#define CONTEXTMANAGER_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  CONTEXTMANAGER_TYPE_OBJECT, ContextManagerObjectClass))
#define CONTEXTMANAGER_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 CONTEXTMANAGER_TYPE_OBJECT))
#define CONTEXTMANAGER_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  CONTEXTMANAGER_TYPE_OBJECT))
#define CONTEXTMANAGER_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								CONTEXTMANAGER_TYPE_OBJECT, ContextManagerObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean contextManager_object_new_query_url(ContextManagerObject* obj, gchar *query_type, gchar *params, gchar **response, GError** error);
gboolean contextManager_object_dev_command(ContextManagerObject* obj, gchar *command, gchar **response, GError** error);
gboolean contextManager_object_ont_update(ContextManagerObject* obj, gchar *params, gchar **response, GError** error);
gboolean contextManager_object_device_info(ContextManagerObject* obj, gchar *params, gchar **response, GError** error);

void contextManager_object_emitSignal(ContextManagerObject* obj, 
                                ContextManagerObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
