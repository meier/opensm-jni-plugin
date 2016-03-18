JAVAHOME=/usr/lib/jvm/java

INCLUDES = -I$(includedir)/infiniband -I$(JAVAHOME)/include -I$(JAVAHOME)/include/linux $(GLIB_CFLAGS)

lib_LTLIBRARIES = src/libOsmJniPi.la

DBGFLAGS = -g

src_libOsmJniPi_la_CFLAGS = -DENABLE_OSM_PERF_MGR -Wall $(DBGFLAGS)

if HAVE_LD_VERSION_SCRIPT
    libOsmJniPi_version_script = -Wl,--version-script=$(srcdir)/libOsmJniPi.map
else
    libOsmJniPi_version_script =
endif

src_libOsmJniPi_la_SOURCES = src/osmJniPi.c src/jni_OsmNativeInterface.c src/jni_NativeUtils.c src/jni_SharedResources.c \
	src/jni_Synchronization.c src/jni_Port.c src/jni_Node.c src/jni_PeerClass.c src/jni_Stats.c src/jni_Subnet.c src/jni_SysInfo.c \
	src/fifo.c
src_libOsmJniPi_la_LDFLAGS = -shared -fPIC -version-info $(libOsmJniPi_api_version)  \
	-export-dynamic $(libOsmJniPi_version_script)
src_libOsmJniPi_la_LIBADD = -L$(libdir) -lrt -losmcomp $(GLIB_LIBS)
src_libOsmJniPi_la_DEPENDENCIES = $(srcdir)/libOsmJniPi.map

man_MANS = man/OsmJniPi.8 man/osmConsole.8 man/OMS.7

# headers are distributed as part of the include dir
EXTRA_DIST = $(srcdir)/opensm-jni-plugin.spec.in \
		$(srcdir)/opensm-jni-plugin.spec \
		$(srcdir)/libOsmJniPi.map \
		$(srcdir)/src/*.h \
		$(srcdir)/etc/opensm-plugin/libOsmJniPi.conf \
		$(srcdir)/config \
		$(srcdir)/ChangeLog \
		$(srcdir)/INSTALL \
		$(srcdir)/LICENSE \
		$(srcdir)/README.md \
		$(srcdir)/META \
		$(srcdir)/DISCLAIMER \
		$(man_MANS)

install-data-hook:
	if test ! -d $(DESTDIR)/$(sysconfdir)/opensm-plugin; then \
		$(top_srcdir)/config/install-sh -m 755 -d $(DESTDIR)/$(sysconfdir)/opensm-plugin; \
	fi
	$(top_srcdir)/config/install-sh -c -m 640 $(top_srcdir)/etc/opensm-plugin/libOsmJniPi.conf $(DESTDIR)/$(sysconfdir)/opensm-plugin/libOsmJniPi.conf

