AUTOMAKE_OPTIONS = foreign

CXXFLAGS=-Wall -I ~/XSB_galileo/emu/ -I ~/XSB_galileo/config/x86_64-unknown-linux-gnu-mt/ -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/local/include/ -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
CFLAGS=-Wall -I ~/XSB_galileo/emu/ -I ~/XSB_galileo/config/x86_64-unknown-linux-gnu-mt/ -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/local/include/ -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""

bin_PROGRAMS = rule_manager
rule_manager_SOURCES = Rule.cpp RuleManager.cpp RuleManagerObject.c url_utils.cpp main.cpp
rule_manager_LDADD= ~/XSB_galileo/config/x86_64-unknown-linux-gnu-mt/saved.o/xsb.o -ldbus-glib-1 -ldbus-1 -lpthread -lrt -lgobject-2.0 -lglib-2.0 -ldl
