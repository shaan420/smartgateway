AUTOMAKE_OPTIONS = foreign

CXXFLAGS=-Wall -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

bin_PROGRAMS = http_rest_server
http_rest_server_SOURCES = httphandler.cpp RestAPI.cpp Executor.cpp strutil.cpp
http_rest_server_LDADD=-ldbus-glib-1 -ldbus-1 -lpthread -lrt -lgobject-2.0 -lglib-2.0 -lz -lmicrohttpd -lboost_regex -lboost_system -lboost_program_options -lboost_filesystem
