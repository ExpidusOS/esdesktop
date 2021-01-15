# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

bin_PROGRAMS = esdesktop

esdesktop_built_sources = \
	expidus-desktop-enum-types.c \
	expidus-desktop-enum-types.h \
	esdesktop-file-manager-proxy.c \
	esdesktop-file-manager-proxy.h \
	esdesktop-lunar-proxy.c \
	esdesktop-lunar-proxy.h \
	esdesktop-trash-proxy.c \
	esdesktop-trash-proxy.h

if HAVE_LIBNOTIFY
esdesktop_notify_sources = \
	esdesktop-notify.c \
	esdesktop-notify.h
endif

esdesktop_SOURCES = \
	$(esdesktop_built_sources) \
	$(esdesktop_notify_sources) \
	main.c \
	menu.c \
	menu.h \
	windowlist.c \
	windowlist.h \
	expidus-backdrop.c \
	expidus-backdrop.h \
	expidus-workspace.c \
	expidus-workspace.h \
	expidus-desktop.c \
	expidus-desktop.h \
	esdesktop-application.c \
	esdesktop-application.h

desktop_icon_sources = \
	esdesktop-icon.c \
	esdesktop-icon.h \
	esdesktop-icon-view.c \
	esdesktop-icon-view.h \
	esdesktop-icon-view-manager.c \
	esdesktop-icon-view-manager.h \
	esdesktop-window-icon.c \
	esdesktop-window-icon.h \
	esdesktop-window-icon-manager.c \
	esdesktop-window-icon-manager.h

desktop_file_icon_sources = \
	esdesktop-clipboard-manager.c \
	esdesktop-clipboard-manager.h \
	esdesktop-file-icon.c \
	esdesktop-file-icon.h \
	esdesktop-file-icon-manager.c \
	esdesktop-file-icon-manager.h \
	esdesktop-file-utils.c \
	esdesktop-file-utils.h \
	esdesktop-regular-file-icon.c \
	esdesktop-regular-file-icon.h \
	esdesktop-special-file-icon.c \
	esdesktop-special-file-icon.h \
	esdesktop-volume-icon.c \
	esdesktop-volume-icon.h

esdesktop_CFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/common	\
	-I$(top_builddir)/common \
	-DWNCK_I_KNOW_THIS_IS_UNSTABLE \
	-DBINDIR=\"$(bindir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DEXPIDUSMODDIR=\"$(libdir)/expidus1/modules\" \
        $(GIO_CFLAGS) \
	$(GIO_UNIX_CFLAGS) \
        $(GLIB_CFLAGS) \
	$(GTHREAD_CFLAGS) \
        $(GTK_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(LIBX11_CFLAGS) \
	$(LIBEXPIDUS1UTIL_CFLAGS) \
	$(LIBEXPIDUS1UI_CFLAGS) \
	$(LIBEXPIDUS1SMCLIENT_PRIVATE_CFLAGS) \
	$(LIBWNCK_CFLAGS) \
	$(ESCONF_CFLAGS) \
	-DENDO_API_SUBJECT_TO_CHANGE \
	$(LIBENDO_CFLAGS)

esdesktop_LDFLAGS = \
	-export-dynamic

# cygwin link order requires this split
esdesktop_LDADD = $(top_builddir)/common/libesdesktop.la
esdesktop_LDADD += \
        $(GIO_LIBS) \
	$(GIO_UNIX_LIBS) \
        $(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
        $(GTK_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(LIBX11_LDFLAGS) \
	$(LIBX11_LIBS) \
	$(LIBEXPIDUS1UTIL_LIBS) \
	$(LIBEXPIDUS1UI_LIBS) \
	$(LIBEXPIDUS1SMCLIENT_PRIVATE_LIBS) \
	$(LIBWNCK_LIBS) \
	$(ESCONF_LIBS) \
	$(LIBENDO_LIBS) \
	-lm

if BUILD_DESKTOP_MENU

esdesktop_CFLAGS += \
	$(MARKON_CFLAGS) \
	$(MARKON_GTK3_CFLAGS)

esdesktop_LDADD += \
	$(MARKON_LIBS) \
	$(MARKON_GTK3_LIBS)

endif

if ENABLE_DESKTOP_ICONS

esdesktop_SOURCES += $(desktop_icon_sources)

if ENABLE_FILE_ICONS

esdesktop_SOURCES += $(desktop_file_icon_sources)

esdesktop_CFLAGS += \
	$(LUNARX_CFLAGS)

esdesktop_LDADD += \
	$(LUNARX_LIBS)

endif
endif

if MAINTAINER_MODE

BUILT_SOURCES = \
	$(esdesktop_built_sources)

DISTCLEANFILES = \
	$(esdesktop_built_sources) \
	stamp-expidus-desktop-enum-types.h


enum_headers = \
	expidus-backdrop.h \
	expidus-desktop.h

expidus-desktop-enum-types.h: stamp-expidus-desktop-enum-types.h
	@true
stamp-expidus-desktop-enum-types.h: $(enum_headers) Makefile
	$(AM_V_GEN) ( cd $(srcdir) && glib-mkenums \
	      --fhead "#ifndef __EXPIDUS_DESKTOP_ENUM_TYPES_H__\n#define __EXPIDUS_DESKTOP_ENUM_TYPES_H__\nG_BEGIN_DECLS\n" \
	      --fprod "/* enumerations from \"@filename@\" */\n" \
	      --vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n#define EXPIDUS_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
	      --ftail "G_END_DECLS\n\n#endif /* !__EXPIDUS_DESKTOP_ENUM_TYPES_H__ */" \
	      $(enum_headers) ) >> xgen-xdeth \
	      && ( cmp -s xgen-xdeth expidus-desktop-enum-types.h || cp xgen-xdeth expidus-desktop-enum-types.h ) \
	      && rm -f xgen-xdeth \
		  && echo timestamp > $(@F)
expidus-desktop-enum-types.c: $(enum_headers) Makefile
	$(AM_V_GEN) ( cd $(srcdir) && glib-mkenums \
	      --fhead "#undef GTK_DISABLE_DEPRECATED\n#define GTK_ENABLE_BROKEN\n#include \"expidus-desktop.h\"\n#include \"expidus-backdrop.h\"\n#include \"expidus-desktop-enum-types.h\"" \
	      --fprod "\n/* enumerations from \"@filename@\" */" \
	      --vhead "GType\n@enum_name@_get_type (void)\n{\n\tstatic GType type = 0;\n\tif (type == 0) {\n\tstatic const G@Type@Value values[] = {"\
	      --vprod "\t{ @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
	      --vtail "\t{ 0, NULL, NULL }\n\t};\n\ttype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n\treturn type;\n}\n" \
	      $(enum_headers) ) >> xgen-xdetc \
	      && cp xgen-xdetc expidus-desktop-enum-types.c \
	      && rm -f xgen-xdetc


esdesktop-file-manager-proxy.c esdesktop-file-manager-proxy.h : $(srcdir)/esdesktop-file-manager-dbus.xml Makefile
	gdbus-codegen \
		--c-namespace=Esdesktop \
		--interface-prefix=com.expidus. \
		--generate-c-code=esdesktop-file-manager-proxy \
		$(srcdir)/esdesktop-file-manager-dbus.xml

esdesktop-lunar-proxy.c esdesktop-lunar-proxy.h : $(srcdir)/esdesktop-lunar-dbus.xml Makefile
	gdbus-codegen \
		--c-namespace=Esdesktop \
		--interface-prefix=com.expidus. \
		--generate-c-code=esdesktop-lunar-proxy \
		$(srcdir)/esdesktop-lunar-dbus.xml

esdesktop-trash-proxy.c esdesktop-trash-proxy.h : $(srcdir)/esdesktop-trash-dbus.xml Makefile
	gdbus-codegen \
		--c-namespace=Esdesktop \
		--interface-prefix=com.expidus. \
		--generate-c-code=esdesktop-trash-proxy \
		$(srcdir)/esdesktop-trash-dbus.xml

endif

EXTRA_DIST = \
	$(desktop_icon_sources) \
	$(desktop_file_icon_sources)
