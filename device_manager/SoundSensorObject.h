#ifndef __SOUNDSENSOR_OBJECT_H__
#define __SOUNDSENSOR_OBJECT_H__
#include "../common/include/common-defs.h"

extern "C" {

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef enum {
	E_SOUNDSENSOR_OBJECT_SIGNAL_CHANGED_STATUS = 0,
	E_SOUNDSENSOR_OBJECT_SIGNAL_ERROR,
	E_SOUNDSENSOR_OBJECT_SIGNAL_COUNT
} SoundSensorObjectSignalNumber;

typedef struct {
	/* The parent class object state. */
	GObject m_parent;
	/* Our first per-object state variable. */
	const gchar * m_cur_status;

	void *m_dev_ptr;
} SoundSensorObject;

typedef struct {
	/* The parent class state. */
	GObjectClass m_parent;
	/* Signals created for this class. */
	guint signals[E_SOUNDSENSOR_OBJECT_SIGNAL_COUNT];
} SoundSensorObjectClass;

/* Forward declaration of the function that will return the GType of
   the Value implementation. Not used in this program. */
GType soundsensor_object_get_type(void);

/* Macro for the above. It is common to define macros using the
   naming convention (seen below) for all GType implementations,
   and that's why we're going to do that here as well. */
#define SOUNDSENSOR_TYPE_OBJECT              (soundsensor_object_get_type())

#define SOUNDSENSOR_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_CAST ((object), \
								 SOUNDSENSOR_TYPE_OBJECT, SoundSensorObject))
#define SOUNDSENSOR_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST ((klass), \
							  SOUNDSENSOR_TYPE_OBJECT, SoundSensorObjectClass))
#define SOUNDSENSOR_IS_OBJECT(object) \
	(G_TYPE_CHECK_INSTANCE_TYPE ((object), \
								 SOUNDSENSOR_TYPE_OBJECT))
#define SOUNDSENSOR_IS_OBJECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
							  SOUNDSENSOR_TYPE_OBJECT))
#define SOUNDSENSOR_OBJECT_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), \
								SOUNDSENSOR_TYPE_OBJECT, SoundSensorObjectClass))

/**
 * Since the stub generator will reference the functions from a call
 * table, the functions must be declared before the stub is included.
 */
gboolean soundsensor_object_get_status(SoundSensorObject* obj, gchar * *cur_status, GError** error); 



void soundsensor_object_emitSignal(SoundSensorObject* obj, 
                                SoundSensorObjectSignalNumber num, 
								const gchar* message);

} /* extern "C" */

#endif
