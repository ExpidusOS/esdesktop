/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright (c) 2004 Brian Tarricone, <bjt23@cornell.edu>
 *  Copyright (c) 2010 Jannis Pohlmann, <jannis@expidus.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 *  Random portions taken from or inspired by the original esdesktop for expidus1:
 *     Copyright (C) 2002-2003 Jasper Huijsmans (huysmans@users.sourceforge.net)
 *     Copyright (C) 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
 */

#ifndef _ESDESKTOP_COMMON_H_
#define _ESDESKTOP_COMMON_H_

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <X11/Xlib.h>

#include <stdarg.h>

#define ESDESKTOP_CHANNEL        "expidus1-desktop"
#define DEFAULT_BACKDROP         DATADIR "/backgrounds/expidus/expidus-river.png"
#define DEFAULT_ICON_FONT_SIZE   12
#define DEFAULT_ICON_SIZE        48
#define ITHEME_FLAGS             (GTK_ICON_LOOKUP_USE_BUILTIN)

#define LIST_TEXT                "# expidus backdrop list"
#define ESDESKTOP_SELECTION_FMT  "ESDESKTOP_SELECTION_%d"
#define ESDESKTOP_IMAGE_FILE_FMT "ESDESKTOP_IMAGE_FILE_%d"
#define ESDESKTOP_RC_VERSION_STAMP "esdesktop-version-1.0.0+-rcfile_format"

#define RELOAD_MESSAGE     "reload"
#define MENU_MESSAGE       "menu"
#define WINDOWLIST_MESSAGE "windowlist"
#define ARRANGE_MESSAGE    "arrange"
#define QUIT_MESSAGE       "quit"

#define SINGLE_WORKSPACE_MODE     "/backdrop/single-workspace-mode"
#define SINGLE_WORKSPACE_NUMBER   "/backdrop/single-workspace-number"

#define DESKTOP_ICONS_SHOW_THUMBNAILS        "/desktop-icons/show-thumbnails"
#define DESKTOP_ICONS_SHOW_HIDDEN_FILES      "/desktop-icons/show-hidden-files"
#define DESKTOP_ICONS_SHOW_NETWORK_REMOVABLE "/desktop-icons/file-icons/show-network-removable"
#define DESKTOP_ICONS_SHOW_DEVICE_REMOVABLE  "/desktop-icons/file-icons/show-device-removable"
#define DESKTOP_ICONS_SHOW_UNKNWON_REMOVABLE "/desktop-icons/file-icons/show-unknown-removable"
#define DESKTOP_ICONS_SHOW_HOME              "/desktop-icons/file-icons/show-home"
#define DESKTOP_ICONS_SHOW_TRASH             "/desktop-icons/file-icons/show-trash"
#define DESKTOP_ICONS_SHOW_FILESYSTEM        "/desktop-icons/file-icons/show-filesystem"
#define DESKTOP_ICONS_SHOW_REMOVABLE         "/desktop-icons/file-icons/show-removable"

#define DESKTOP_MENU_MAX_TEMPLATE_FILES     "/desktop-menu/max-template-files"

/**
 * File information namespaces queried for #GFileInfo objects.
 */
#define ESDESKTOP_FILE_INFO_NAMESPACE \
  "access::*," \
  "id::*," \
  "mountable::*," \
  "preview::*," \
  "standard::*," \
  "time::*," \
  "thumbnail::*," \
  "trash::*," \
  "unix::*," \
  "metadata::*"

/**
 * Filesystem information namespaces queried for #GFileInfo * objects.
 */
#define ESDESKTOP_FILESYSTEM_INFO_NAMESPACE \
  "filesystem::*"

G_BEGIN_DECLS

gchar* esdesktop_get_monitor_name_from_gtk_widget(GtkWidget *widget,
                                                  gint monitor_num);

gint esdesktop_compare_paths(GFile *a, GFile *b);

gboolean esdesktop_image_file_is_valid(const gchar *filename);

gchar *esdesktop_get_file_mimetype(const gchar *file);

gint expidus_translate_image_styles(gint input);

gchar* esdesktop_remove_whitspaces(gchar* str);

GtkWidget* esdesktop_menu_create_menu_item_with_markup(const gchar *name,
                                                       GtkWidget   *image);

GtkWidget* esdesktop_menu_create_menu_item_with_mnemonic(const gchar *name,
                                                         GtkWidget   *image);

void esdesktop_get_screen_dimensions(GdkScreen *gcreen,
                                     gint      *width,
                                     gint      *height);

gint esdesktop_get_monitor_num(GdkDisplay *display,
                               GdkMonitor *monitor);

gint esdesktop_get_current_monitor_num(GdkDisplay *display);

#if defined(G_HAVE_ISO_VARARGS)

#define XF_DEBUG(...) esdesktop_debug (__func__, __FILE__, __LINE__, __VA_ARGS__)

void esdesktop_debug(const char *func, const char *file, int line, const char *format, ...) __attribute__((format (printf,4,5)));

#else /* defined(G_HAVE_ISO_VARARGS) */

#define XF_DEBUG(...)

#endif /* defined(G_HAVE_ISO_VARARGS) */

void esdesktop_debug_set(gboolean debug);

G_END_DECLS

#endif
