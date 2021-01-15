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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gio/gio.h>
#ifdef HAVE_GIO_UNIX
#include <gio/gunixmounts.h>
#endif

#include <gtk/gtk.h>

#include <libexpidus1ui/libexpidus1ui.h>

#include <endo/endo.h>

#ifdef HAVE_LUNARX
#include <lunarx/lunarx.h>
#endif

#include "esdesktop-common.h"
#include "esdesktop-file-icon.h"
#include "esdesktop-file-manager-proxy.h"
#include "esdesktop-file-utils.h"
#include "esdesktop-trash-proxy.h"
#include "esdesktop-lunar-proxy.h"

static void esdesktop_file_utils_add_emblems(GdkPixbuf *pix, GList *emblems);

static EsdesktopTrash       *esdesktop_file_utils_peek_trash_proxy(void);
static EsdesktopFileManager *esdesktop_file_utils_peek_filemanager_proxy(void);

static void esdesktop_file_utils_trash_proxy_new_cb (GObject *source_object,
                                                     GAsyncResult *res,
                                                     gpointer user_data);

static void esdesktop_file_utils_file_manager_proxy_new_cb (GObject *source_object,
                                                            GAsyncResult *res,
                                                            gpointer user_data);

static void esdesktop_file_utils_lunar_proxy_new_cb (GObject *source_object,
                                                      GAsyncResult *res,
                                                      gpointer user_data);

#ifdef HAVE_LUNARX
static EsdesktopLunar *esdesktop_file_utils_peek_lunar_proxy(void);
#else
static gpointer esdesktop_file_utils_peek_lunar_proxy(void);
#endif

gboolean
esdesktop_file_utils_is_desktop_file(GFileInfo *info)
{
    const gchar *content_type;
    gboolean is_desktop_file = FALSE;

    content_type = g_file_info_get_content_type(info);
    if(content_type)
        is_desktop_file = g_content_type_equals(content_type, "application/x-desktop");

    return is_desktop_file
        && !g_str_has_suffix(g_file_info_get_name(info), ".directory");
}

gboolean
esdesktop_file_utils_file_is_executable(GFileInfo *info)
{
    const gchar *content_type;
    gboolean can_execute = FALSE;

    g_return_val_if_fail(G_IS_FILE_INFO(info), FALSE);

    if(g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)) {
        /* get the content type of the file */
        content_type = g_file_info_get_content_type(info);
        if(content_type != NULL) {
#ifdef G_OS_WIN32
            /* check for .exe, .bar or .com */
            can_execute = g_content_type_can_be_executable(content_type);
#else
            /* check if the content type is save to execute, we don't use
             * g_content_type_can_be_executable() for unix because it also returns
             * true for "text/plain" and we don't want that */
            if(g_content_type_is_a(content_type, "application/x-executable")
               || g_content_type_is_a(content_type, "application/x-shellscript"))
            {
                can_execute = TRUE;
            }
#endif
        }
    }

    return can_execute || esdesktop_file_utils_is_desktop_file(info);
}

gchar *
esdesktop_file_utils_format_time_for_display(guint64 file_time)
{
    const gchar *date_format;
    struct tm *tfile;
    time_t ftime;
    GDate dfile;
    GDate dnow;
    gchar buffer[128];
    gint diff;

    /* check if the file_time is valid */
    if(file_time != 0) {
        ftime = (time_t) file_time;

        /* determine the local file time */
        tfile = localtime(&ftime);

        /* setup the dates for the time values */
        g_date_set_time_t(&dfile, (time_t) ftime);
        g_date_set_time_t(&dnow, time(NULL));

        /* determine the difference in days */
        diff = g_date_get_julian(&dnow) - g_date_get_julian(&dfile);
        if(diff == 0) {
            /* TRANSLATORS: file was modified less than one day ago */
            strftime(buffer, 128, _("Today at %X"), tfile);
            return g_strdup(buffer);
        } else if(diff == 1) {
            /* TRANSLATORS: file was modified less than two days ago */
            strftime(buffer, 128, _("Yesterday at %X"), tfile);
            return g_strdup(buffer);
        } else {
            if (diff > 1 && diff < 7) {
                /* Days from last week */
                date_format = _("%A at %X");
            } else {
                /* Any other date */
                date_format = _("%x at %X");
            }

            /* format the date string accordingly */
            strftime(buffer, 128, date_format, tfile);
            return g_strdup(buffer);
        }
    }

    /* the file_time is invalid */
    return g_strdup(_("Unknown"));
}

GKeyFile *
esdesktop_file_utils_query_key_file(GFile *file,
                                    GCancellable *cancellable,
                                    GError **error)
{
    GKeyFile *key_file;
    gchar *contents = NULL;
    gsize length;

    g_return_val_if_fail(G_IS_FILE(file), NULL);
    g_return_val_if_fail(cancellable == NULL || G_IS_CANCELLABLE(cancellable), NULL);
    g_return_val_if_fail(error == NULL || *error == NULL, NULL);

    /* try to load the entire file into memory */
    if (!g_file_load_contents(file, cancellable, &contents, &length, NULL, error))
        return NULL;

    /* allocate a new key file */
    key_file = g_key_file_new();

    /* try to parse the key file from the contents of the file */
    if (length == 0
        || g_key_file_load_from_data(key_file, contents, length,
                                     G_KEY_FILE_KEEP_COMMENTS
                                     | G_KEY_FILE_KEEP_TRANSLATIONS,
                                     error))
    {
        g_free(contents);
        return key_file;
    }
    else
    {
        g_free(contents);
        g_key_file_free(key_file);
        return NULL;
    }
}

gchar *
esdesktop_file_utils_get_display_name(GFile *file,
                                      GFileInfo *info)
{
    GKeyFile *key_file;
    gchar *display_name = NULL;

    g_return_val_if_fail(G_IS_FILE_INFO(info), NULL);

    /* check if we have a desktop entry */
    if(esdesktop_file_utils_is_desktop_file(info)) {
        /* try to load its data into a GKeyFile */
        key_file = esdesktop_file_utils_query_key_file(file, NULL, NULL);
        if(key_file) {
            /* try to parse the display name */
            display_name = g_key_file_get_locale_string(key_file,
                                                        G_KEY_FILE_DESKTOP_GROUP,
                                                        G_KEY_FILE_DESKTOP_KEY_NAME,
                                                        NULL,
                                                        NULL);

            /* free the key file */
            g_key_file_free (key_file);
        }
    }

    /* use the default display name as a fallback */
    if(!display_name
       || *display_name == '\0'
       || !g_utf8_validate(display_name, -1, NULL))
    {
        display_name = g_strdup(g_file_info_get_display_name(info));
    }

    return display_name;
}

GList *
esdesktop_file_utils_file_icon_list_to_file_list(GList *icon_list)
{
    GList *file_list = NULL, *l;
    EsdesktopFileIcon *icon;
    GFile *file;

    for(l = icon_list; l; l = l->next) {
        icon = ESDESKTOP_FILE_ICON(l->data);
        file = esdesktop_file_icon_peek_file(icon);
        if(file)
            file_list = g_list_prepend(file_list, g_object_ref(file));
    }

    return g_list_reverse(file_list);
}

GList *
esdesktop_file_utils_file_list_from_string(const gchar *string)
{
    GList *list = NULL;
    gchar **uris;
    gsize n;

    uris = g_uri_list_extract_uris(string);

    for (n = 0; uris != NULL && uris[n] != NULL; ++n)
      list = g_list_append(list, g_file_new_for_uri(uris[n]));

    g_strfreev (uris);

    return list;
}

gchar *
esdesktop_file_utils_file_list_to_string(GList *list)
{
    GString *string;
    GList *lp;
    gchar *uri;

    /* allocate initial string */
    string = g_string_new(NULL);

    for (lp = list; lp != NULL; lp = lp->next) {
        uri = g_file_get_uri(lp->data);
        string = g_string_append(string, uri);
        g_free(uri);

        string = g_string_append(string, "\r\n");
      }

    return g_string_free(string, FALSE);
}

gchar **
esdesktop_file_utils_file_list_to_uri_array(GList *file_list)
{
    GList *lp;
    gchar **uris = NULL;
    guint list_length, n;

    list_length = g_list_length(file_list);

    uris = g_new0(gchar *, list_length + 1);
    for (n = 0, lp = file_list; lp != NULL; ++n, lp = lp->next)
        uris[n] = g_file_get_uri(lp->data);
    uris[n] = NULL;

    return uris;
}

void
esdesktop_file_utils_file_list_free(GList *file_list)
{
  g_list_foreach(file_list, (GFunc) g_object_unref, NULL);
  g_list_free(file_list);
}

static GdkPixbuf *esdesktop_fallback_icon = NULL;
static gint esdesktop_fallback_icon_size = -1;

GdkPixbuf *
esdesktop_file_utils_get_fallback_icon(gint size)
{
    g_return_val_if_fail(size > 0, NULL);

    if(size != esdesktop_fallback_icon_size && esdesktop_fallback_icon) {
        g_object_unref(G_OBJECT(esdesktop_fallback_icon));
        esdesktop_fallback_icon = NULL;
    }

    if(!esdesktop_fallback_icon) {
        esdesktop_fallback_icon = gdk_pixbuf_new_from_file_at_size(DATADIR "/pixmaps/esdesktop/esdesktop-fallback-icon.png",
                                                                   size,
                                                                   size,
                                                                   NULL);
    }

    if(G_UNLIKELY(!esdesktop_fallback_icon)) {
        /* this is kinda crappy, but hopefully should never happen */
        esdesktop_fallback_icon = gtk_icon_theme_load_icon(gtk_icon_theme_get_default(),
                                                           "image-missing",
                                                           size,
                                                           GTK_ICON_LOOKUP_USE_BUILTIN,
                                                           NULL);
        if(gdk_pixbuf_get_width(esdesktop_fallback_icon) != size
           || gdk_pixbuf_get_height(esdesktop_fallback_icon) != size)
        {
            GdkPixbuf *tmp = gdk_pixbuf_scale_simple(esdesktop_fallback_icon,
                                                     size, size,
                                                     GDK_INTERP_BILINEAR);
            g_object_unref(G_OBJECT(esdesktop_fallback_icon));
            esdesktop_fallback_icon = tmp;
        }
    }

    esdesktop_fallback_icon_size = size;

    return GDK_PIXBUF(g_object_ref(G_OBJECT(esdesktop_fallback_icon)));
}

GdkPixbuf *
esdesktop_file_utils_get_icon(GIcon *icon,
                              gint width,
                              gint height,
                              guint opacity)
{
    GtkIconTheme *itheme = gtk_icon_theme_get_default();
    GdkPixbuf *pix_theme = NULL, *pix = NULL;
    GIcon *base_icon = NULL;
    gint size = MIN(width, height);

    g_return_val_if_fail(width > 0 && height > 0 && icon != NULL, NULL);

    /* Extract the base icon if available */
    if(G_IS_EMBLEMED_ICON(icon))
        base_icon = g_emblemed_icon_get_icon(G_EMBLEMED_ICON(icon));
    else
        base_icon = icon;

    if(!base_icon)
        return NULL;

    if(G_IS_THEMED_ICON(base_icon)) {
      GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon(itheme,
                                                              base_icon, size,
                                                              ITHEME_FLAGS);
      if(icon_info) {
          pix_theme = gtk_icon_info_load_icon(icon_info, NULL);
          g_object_unref(icon_info);
      }
    } else if(G_IS_LOADABLE_ICON(base_icon)) {
        GInputStream *stream = g_loadable_icon_load(G_LOADABLE_ICON(base_icon),
                                                    size, NULL, NULL, NULL);
        if(stream) {
            pix = gdk_pixbuf_new_from_stream_at_scale(stream, width, height, TRUE, NULL, NULL);
            g_object_unref(stream);
        }
    } else if(G_IS_FILE_ICON(base_icon)) {
        GFile *file = g_file_icon_get_file(G_FILE_ICON(icon));
        gchar *path = g_file_get_path(file);

        pix = gdk_pixbuf_new_from_file_at_size(path, width, height, NULL);

        g_free(path);
        g_object_unref(file);
    }


    if(pix_theme) {
        GdkPixbuf *tmp;
        /* we can't edit thsese icons */
        tmp = gdk_pixbuf_copy(pix_theme);

        /* ensure icons are within our size requirements since
         * gtk_icon_theme_lookup_by_gicon isn't exact */
        pix = endo_gdk_pixbuf_scale_down(tmp, TRUE, width, height);

        g_object_unref(G_OBJECT(tmp));
        g_object_unref(G_OBJECT(pix_theme));
        pix_theme = tmp = NULL;
    }

    /* fallback */
    if(G_UNLIKELY(!pix))
        pix = esdesktop_file_utils_get_fallback_icon(size);

    /* sanity check */
    if(G_UNLIKELY(!pix)) {
        g_warning("Unable to find fallback icon");
        return NULL;
    }

    /* Add the emblems */
    if(G_IS_EMBLEMED_ICON(icon))
        esdesktop_file_utils_add_emblems(pix, g_emblemed_icon_get_emblems(G_EMBLEMED_ICON(icon)));

    if(opacity != 100) {
        GdkPixbuf *tmp = endo_gdk_pixbuf_lucent(pix, opacity);
        g_object_unref(G_OBJECT(pix));
        pix = tmp;
    }

    return pix;
}

static void
esdesktop_file_utils_add_emblems(GdkPixbuf *pix, GList *emblems)
{
    GdkPixbuf *emblem_pix = NULL;
    gint max_emblems;
    gint pix_width, pix_height;
    gint emblem_size;
    gint dest_x, dest_y, dest_width, dest_height;
    gint position;
    GList *iter;
    GtkIconTheme *itheme = gtk_icon_theme_get_default();

    g_return_if_fail(pix != NULL);

    pix_width = gdk_pixbuf_get_width(pix);
    pix_height = gdk_pixbuf_get_height(pix);

    emblem_size = MIN(pix_width, pix_height) / 2;

    /* render up to four emblems for sizes from 48 onwards, else up to 2 emblems */
    max_emblems = (pix_height < 48 && pix_width < 48) ? 2 : 4;

    for(iter = g_list_last(emblems), position = 0;
        iter != NULL && position < max_emblems; iter = iter->prev) {
        /* extract the icon from the emblem and load it */
        GIcon *emblem = g_emblem_get_icon(iter->data);
        GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon(itheme,
                                                                emblem,
                                                                emblem_size,
                                                                ITHEME_FLAGS);
        if(icon_info) {
            emblem_pix = gtk_icon_info_load_icon(icon_info, NULL);
            g_object_unref(icon_info);
        }

        if(emblem_pix) {
            if(gdk_pixbuf_get_width(emblem_pix) != emblem_size
               || gdk_pixbuf_get_height(emblem_pix) != emblem_size)
            {
                GdkPixbuf *tmp = gdk_pixbuf_scale_simple(emblem_pix,
                                                         emblem_size,
                                                         emblem_size,
                                                         GDK_INTERP_BILINEAR);
                g_object_unref(emblem_pix);
                emblem_pix = tmp;
            }

            dest_width = pix_width - emblem_size;
            dest_height = pix_height - emblem_size;

            switch(position) {
                case 0: /* bottom right */
                    dest_x = dest_width;
                    dest_y = dest_height;
                    break;
                case 1: /* bottom left */
                    dest_x = 0;
                    dest_y = dest_height;
                    break;
                case 2: /* upper left */
                    dest_x = dest_y = 0;
                    break;
                case 3: /* upper right */
                    dest_x = dest_width;
                    dest_y = 0;
                    break;
                default:
                    g_warning("Invalid emblem position in esdesktop_file_utils_add_emblems");
            }

            DBG("calling gdk_pixbuf_composite(%p, %p, %d, %d, %d, %d, %d, %d, %.1f, %.1f, %d, %d) pixbuf w: %d h: %d",
                emblem_pix, pix,
                dest_x, dest_y,
                emblem_size, emblem_size,
                dest_x, dest_y,
                1.0, 1.0, GDK_INTERP_BILINEAR, 255, pix_width, pix_height);

            /* Add the emblem */
            gdk_pixbuf_composite(emblem_pix, pix,
                                 dest_x, dest_y,
                                 emblem_size, emblem_size,
                                 dest_x, dest_y,
                                 1.0, 1.0, GDK_INTERP_BILINEAR, 255);

            g_object_unref(emblem_pix);
            emblem_pix = NULL;

            position++;
        }
    }
}

void
esdesktop_file_utils_set_window_cursor(GtkWindow *window,
                                       GdkCursorType cursor_type)
{
    GdkCursor *cursor;

    if(!window || !gtk_widget_get_window(GTK_WIDGET(window)))
        return;

    cursor = gdk_cursor_new_for_display(gtk_widget_get_display(GTK_WIDGET(window)), cursor_type);
    if(G_LIKELY(cursor)) {
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(window)), cursor);
        g_object_unref(cursor);
    }
}

static gchar *
esdesktop_file_utils_change_working_directory (const gchar *new_directory)
{
  gchar *old_directory;

  g_return_val_if_fail(new_directory && *new_directory != '\0', NULL);

  /* allocate a path buffer for the old working directory */
  old_directory = g_malloc0(sizeof(gchar) * MAXPATHLEN);

  /* try to determine the current working directory */
#ifdef G_PLATFORM_WIN32
  if(!_getcwd(old_directory, MAXPATHLEN))
#else
  if(!getcwd (old_directory, MAXPATHLEN))
#endif
  {
      /* working directory couldn't be determined, reset the buffer */
      g_free(old_directory);
      old_directory = NULL;
  }

  /* try switching to the new working directory */
#ifdef G_PLATFORM_WIN32
  if(_chdir (new_directory))
#else
  if(chdir (new_directory))
#endif
  {
      /* switching failed, we don't need to return the old directory */
      g_free(old_directory);
      old_directory = NULL;
  }

  return old_directory;
}

gboolean
esdesktop_file_utils_app_info_launch(GAppInfo *app_info,
                                     GFile *working_directory,
                                     GList *files,
                                     GAppLaunchContext *context,
                                     GError **error)
{
    gboolean result = FALSE;
    gchar *new_path = NULL;
    gchar *old_path = NULL;

    g_return_val_if_fail(G_IS_APP_INFO(app_info), FALSE);
    g_return_val_if_fail(working_directory == NULL || G_IS_FILE(working_directory), FALSE);
    g_return_val_if_fail(files != NULL && files->data != NULL, FALSE);
    g_return_val_if_fail(G_IS_APP_LAUNCH_CONTEXT(context), FALSE);
    g_return_val_if_fail(error == NULL || *error == NULL, FALSE);

    /* check if we want to set the working directory of the spawned app */
    if(working_directory) {
        /* determine the working directory path */
        new_path = g_file_get_path(working_directory);
        if(new_path) {
            /* switch to the desired working directory, remember that
             * of esdesktop itself */
            old_path = esdesktop_file_utils_change_working_directory(new_path);

            /* forget about the new working directory path */
            g_free(new_path);
        }
    }

    /* launch the paths with the specified app info */
    result = g_app_info_launch(app_info, files, context, error);

    /* check if we need to reset the working directory to the one esdesktop was
     * opened from */
    if(old_path) {
        /* switch to esdesktop's original working directory */
        new_path = esdesktop_file_utils_change_working_directory(old_path);

        /* clean up */
        g_free (new_path);
        g_free (old_path);
    }

    return result;
}

void
esdesktop_file_utils_open_folder(GFile *file,
                                 GdkScreen *screen,
                                 GtkWindow *parent)
{
    gchar *uri = NULL;
    GError *error = NULL;

    g_return_if_fail(G_IS_FILE(file));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    uri = g_file_get_uri(file);

    if(!endo_execute_preferred_application_on_screen("FileManager",
                                                    uri,
                                                    NULL,
                                                    NULL,
                                                    screen,
                                                    &error))
    {
        expidus_message_dialog(parent,
                            _("Launch Error"), "dialog-error",
                            _("The folder could not be opened"),
                            error->message,
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);

        g_clear_error(&error);
    }

    g_free(uri);
}

static void
esdesktop_file_utils_async_handle_error(GError *error, gpointer userdata)
{
    GtkWindow *parent = GTK_WINDOW(userdata);

    if(error != NULL) {
        if(error->domain != G_IO_ERROR || error->code != G_IO_ERROR_TIMED_OUT) {
            expidus_message_dialog(parent,
                                _("Error"), "dialog-error",
                                _("The requested operation could not be completed"),
                                error->message,
                                EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                                NULL);
        }

        g_clear_error(&error);
    }
}

static void
rename_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_file_manager_call_rename_file_finish(ESDESKTOP_FILE_MANAGER(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_rename_file(GFile *file,
                                 GdkScreen *screen,
                                 GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(G_IS_FILE(file));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        gchar *uri = g_file_get_uri(file);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_file_manager_call_rename_file(fileman_proxy,
                                                uri, display_name, startup_id,
                                                NULL,
                                                rename_cb,
                                                parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_free(uri);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Rename Error"), "dialog-error",
                            _("The file could not be renamed"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
bulk_rename_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_lunar_call_bulk_rename_finish(ESDESKTOP_LUNAR(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_bulk_rename(GFile *working_directory,
                                 GList *files,
                                 GdkScreen *screen,
                                 GtkWindow *parent)
{
    EsdesktopLunar *lunar_proxy;

    g_return_if_fail(G_IS_FILE(working_directory));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    lunar_proxy = esdesktop_file_utils_peek_lunar_proxy();
    if(lunar_proxy) {
        gchar *directory = g_file_get_path(working_directory);
        guint nfiles = g_list_length(files);
        gchar **filenames = g_new0(gchar *, nfiles+1);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());
        GList *lp;
        gint n;

        /* convert GFile list into an array of filenames */
        for(n = 0, lp = files; lp != NULL; ++n, lp = lp->next)
            filenames[n] = g_file_get_basename(lp->data);
        filenames[n] = NULL;

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_lunar_call_bulk_rename(lunar_proxy,
                                          directory, (const gchar **)filenames,
                                          FALSE, display_name, startup_id,
                                          NULL,
                                          bulk_rename_cb,
                                          parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(directory);
        g_free(startup_id);
        g_strfreev(filenames);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Rename Error"), "dialog-error",
                            _("The files could not be renamed"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
unlink_files_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_file_manager_call_unlink_files_finish(ESDESKTOP_FILE_MANAGER(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_unlink_files(GList *files,
                                  GdkScreen *screen,
                                  GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(files != NULL && G_IS_FILE(files->data));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        guint nfiles = g_list_length(files);
        gchar **uris = g_new0(gchar *, nfiles+1);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());
        GList *lp;
        gint n;

        /* convert GFile list into an array of URIs */
        for(n = 0, lp = files; lp != NULL; ++n, lp = lp->next)
            uris[n] = g_file_get_uri(lp->data);
        uris[n] = NULL;

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_file_manager_call_unlink_files(fileman_proxy,
                                                 "", (const gchar **)uris,
                                                 display_name, startup_id,
                                                 NULL,
                                                 unlink_files_cb,
                                                 parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_strfreev(uris);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Delete Error"), "dialog-error",
                            _("The selected files could not be deleted"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
trash_files_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_trash_call_move_to_trash_finish(ESDESKTOP_TRASH(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_trash_files(GList *files,
                                 GdkScreen *screen,
                                 GtkWindow *parent)
{
    EsdesktopTrash *trash_proxy;

    g_return_if_fail(files != NULL && G_IS_FILE(files->data));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    trash_proxy = esdesktop_file_utils_peek_trash_proxy();
    if(trash_proxy) {
        guint nfiles = g_list_length(files);
        gchar **uris = g_new0(gchar *, nfiles+1);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());
        GList *lp;
        gint n;

        /* convert GFile list into an array of URIs */
        for(n = 0, lp = files; lp != NULL; ++n, lp = lp->next)
            uris[n] = g_file_get_uri(lp->data);
        uris[n] = NULL;

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_trash_call_move_to_trash(trash_proxy,
                                           (const gchar **)uris,
                                           display_name, startup_id,
                                           NULL,
                                           trash_files_cb,
                                           parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_strfreev(uris);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Trash Error"), "dialog-error",
                            _("The selected files could not be moved to the trash"),
                            _("This feature requires a trash service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
empty_trash_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_trash_call_empty_trash_finish(ESDESKTOP_TRASH(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_empty_trash(GdkScreen *screen,
                                 GtkWindow *parent)
{
    EsdesktopTrash *trash_proxy;

    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    trash_proxy = esdesktop_file_utils_peek_trash_proxy();
    if(trash_proxy) {
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_trash_call_empty_trash(trash_proxy,
                                         display_name, startup_id,
                                         NULL,
                                         empty_trash_cb,
                                         parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Trash Error"), "dialog-error",
                            _("Could not empty the trash"),
                            _("This feature requires a trash service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
create_file_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_file_manager_call_create_file_finish(ESDESKTOP_FILE_MANAGER(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_create_file(GFile *parent_folder,
                                 const gchar *content_type,
                                 GdkScreen *screen,
                                 GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(G_IS_FILE(parent_folder));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        gchar *parent_directory = g_file_get_uri(parent_folder);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_file_manager_call_create_file(fileman_proxy,
                                                parent_directory,
                                                content_type, display_name,
                                                startup_id,
                                                NULL,
                                                create_file_cb,
                                                parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_free(parent_directory);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Create File Error"), "dialog-error",
                            _("Could not create a new file"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
create_file_from_template_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_file_manager_call_create_file_from_template_finish(ESDESKTOP_FILE_MANAGER(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_create_file_from_template(GFile *parent_folder,
                                               GFile *template_file,
                                               GdkScreen *screen,
                                               GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(G_IS_FILE(parent_folder));
    g_return_if_fail(G_IS_FILE(template_file));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        gchar *parent_directory = g_file_get_uri(parent_folder);
        gchar *template_uri = g_file_get_uri(template_file);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_file_manager_call_create_file_from_template(fileman_proxy,
                                                              parent_directory,
                                                              template_uri,
                                                              display_name,
                                                              startup_id,
                                                              NULL,
                                                              create_file_from_template_cb,
                                                              parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_free(display_name);
        g_free(parent_directory);
    } else {
        expidus_message_dialog(parent,
                            _("Create Document Error"), "dialog-error",
                            _("Could not create a new document from the template"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
show_properties_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_file_manager_call_display_file_properties_finish(ESDESKTOP_FILE_MANAGER(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_show_properties_dialog(GFile *file,
                                            GdkScreen *screen,
                                            GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(G_IS_FILE(file));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        gchar *uri = g_file_get_uri(file);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);


        esdesktop_file_manager_call_display_file_properties(fileman_proxy,
                                                            uri, display_name, startup_id,
                                                            NULL,
                                                            show_properties_cb,
                                                            parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_free(uri);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("File Properties Error"), "dialog-error",
                            _("The file properties dialog could not be opened"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

static void
launch_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_file_manager_call_launch_files_finish(ESDESKTOP_FILE_MANAGER(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_launch(GFile *file,
                            GdkScreen *screen,
                            GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(G_IS_FILE(file));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        gchar **uris;
        GFile  *parent_file = g_file_get_parent(file);
        gchar  *parent_path = g_file_get_path(parent_file);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar  *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);

        uris = g_new0(gchar *, 2);
        uris[0] = g_file_get_uri(file);
        uris[1] = NULL;

        esdesktop_file_manager_call_launch_files(fileman_proxy, parent_path,
                                                 (const gchar * const*)uris,
                                                 display_name, startup_id,
                                                 NULL,
                                                 launch_cb,
                                                 parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_free(uris[0]);
        g_free(uris);
        g_free(parent_path);
        g_object_unref(parent_file);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Launch Error"), "dialog-error",
                            _("The file could not be opened"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

gboolean
esdesktop_file_utils_execute(GFile *working_directory,
                             GFile *file,
                             GList *files,
                             GdkScreen *screen,
                             GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;
    gboolean success = TRUE;

    g_return_val_if_fail(working_directory == NULL || G_IS_FILE(working_directory), FALSE);
    g_return_val_if_fail(G_IS_FILE(file), FALSE);
    g_return_val_if_fail(screen == NULL || GDK_IS_SCREEN(screen), FALSE);
    g_return_val_if_fail(parent == NULL || GTK_IS_WINDOW(parent), FALSE);

    if(!screen)
        screen = gdk_display_get_default_screen(gdk_display_get_default());

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        GError *error = NULL;
        gchar *working_dir = working_directory != NULL ? g_file_get_uri(working_directory) : NULL;
        gchar *uri = g_file_get_uri(file);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());
        GList *lp;
        guint n = g_list_length (files);
        gchar **uris = g_new0 (gchar *, n + 1);

        for (n = 0, lp = files; lp != NULL; ++n, lp = lp->next)
            uris[n] = g_file_get_uri(lp->data);
        uris[n] = NULL;

        /* If the working_dir wasn't set check if this is a .desktop file
         * we can parse a working dir from */
        if(working_dir == NULL) {
            GFileInfo *info = g_file_query_info(file,
                                                ESDESKTOP_FILE_INFO_NAMESPACE,
                                                G_FILE_QUERY_INFO_NONE,
                                                NULL, NULL);

            if(esdesktop_file_utils_is_desktop_file(info)) {
                ExpidusRc *rc;
                gchar *path = g_file_get_path(file);
                if(path != NULL) {
                    rc = expidus_rc_simple_open(path, TRUE);
                    if(rc != NULL) {
                        working_dir = g_strdup(expidus_rc_read_entry(rc, "Path", NULL));
                        expidus_rc_close(rc);
                    }
                    g_free(path);
                }
            }

            if(info)
                g_object_unref(info);
        }

        if(!esdesktop_file_manager_call_execute_sync(fileman_proxy,
                                                     working_dir, uri,
                                                     (const gchar **)uris,
                                                     display_name, startup_id,
                                                     NULL, &error))
        {
            gchar *filename = g_file_get_uri(file);
            gchar *name = g_filename_display_basename(filename);
            gchar *primary = g_markup_printf_escaped(_("Failed to run \"%s\""), name);

            expidus_message_dialog(parent,
                                _("Launch Error"), "dialog-error",
                                primary, error->message,
                                EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                                NULL);

            g_free(primary);
            g_free(name);
            g_free(filename);
            g_clear_error(&error);

            success = FALSE;
        }

        g_free(startup_id);
        g_strfreev(uris);
        g_free(uri);
        g_free(working_dir);
        g_free(display_name);
    } else {
        gchar *filename = g_file_get_uri(file);
        gchar *name = g_filename_display_basename(filename);
        gchar *primary = g_markup_printf_escaped(_("Failed to run \"%s\""), name);

        expidus_message_dialog(parent,
                            _("Launch Error"), "dialog-error",
                            primary,
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);

        g_free(primary);
        g_free(name);
        g_free(filename);

        success = FALSE;
    }

    return success;
}

static void
display_chooser_cb (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    GError *error = NULL;
    if (!esdesktop_file_manager_call_display_chooser_dialog_finish(ESDESKTOP_FILE_MANAGER(source_object), res, &error))
        esdesktop_file_utils_async_handle_error(error, user_data);
}

void
esdesktop_file_utils_display_chooser_dialog(GFile *file,
                                            gboolean open,
                                            GdkScreen *screen,
                                            GtkWindow *parent)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(G_IS_FILE(file));
    g_return_if_fail(GDK_IS_SCREEN(screen) || GTK_IS_WINDOW(parent));

    if(!screen)
        screen = gtk_widget_get_screen(GTK_WIDGET(parent));

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        gchar *uri = g_file_get_uri(file);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        esdesktop_file_utils_set_window_cursor(parent, GDK_WATCH);

        esdesktop_file_manager_call_display_chooser_dialog(fileman_proxy,
                                                           uri, open,
                                                           display_name,
                                                           startup_id,
                                                           NULL,
                                                           display_chooser_cb,
                                                           parent);

        esdesktop_file_utils_set_window_cursor(parent, GDK_LEFT_PTR);

        g_free(startup_id);
        g_free(uri);
        g_free(display_name);
    } else {
        expidus_message_dialog(parent,
                            _("Launch Error"), "dialog-error",
                            _("The application chooser could not be opened"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

void
esdesktop_file_utils_transfer_file(GdkDragAction action,
                                   GFile *source_file,
                                   GFile *target_file,
                                   GdkScreen *screen)
{
    EsdesktopFileManager *fileman_proxy;

    g_return_if_fail(G_IS_FILE(source_file));
    g_return_if_fail(G_IS_FILE(target_file));
    g_return_if_fail(screen == NULL || GDK_IS_SCREEN(screen));

    if(!screen)
        screen = gdk_display_get_default_screen(gdk_display_get_default());

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        GError *error = NULL;
        gchar *source_uris[2] = { g_file_get_uri(source_file), NULL };
        gchar *target_uris[2] = { g_file_get_uri(target_file), NULL };
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        switch(action) {
            case GDK_ACTION_MOVE:
                esdesktop_file_manager_call_move_into_sync(fileman_proxy, "",
                                                           (const gchar **)source_uris,
                                                           (const gchar *)target_uris[0],
                                                           display_name, startup_id,
                                                           NULL, &error);
                break;
            case GDK_ACTION_COPY:
                esdesktop_file_manager_call_copy_to_sync(fileman_proxy, "",
                                                         (const gchar **)source_uris,
                                                         (const gchar **)target_uris,
                                                         display_name, startup_id,
                                                         NULL, &error);
                break;
            case GDK_ACTION_LINK:
                esdesktop_file_manager_call_link_into_sync(fileman_proxy, "",
                                                           (const gchar **)source_uris,
                                                           (const gchar *)target_uris[0],
                                                           display_name, startup_id,
                                                           NULL, &error);
                break;
            default:
                g_warning("Unsupported transfer action");
        }

        if(error) {
            expidus_message_dialog(NULL,
                                _("Transfer Error"), "dialog-error",
                                _("The file transfer could not be performed"),
                                error->message,
                                EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                                NULL);

            g_clear_error(&error);
        }

        g_free(startup_id);
        g_free(display_name);
        g_free(target_uris[0]);
        g_free(source_uris[0]);
    } else {
        expidus_message_dialog(NULL,
                            _("Transfer Error"), "dialog-error",
                            _("The file transfer could not be performed"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);
    }
}

gboolean
esdesktop_file_utils_transfer_files(GdkDragAction action,
                                    GList *source_files,
                                    GList *target_files,
                                    GdkScreen *screen)
{
    EsdesktopFileManager *fileman_proxy;
    gboolean success = TRUE;

    g_return_val_if_fail(source_files != NULL && G_IS_FILE(source_files->data), FALSE);
    g_return_val_if_fail(target_files != NULL && G_IS_FILE(target_files->data), FALSE);
    g_return_val_if_fail(screen == NULL || GDK_IS_SCREEN(screen), FALSE);

    if(!screen)
        screen = gdk_display_get_default_screen(gdk_display_get_default());

    fileman_proxy = esdesktop_file_utils_peek_filemanager_proxy();
    if(fileman_proxy) {
        GError *error = NULL;
        gchar **source_uris = esdesktop_file_utils_file_list_to_uri_array(source_files);
        gchar **target_uris = esdesktop_file_utils_file_list_to_uri_array(target_files);
        gchar *display_name = g_strdup(gdk_display_get_name(gdk_screen_get_display(screen)));
        gchar *startup_id = g_strdup_printf("_TIME%d", gtk_get_current_event_time());

        switch(action) {
            case GDK_ACTION_MOVE:
                esdesktop_file_manager_call_move_into_sync(fileman_proxy, "",
                                                           (const gchar **)source_uris,
                                                           (const gchar *)target_uris[0],
                                                           display_name, startup_id,
                                                           NULL, &error);
                break;
            case GDK_ACTION_COPY:
                esdesktop_file_manager_call_copy_to_sync(fileman_proxy, "",
                                                         (const gchar **)source_uris,
                                                         (const gchar **)target_uris,
                                                         display_name, startup_id,
                                                         NULL, &error);
                break;
            case GDK_ACTION_LINK:
                esdesktop_file_manager_call_link_into_sync(fileman_proxy, "",
                                                          (const gchar **)source_uris,
                                                          (const gchar *)target_uris[0],
                                                          display_name, startup_id,
                                                          NULL, &error);
                break;
            default:
                g_warning("Unsupported transfer action");
                success = FALSE;
                break;
        }

        if(error) {
            expidus_message_dialog(NULL,
                                _("Transfer Error"), "dialog-error",
                                _("The file transfer could not be performed"),
                                error->message,
                                EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                                NULL);

            g_clear_error(&error);

            success = FALSE;
        }

        g_free(startup_id);
        g_free(display_name);
        g_free(target_uris[0]);
        g_free(source_uris[0]);
    } else {
        expidus_message_dialog(NULL,
                            _("Transfer Error"), "dialog-error",
                            _("The file transfer could not be performed"),
                            _("This feature requires a file manager service to "
                              "be present (such as the one supplied by Lunar)."),
                            EXPIDUS_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                            NULL);

        success = FALSE;
    }

    return success;
}

static gint dbus_ref_cnt = 0;
static GDBusConnection *dbus_gconn = NULL;
static EsdesktopTrash *dbus_trash_proxy = NULL;
static EsdesktopFileManager *dbus_filemanager_proxy = NULL;
#ifdef HAVE_LUNARX
static EsdesktopLunar *dbus_lunar_proxy = NULL;
#else
static GDBusProxy *dbus_lunar_proxy = NULL;
#endif
gboolean
esdesktop_file_utils_dbus_init(void)
{
    gboolean ret = TRUE;

    if(dbus_ref_cnt++)
        return TRUE;

    if(!dbus_gconn) {
        dbus_gconn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
    }

    if(dbus_gconn) {
        esdesktop_trash_proxy_new(dbus_gconn,
                                  G_DBUS_PROXY_FLAGS_NONE,
                                  "com.expidus.FileManager",
                                  "/com/expidus/FileManager",
                                  NULL,
                                  esdesktop_file_utils_trash_proxy_new_cb,
                                  NULL);

        esdesktop_file_manager_proxy_new(dbus_gconn,
                                         G_DBUS_PROXY_FLAGS_NONE,
                                         "com.expidus.FileManager",
                                         "/com/expidus/FileManager",
                                         NULL,
                                         esdesktop_file_utils_file_manager_proxy_new_cb,
                                         NULL);

#ifdef HAVE_LUNARX
        esdesktop_lunar_proxy_new(dbus_gconn,
                                   G_DBUS_PROXY_FLAGS_NONE,
                                   "com.expidus.FileManager",
                                   "/com/expidus/FileManager",
                                   NULL,
                                   esdesktop_file_utils_lunar_proxy_new_cb,
                                   NULL);
#else
        dbus_lunar_proxy = NULL;
#endif

    } else {
        ret = FALSE;
        dbus_ref_cnt = 0;
    }

    return ret;
}

static EsdesktopTrash *
esdesktop_file_utils_peek_trash_proxy(void)
{
    return dbus_trash_proxy;
}

static EsdesktopFileManager *
esdesktop_file_utils_peek_filemanager_proxy(void)
{
    return dbus_filemanager_proxy;
}

#ifdef HAVE_LUNARX
static EsdesktopLunar *
esdesktop_file_utils_peek_lunar_proxy(void)
{
    return dbus_lunar_proxy;
}
#else
static gpointer
esdesktop_file_utils_peek_lunar_proxy(void)
{
    return NULL;
}
#endif

static void
esdesktop_file_utils_trash_proxy_new_cb (GObject *source_object,
                                         GAsyncResult *res,
                                         gpointer user_data) {
    dbus_trash_proxy = esdesktop_trash_proxy_new_finish (res, NULL);
}

static void
esdesktop_file_utils_file_manager_proxy_new_cb (GObject *source_object,
                                                GAsyncResult *res,
                                                gpointer user_data) {
    dbus_filemanager_proxy = esdesktop_file_manager_proxy_new_finish (res, NULL);
}

static void
esdesktop_file_utils_lunar_proxy_new_cb (GObject *source_object,
                                          GAsyncResult *res,
                                          gpointer user_data) {
#ifdef HAVE_LUNARX
    dbus_lunar_proxy = esdesktop_lunar_proxy_new_finish (res, NULL);
#endif
}

void
esdesktop_file_utils_dbus_cleanup(void)
{
    if(dbus_ref_cnt == 0 || --dbus_ref_cnt > 0)
        return;

    if(dbus_trash_proxy)
        g_object_unref(G_OBJECT(dbus_trash_proxy));
    if(dbus_filemanager_proxy)
        g_object_unref(G_OBJECT(dbus_filemanager_proxy));
    if(dbus_lunar_proxy)
        g_object_unref(G_OBJECT(dbus_lunar_proxy));
    if(dbus_gconn)
        g_object_unref(G_OBJECT(dbus_gconn));
}



#ifdef HAVE_LUNARX

/* lunar extension interface stuff: LunarxFileInfo implementation */

gchar *
esdesktop_lunarx_file_info_get_name(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFile *file = esdesktop_file_icon_peek_file(icon);

    return file ? g_file_get_basename(file) : NULL;
}

gchar *
esdesktop_lunarx_file_info_get_uri(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFile *file = esdesktop_file_icon_peek_file(icon);

    return file ? g_file_get_uri(file) : NULL;
}

gchar *
esdesktop_lunarx_file_info_get_parent_uri(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFile *file = esdesktop_file_icon_peek_file(icon);
    gchar *uri = NULL;

    if(file) {
        GFile *parent = g_file_get_parent(file);
        if(parent) {
            uri = g_file_get_uri(parent);
            g_object_unref(parent);
        }
    }

    return uri;
}

gchar *
esdesktop_lunarx_file_info_get_uri_scheme_file(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFile *file = esdesktop_file_icon_peek_file(icon);

    return file ? g_file_get_uri_scheme(file) : NULL;
}

gchar *
esdesktop_lunarx_file_info_get_mime_type(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFileInfo *info = esdesktop_file_icon_peek_file_info(icon);

    return info ? g_strdup(g_file_info_get_content_type(info)) : NULL;
}

gboolean
esdesktop_lunarx_file_info_has_mime_type(LunarxFileInfo *file_info,
                                          const gchar *mime_type)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFileInfo *info = esdesktop_file_icon_peek_file_info(icon);
    const gchar *content_type;

    if(!info)
        return FALSE;

    content_type = g_file_info_get_content_type(info);
    return g_content_type_is_a(content_type, mime_type);
}

gboolean
esdesktop_lunarx_file_info_is_directory(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFileInfo *info = esdesktop_file_icon_peek_file_info(icon);

    return (info && g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY);
}

GFileInfo *
esdesktop_lunarx_file_info_get_file_info(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFileInfo *info = esdesktop_file_icon_peek_file_info(icon);
    return info ? g_object_ref (info) : NULL;
}

GFileInfo *
esdesktop_lunarx_file_info_get_filesystem_info(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFileInfo *info = esdesktop_file_icon_peek_filesystem_info(icon);
    return info ? g_object_ref (info) : NULL;
}

GFile *
esdesktop_lunarx_file_info_get_location(LunarxFileInfo *file_info)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(file_info);
    GFile *file = esdesktop_file_icon_peek_file(icon);
    return g_object_ref (file);
}

#endif  /* HAVE_LUNARX */
