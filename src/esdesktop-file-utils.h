/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright(c) 2006      Brian Tarricone, <bjt23@cornell.edu>
 *  Copyright(c) 2010-2011 Jannis Pohlmann, <jannis@expidus.org>
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
 */

#ifndef __ESDESKTOP_FILE_UTILS_H__
#define __ESDESKTOP_FILE_UTILS_H__

#include <gio/gio.h>

#ifdef HAVE_LUNARX
#include <lunarx/lunarx.h>
#endif

#include "esdesktop-file-icon.h"

gboolean esdesktop_file_utils_is_desktop_file(GFileInfo *info);
gboolean esdesktop_file_utils_file_is_executable(GFileInfo *info);
gchar *esdesktop_file_utils_format_time_for_display(guint64 file_time);
GKeyFile *esdesktop_file_utils_query_key_file(GFile *file,
                                              GCancellable *cancellable,
                                              GError **error);
gchar *esdesktop_file_utils_get_display_name(GFile *file,
                                             GFileInfo *info);

GList *esdesktop_file_utils_file_icon_list_to_file_list(GList *icon_list);
GList *esdesktop_file_utils_file_list_from_string(const gchar *string);
gchar *esdesktop_file_utils_file_list_to_string(GList *file_list);
gchar **esdesktop_file_utils_file_list_to_uri_array(GList *file_list);
void esdesktop_file_utils_file_list_free(GList *file_list);

GdkPixbuf *esdesktop_file_utils_get_fallback_icon(gint size);

GdkPixbuf *esdesktop_file_utils_get_icon(GIcon *icon,
                                         gint width,
                                         gint height,
                                         guint opacity);

void esdesktop_file_utils_set_window_cursor(GtkWindow *window,
                                            GdkCursorType cursor_type);

gboolean esdesktop_file_utils_app_info_launch(GAppInfo *app_info,
                                              GFile *working_directory,
                                              GList *files,
                                              GAppLaunchContext *context,
                                              GError **error);

void esdesktop_file_utils_open_folder(GFile *file,
                                      GdkScreen *screen,
                                      GtkWindow *parent);
void esdesktop_file_utils_rename_file(GFile *file,
                                      GdkScreen *screen,
                                      GtkWindow *parent);
void esdesktop_file_utils_bulk_rename(GFile *working_directory,
                                      GList *files,
                                      GdkScreen *screen,
                                      GtkWindow *parent);
void esdesktop_file_utils_trash_files(GList *files,
                                       GdkScreen *screen,
                                       GtkWindow *parent);
void esdesktop_file_utils_empty_trash(GdkScreen *screen,
                                      GtkWindow *parent);
void esdesktop_file_utils_unlink_files(GList *files,
                                       GdkScreen *screen,
                                       GtkWindow *parent);
void esdesktop_file_utils_create_file(GFile *parent_folder,
                                      const gchar *content_type,
                                      GdkScreen *screen,
                                      GtkWindow *parent);
void esdesktop_file_utils_create_file_from_template(GFile *parent_folder,
                                                    GFile *template_file,
                                                    GdkScreen *screen,
                                                    GtkWindow *parent);
void esdesktop_file_utils_show_properties_dialog(GFile *file,
                                                 GdkScreen *screen,
                                                 GtkWindow *parent);
void esdesktop_file_utils_launch(GFile *file,
                                 GdkScreen *screen,
                                 GtkWindow *parent);
gboolean esdesktop_file_utils_execute(GFile *working_directory,
                                      GFile *file,
                                      GList *files,
                                      GdkScreen *screen,
                                      GtkWindow *parent);
void esdesktop_file_utils_display_chooser_dialog(GFile *file,
                                                 gboolean open,
                                                 GdkScreen *screen,
                                                 GtkWindow *parent);
void esdesktop_file_utils_transfer_file(GdkDragAction action,
                                        GFile *source_file,
                                        GFile *target_file,
                                        GdkScreen *screen);
gboolean esdesktop_file_utils_transfer_files(GdkDragAction action,
                                             GList *source_files,
                                             GList *target_files,
                                             GdkScreen *screen);


gboolean esdesktop_file_utils_dbus_init(void);
void esdesktop_file_utils_dbus_cleanup(void);



#ifdef HAVE_LUNARX
gchar *esdesktop_lunarx_file_info_get_name(LunarxFileInfo *file_info);
gchar *esdesktop_lunarx_file_info_get_uri(LunarxFileInfo *file_info);
gchar *esdesktop_lunarx_file_info_get_parent_uri(LunarxFileInfo *file_info);
gchar *esdesktop_lunarx_file_info_get_uri_scheme_file(LunarxFileInfo *file_info);
gchar *esdesktop_lunarx_file_info_get_mime_type(LunarxFileInfo *file_info);
gboolean esdesktop_lunarx_file_info_has_mime_type(LunarxFileInfo *file_info,
                                                   const gchar *mime_type);
gboolean esdesktop_lunarx_file_info_is_directory(LunarxFileInfo *file_info);
GFile *esdesktop_lunarx_file_info_get_location(LunarxFileInfo *file_info);
GFileInfo *esdesktop_lunarx_file_info_get_file_info(LunarxFileInfo *file_info);
GFileInfo *esdesktop_lunarx_file_info_get_filesystem_info(LunarxFileInfo *file_info);
#endif

#endif
