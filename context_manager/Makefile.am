AUTOMAKE_OPTIONS = foreign

#HOME=/home/ayanami-dummy
HOME=/home/shankar

#BOOST_LIB_PATH=$(HOME)/boost_1_55_0/bin.v2/libs
BOOST_LIB_PATH=$(HOME)/owlcpp-libs-galileo

OWLCPP_LIB_PATH=$(HOME)/owlcpp-master/out/bin
OWLCPP_LIB_PATH_EXT=$(HOME)/owlcpp-master/out/ext

CXXFLAGS=-Wall -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/$(HOME)/owlcpp-master/include -I/$(HOME)/owlcpp-master/out/include -I ~/boost_1_55_0/ -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""
CFLAGS=-Wall -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include/ -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/$(HOME)/owlcpp-master/include -I/$(HOME)/owlcpp-master/out/include -I ~/boost_1_55_0/ -DVERSION=\"$(VERSION)\" -DPROG="\"$(PACKAGE)\""

bin_PROGRAMS = context_manager
context_manager_SOURCES = ContextManager.cpp ContextManagerObject.c OntologyManager.cpp DeviceCatalog.cpp url_utils.cpp main.cpp
context_manager_LDADD=-L $(BOOST_LIB_PATH) -L $(OWLCPP_LIB_PATH_EXT)/factpp/factpp/gcc-x86/release/link-static -L $(OWLCPP_LIB_PATH)/rdf/gcc-x86/release/link-static -L $(OWLCPP_LIB_PATH)/logic/gcc-x86/release/link-static -L $(OWLCPP_LIB_PATH)/io/gcc-x86/release/link-static -L $(OWLCPP_LIB_PATH_EXT)/libxml2/libxml2/gcc-x86/release/libxml2-version-2.9.0/link-static -L $(OWLCPP_LIB_PATH_EXT)/raptor/raptor/gcc-x86/release/link-static/raptor-version-2.0.8/ -ldbus-glib-1 -ldbus-1 -lpthread -lrt -lgobject-2.0 -lglib-2.0 -lfactpp_kernel -lowlcpp_io -lowlcpp_logic -lowlcpp_rdf -lraptor $(HOME)/owlcpp-libs-galileo/libxml2.a -lz -llzma $(HOME)/owlcpp-libs-galileo/libboost_filesystem.a $(HOME)/owlcpp-libs-galileo/libboost_program_options.a $(HOME)/owlcpp-libs-galileo/libboost_system.a $(HOME)/owlcpp-libs-galileo/libboost_regex.a
