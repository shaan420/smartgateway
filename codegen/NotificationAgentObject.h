#ifndef __NOTIFICATIONAGENT_OBJECT_H__
#define __NOTIFICATIONAGENT_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_NOTIFICATIONAGENT_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_NOTIFICATIONAGENT_OBJECT_SIGNAL_ERROR,
	E_NOTIFICATIONAGENT_OBJECT_SIGNAL_COUNT
} NotificationAgentObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	gchar * m_event;
gchar * m_event_params;
gchar * m_new_event;
gchar * m_new_sub;

	void *m_dev_ptr;
} NotificationAgentObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_NOTIFICATIONAGENT_OBJECT_SIGNAL_COUNT];
} NotificationAgentObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType notificationagent_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define NOTIFICATIONAGENT_TYPE_OBJECT              (notificationagent_object_get_type())

#define NOTIFICATIONAGENT_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 NOTIFICATIONAGENT_TYPE_OBJECT, NotificationAgentObject))
#define NOTIFICATIONAGENT_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  NOTIFICATIONAGENT_TYPE_OBJECT, NotificationAgentObjectClass))
#define NOTIFICATIONAGENT_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 NOTIFICATIONAGENT_TYPE_OBJECT))
#define NOTIFICATIONAGENT_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  NOTIFICATIONAGENT_TYPE_OBJECT))
#define NOTIFICATIONAGENT_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								NOTIFICATIONAGENT_TYPE_OBJECT, NotificationAgentObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean notificationagent_object_new_event(NotificationAgentObject* obj, gchar * new_event, GError** error); 

gboolean notificationagent_object_new_subscriber(NotificationAgentObject* obj, gchar * event,gchar * new_sub, GError** error); 

gboolean notificationagent_object_publish_event(NotificationAgentObject* obj, gchar * event_params, GError** error); 



void notificationagent_object_emitSignal(NotificationAgentObject* obj, 
                                NotificationAgentObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
