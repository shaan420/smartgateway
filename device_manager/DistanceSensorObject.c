#include "DistanceSensorDevice.hpp"

extern "C" {
#include "DistanceSensorObject.h"
/**
 * Pull in the stub for the server side.
 */
#include "distancesensor-server-stub.h"

/**
 * Per object initializer
 *
 * Only sets up internal state (both values set to zero)
 */
static void distancesensor_object_init(DistanceSensorObject* obj) {
	printf("Called");
	g_assert(obj != NULL);
	obj->m_cur_status = NULL;
}

/**
 * Per class initializer
 *
 * Sets up the thresholds (-100 .. 100), creates the signals that we
 * can emit from any object of this class and finally registers the
 * type into the GLib/D-Bus wrapper so that it may add its own magic.
 */
static void distancesensor_object_class_init(DistanceSensorObjectClass* klass) {

	/* Since all signals have the same prototype (each will get one
	   string as a parameter), we create them in a loop below. The only
	   difference between them is the index into the klass->signals
	   array, and the signal name.

	   Since the index goes from 0 to E_DISTANCESENSOR_OBJECT_SIGNAL_COUNT-1, we just specify
	   the signal names into an array and iterate over it.

	   Note that the order here must correspond to the order of the
	   enumerations before. */
	const gchar* signalNames[E_DISTANCESENSOR_OBJECT_SIGNAL_COUNT] = {
		SIGNAL_CHANGED_STATUS,
		SIGNAL_ERROR };
	/* Loop variable */
	int i;

	printf("Called");
	g_assert(klass != NULL);

	printf("Creating signals");

	/* Create the signals in one loop, since they all are similar
	   (except for the names). */
	for (i = 0; i < E_DISTANCESENSOR_OBJECT_SIGNAL_COUNT; i++) {
		guint signalId;

		/* Most of the time you will encounter the following code without
		   comments. This is why all the parameters are documented
		   directly below. */
		signalId =
					/* str name of the signal */
					g_signal_new(signalNames[i],
					/* GType to which signal is bound to */
					G_OBJECT_CLASS_TYPE(klass),
					/* Combination of GSignalFlags which tell the
					   signal dispatch machinery how and when to
					   dispatch this signal. The most common is the
					   G_SIGNAL_RUN_LAST specification. */
					G_SIGNAL_RUN_LAST,
					/* Offset into the class structure for the type
					   function pointer. Since we're implementing a
					   simple class/type, we'll leave this at zero. */
					0,
					/* GSignalAccumulator to use. We don't need one. */
					NULL,
					/* User-data to pass to the accumulator. */
					NULL,
					/* Function to use to marshal the signal data into
					   the parameters of the signal call. Luckily for
					   us, GLib (GCClosure) already defines just the
					   function that we want for a signal handler that
					   we don't expect any return values (void) and
					   one that will accept one string as parameter
					   (besides the instance pointer and pointer to
					   user-data).

					   If no such function would exist, you would need
					   to create a new one (by using glib-genmarshal
					   tool). */
					g_cclosure_marshal_VOID__STRING,
					/* Return GType of the return value. The handler
			   		   does not return anything, so we use G_TYPE_NONE
			   		   to mark that. */
					G_TYPE_NONE,
					/* Number of parameter GTypes to follow. */
					1,
					/* GType(s) of the parameters. We only have one. */
					G_TYPE_STRING);

		/* Store the signal Id into the class state, so that we can use
		   it later. */
		klass->signals[i] = signalId;

		/* Proceed with the next signal creation. */
	}
	/* All signals created. */

	printf("Binding to GLib/D-Bus");

	/* Time to bind this GType into the GLib/D-Bus wrappers.
	 * NOTE: This is not yet "publishing" the object on the D-Bus, but
	 * since it is only allowed to do this once per class
	 * creation, the safest place to put it is in the class
	 * initializer.
	 * Specifically, this function adds "method introspection
	 * data" to the class so that methods can be called over
	 * the D-Bus. */
	dbus_g_object_type_install_info(DISTANCESENSOR_TYPE_OBJECT,
			&dbus_glib_distancesensor_object_object_info);

	printf("Done");
	/* All done. Class is ready to be used for instantiating objects */
}


G_DEFINE_TYPE(DistanceSensorObject, distancesensor_object, G_TYPE_OBJECT)
/**
 * Utility helper to emit a signal given with internal enumeration and
 * the passed string as the signal data.
 *
 * Used in the setter functions below.
 */
void distancesensor_object_emitSignal(DistanceSensorObject* obj,
		DistanceSensorObjectSignalNumber num,
		const gchar* message) {

	/* In order to access the signal identifiers, we need to get a hold
	   of the class structure first. */
	DistanceSensorObjectClass* klass = DISTANCESENSOR_OBJECT_GET_CLASS(obj);

	/* Check that the given num is valid (abort if not).
	   Given that this file is the module actually using this utility,
	   you can consider this check superfluous (but useful for
	   development work). */
	g_assert((num < E_DISTANCESENSOR_OBJECT_SIGNAL_COUNT) && (num >= 0));

	printf("Emitting signal id %d, with message '%s'", num, message);

	/* This is the simplest way of emitting signals. */
	g_signal_emit(/* Instance of the object that is generating this
					 signal. This will be passed as the first parameter
					 to the signal handler (eventually). But obviously
					 when speaking about D-Bus, a signal caught on the
					 other side of D-Bus will be first processed by
					 the GLib-wrappers (the object proxy) and only then
					 processed by the signal handler. */
			obj,
			/* Signal id for the signal to generate. These are
			   stored inside the class state structure. */
			klass->signals[num],
			/* Detail of signal. Since we are not using detailed
			   signals, we leave this at zero (default). */
			0,
			/* Data to marshal into the signal. In our case it's
			   just one string. */
			message);
	/* g_signal_emit returns void, so we cannot check for success. */

	/* Done emitting signal. */
}

gboolean distancesensor_object_get_status(DistanceSensorObject* obj, gchar **cur_status, GError** error) 
{
	printf("distancesensor1: get_status called. cur_value = %s\n", obj->m_cur_status);
	g_assert(obj != NULL);

		/* Copy the current first distancesensor to caller specified memory. */
/*	if (obj->m_cur_status == DEVICE_STATUS_ERROR)
	{
		distancesensor_object_emitSignal(obj, E_DISTANCESENSOR_OBJECT_SIGNAL_ERROR, "error");
	}
	*/
	*cur_status = g_strdup(obj->m_cur_status);

	/* Return success to GLib/D-Bus wrappers. In this case we don't need
	   to touch the supplied error pointer-pointer. */
	return TRUE;
}



} /* extern "C" */

