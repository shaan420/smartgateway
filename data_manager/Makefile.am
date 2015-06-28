AUTOMAKE_OPTIONS = foreign

CXXFLAGS=-Wall -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/local/include/rasqal/ -I/usr/local/include/raptor2/ -I/usr/local/include/ -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
CFLAGS=-Wall -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/local/include/rasqal/ -I/usr/local/include/raptor2/ -I/usr/local/include/ -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""

bin_PROGRAMS = data_manager
data_manager_SOURCES = DevStorage.cpp TimeManager.cpp DataManager.cpp DataManagerObject.c url_utils.cpp main.cpp
data_manager_LDADD=-ldbus-glib-1 -ldbus-1 -lpthread -lrt -lgobject-2.0 -lglib-2.0 -lthingspeak
