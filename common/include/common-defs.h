#ifndef INCLUDE_COMMON_DEFS_H
#define INCLUDE_COMMON_DEFS_H
/**
 * This maemo code example is licensed under a MIT-style license,
 * that can be found in the file called "License" in the same
 * directory as this file.
 * Copyright (c) 2007 Nokia Corporation. All rights reserved.
 *
 * This file includes the common symbolic defines for both client and
 * the server. Normally this kind of information would be part of the
 * object usage documentation, but in this example we take the easy
 * way out.
 *
 * To re-iterate: You could just as easily use strings in both client
 *                and server, and that would be the more common way.
 */
//#define SMARTGATEWAY_HOME_ONTOLOGY_FILEPATH "/home/shankar/Dropbox/ASU_Courses/SmartHome/ontology/home.owl"
#define SMARTGATEWAY_HOME_ONTOLOGY_FILEPATH "/home/root/home.owl"

/* Well-known name for this service. */
#define SMARTGATEWAY_SERVICE_PATH_PREFIX        "org.asu.smarthome.smartgateway."
/* Object path to the provided object. */
#define SMARTGATEWAY_SERVICE_OBJECT_PATH_PREFIX "/org/asu/smarthome/smartgateway/"
/* And we're interested in using it through this interface.
   This must match the entry in the interface definition XML. */
#define SMARTGATEWAY_SERVICE_INTERFACE_PATH_PREFIX   "org.asu.smarthome.smartgateway."

/* Symbolic constants for the signal names to use with GLib.
   These need to map into the D-Bus signal names. */
/* 
 * The following 2 macros are used by the DeviceManager for every device
 * that sends out an async notification that its value has changed
 */
#define SIGNAL_CHANGED_STATUS    "changed_status"
#define SIGNAL_ERROR "error_detected"

#define SIGNAL_NEW_KEY "new_key"
#define SIGNAL_NEW_RULE "new_rule"
#define SIGNAL_NEW_QUERY_URL "new_query_url"

#define DEVICE_STATUS_ON 1
#define DEVICE_STATUS_OFF 0
#define DEVICE_STATUS_ERROR -1

#define MAX_STR_LEN 256
#endif
