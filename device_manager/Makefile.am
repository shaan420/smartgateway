AUTOMAKE_OPTIONS = foreign

CXXFLAGS=-Wall -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
CFLAGS=-Wall -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""

bin_PROGRAMS = device_manager
device_manager_SOURCES = Subscriber.cpp Event.cpp DeviceBase.cpp DeviceAgentObject.c DeviceAgent.cpp NotificationAgentObject.c NotificationAgentDevice.cpp GyroSensorObject.c GyroSensorDevice.cpp DistanceSensorObject.c DistanceSensorDevice.cpp LightingObject.c ledLightingDevice.cpp TemperatureSensorObject.c groveTemperatureSensorDevice.cpp LightSensorObject.c groveLightSensorDevice.cpp SoundSensorObject.c groveSoundSensorDevice.cpp TouchSensorObject.c groveTouchSensorDevice.cpp SoundObject.c groveBuzzerDevice.cpp groveLightingDevice.cpp testLightingDevice.cpp url_utils.cpp main.cpp
device_manager_LDADD=-ldbus-glib-1 -ldbus-1 -lpthread -lrt -lgobject-2.0 -lglib-2.0 -lcurl -lthingspeak -lmraa -lupm-grove -lupm-groveloudness -lupm-ttp223 -lupm-buzzer
