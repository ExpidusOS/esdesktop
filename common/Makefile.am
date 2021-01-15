noinst_LTLIBRARIES = libesdesktop.la

libesdesktop_built_sources = \
	esdesktop-marshal.c \
	esdesktop-marshal.h \
	tumbler.c \
	tumbler.h

libesdesktop_la_SOURCES = \
	$(libesdesktop_built_sources) \
	esdesktop-common.c \
	esdesktop-common.h \
	esdesktop-thumbnailer.c \
	esdesktop-thumbnailer.h

libesdesktop_la_CFLAGS = \
	-I$(top_srcdir)/src \
	$(LIBEXPIDUS1UTIL_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS)


if MAINTAINER_MODE

esdesktop-marshal.h: esdesktop-marshal.list Makefile
	$(AM_V_GEN) glib-genmarshal --prefix=esdesktop_marshal --internal --header $< > $@

esdesktop-marshal.c: esdesktop-marshal.list Makefile
	$(AM_V_GEN) echo '#include "esdesktop-marshal.h"' > $@ \
	&& glib-genmarshal --prefix=esdesktop_marshal --body $< >> $@

tumbler.c tumbler.h : $(srcdir)/tumbler-service-dbus.xml Makefile.am
	gdbus-codegen \
		--c-namespace=Tumbler \
		--interface-prefix=org.freedesktop.thumbnails. \
		--generate-c-code=tumbler \
		$(srcdir)/tumbler-service-dbus.xml

DISTCLEANFILES = \
	$(libesdesktop_built_sources)

BUILT_SOURCES = \
	$(libesdesktop_built_sources)

endif

EXTRA_DIST = \
	esdesktop-marshal.list
