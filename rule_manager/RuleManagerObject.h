#ifndef __RULEMANAGER_OBJECT_H__
#define __RULEMANAGER_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_RULEMANAGER_OBJECT_SIGNAL_NEW_RULE = 0,
	E_RULEMANAGER_OBJECT_SIGNAL_ERROR,
	E_RULEMANAGER_OBJECT_SIGNAL_COUNT
} RuleManagerObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
} RuleManagerObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_RULEMANAGER_OBJECT_SIGNAL_COUNT];
} RuleManagerObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType ruleManager_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define RULEMANAGER_TYPE_OBJECT              (ruleManager_object_get_type())

#define RULEMANAGER_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 RULEMANAGER_TYPE_OBJECT, RuleManagerObject))
#define RULEMANAGER_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  RULEMANAGER_TYPE_OBJECT, RuleManagerObjectClass))
#define RULEMANAGER_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 RULEMANAGER_TYPE_OBJECT))
#define RULEMANAGER_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  RULEMANAGER_TYPE_OBJECT))
#define RULEMANAGER_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								RULEMANAGER_TYPE_OBJECT, RuleManagerObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean ruleManager_object_insert_rule(RuleManagerObject* obj, gchar *rule, gchar **response, GError** error);
gboolean ruleManager_object_delete_rule(RuleManagerObject* obj, gchar *ruleId, gchar **response, GError** error);
gboolean ruleManager_object_insert_event(RuleManagerObject* obj, gchar *event, gchar **response, GError** error);
gboolean ruleManager_object_insert_event_external(RuleManagerObject* obj, gchar *event, gchar **response, GError** error);
gboolean ruleManager_object_delete_event(RuleManagerObject* obj, gchar *eventId, gchar **response, GError** error);
gboolean ruleManager_object_publish_event(RuleManagerObject* obj, gchar *event, gchar **response, GError** error);

gboolean ruleManager_object_fetch_all_rules(RuleManagerObject* obj, gchar *params, gchar **response, GError** error);
gboolean ruleManager_object_fetch_all_events(RuleManagerObject* obj, gchar *params,  gchar **response, GError** error);

void ruleManager_object_emitSignal(RuleManagerObject* obj, 
                                RuleManagerObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
