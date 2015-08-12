#ifndef __SOUND_OBJECT_H__
#define __SOUND_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_SOUND_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_SOUND_OBJECT_SIGNAL_ERROR,
	E_SOUND_OBJECT_SIGNAL_COUNT
} SoundObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	const gchar *m_cur_status;
gint m_new_status;

	void *m_dev_ptr;
} SoundObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_SOUND_OBJECT_SIGNAL_COUNT];
} SoundObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType sound_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define SOUND_TYPE_OBJECT              (sound_object_get_type())

#define SOUND_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 SOUND_TYPE_OBJECT, SoundObject))
#define SOUND_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  SOUND_TYPE_OBJECT, SoundObjectClass))
#define SOUND_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 SOUND_TYPE_OBJECT))
#define SOUND_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  SOUND_TYPE_OBJECT))
#define SOUND_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								SOUND_TYPE_OBJECT, SoundObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean sound_object_get_status(SoundObject* obj, gchar * *cur_status, GError** error); 

gboolean sound_object_set_status(SoundObject* obj, gint new_status, GError** error); 



void sound_object_emitSignal(SoundObject* obj, 
                                SoundObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
