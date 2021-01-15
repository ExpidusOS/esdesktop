/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright(c) 2006 Brian Tarricone, <bjt23@cornell.edu>
 *  Copyright(c) 2006 Benedikt Meurer, <benny@expidus.org>
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

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include <endo/endo.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include <gio/gio.h>

#include <libexpidus1ui/libexpidus1ui.h>

#ifdef HAVE_LUNARX
#include <lunarx/lunarx.h>
#endif

#include "esdesktop-file-utils.h"
#include "esdesktop-common.h"
#include "esdesktop-regular-file-icon.h"

#define EMBLEM_UNREADABLE "emblem-unreadable"
#define EMBLEM_READONLY   "emblem-readonly"
#define EMBLEM_SYMLINK    "emblem-symbolic-link"

struct _EsdesktopRegularFileIconPrivate
{
    gchar *display_name;
    gchar *tooltip;
    guint pix_opacity;
    GFileInfo *file_info;
    GFileInfo *filesystem_info;
    GFile *file;
    GFile *thumbnail_file;
    GFileMonitor *monitor;
    GdkScreen *gscreen;
    EsdesktopFileIconManager *fmanager;
    gboolean show_thumbnails;
};

static void esdesktop_regular_file_icon_finalize(GObject *obj);

static void esdesktop_regular_file_icon_set_thumbnail_file(EsdesktopIcon *icon, GFile *file);
static void esdesktop_regular_file_icon_delete_thumbnail_file(EsdesktopIcon *icon);

static GdkPixbuf *esdesktop_regular_file_icon_peek_pixbuf(EsdesktopIcon *icon,
                                                          gint width, gint height);
static const gchar *esdesktop_regular_file_icon_peek_label(EsdesktopIcon *icon);
static gchar *esdesktop_regular_file_icon_get_identifier(EsdesktopIcon *icon);
static GdkPixbuf *esdesktop_regular_file_icon_peek_tooltip_pixbuf(EsdesktopIcon *icon,
                                                                  gint width, gint height);
static const gchar *esdesktop_regular_file_icon_peek_tooltip(EsdesktopIcon *icon);
static GdkDragAction esdesktop_regular_file_icon_get_allowed_drag_actions(EsdesktopIcon *icon);
static GdkDragAction esdesktop_regular_file_icon_get_allowed_drop_actions(EsdesktopIcon *icon,
                                                                          GdkDragAction *suggested_action);
static gboolean esdesktop_regular_file_icon_do_drop_dest(EsdesktopIcon *icon,
                                                         EsdesktopIcon *src_icon,
                                                         GdkDragAction action);

static GFileInfo *esdesktop_regular_file_icon_peek_file_info(EsdesktopFileIcon *icon);
static GFileInfo *esdesktop_regular_file_icon_peek_filesystem_info(EsdesktopFileIcon *icon);
static GFile *esdesktop_regular_file_icon_peek_file(EsdesktopFileIcon *icon);
static void esdesktop_regular_file_icon_update_file_info(EsdesktopFileIcon *icon,
                                                         GFileInfo *info);
static gboolean esdesktop_regular_file_can_write_parent(EsdesktopFileIcon *icon);

#ifdef HAVE_LUNARX
static void esdesktop_regular_file_icon_tfi_init(LunarxFileInfoIface *iface);

G_DEFINE_TYPE_EXTENDED(EsdesktopRegularFileIcon, esdesktop_regular_file_icon,
                       ESDESKTOP_TYPE_FILE_ICON, 0,
                       G_IMPLEMENT_INTERFACE(LUNARX_TYPE_FILE_INFO,
                                             esdesktop_regular_file_icon_tfi_init)
                       G_ADD_PRIVATE(EsdesktopRegularFileIcon)
                       )
#else
G_DEFINE_TYPE_WITH_PRIVATE(EsdesktopRegularFileIcon, esdesktop_regular_file_icon,
                           ESDESKTOP_TYPE_FILE_ICON)
#endif



static void
esdesktop_regular_file_icon_class_init(EsdesktopRegularFileIconClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    EsdesktopIconClass *icon_class = (EsdesktopIconClass *)klass;
    EsdesktopFileIconClass *file_icon_class = (EsdesktopFileIconClass *)klass;

    gobject_class->finalize = esdesktop_regular_file_icon_finalize;

    icon_class->peek_pixbuf = esdesktop_regular_file_icon_peek_pixbuf;
    icon_class->peek_label = esdesktop_regular_file_icon_peek_label;
    icon_class->get_identifier = esdesktop_regular_file_icon_get_identifier;
    icon_class->peek_tooltip_pixbuf = esdesktop_regular_file_icon_peek_tooltip_pixbuf;
    icon_class->peek_tooltip = esdesktop_regular_file_icon_peek_tooltip;
    icon_class->get_allowed_drag_actions = esdesktop_regular_file_icon_get_allowed_drag_actions;
    icon_class->get_allowed_drop_actions = esdesktop_regular_file_icon_get_allowed_drop_actions;
    icon_class->do_drop_dest = esdesktop_regular_file_icon_do_drop_dest;
    icon_class->set_thumbnail_file = esdesktop_regular_file_icon_set_thumbnail_file;
    icon_class->delete_thumbnail_file = esdesktop_regular_file_icon_delete_thumbnail_file;

    file_icon_class->peek_file_info = esdesktop_regular_file_icon_peek_file_info;
    file_icon_class->peek_filesystem_info = esdesktop_regular_file_icon_peek_filesystem_info;
    file_icon_class->peek_file = esdesktop_regular_file_icon_peek_file;
    file_icon_class->update_file_info = esdesktop_regular_file_icon_update_file_info;
    file_icon_class->can_rename_file = esdesktop_regular_file_can_write_parent;
    file_icon_class->can_delete_file = esdesktop_regular_file_can_write_parent;
}

static void
esdesktop_regular_file_icon_init(EsdesktopRegularFileIcon *icon)
{
    icon->priv = esdesktop_regular_file_icon_get_instance_private(icon);
    icon->priv->pix_opacity = 100;
    icon->priv->display_name = NULL;
}

static void
esdesktop_regular_file_icon_finalize(GObject *obj)
{
    EsdesktopRegularFileIcon *icon = ESDESKTOP_REGULAR_FILE_ICON(obj);
    GtkIconTheme *itheme = gtk_icon_theme_get_for_screen(icon->priv->gscreen);

    g_signal_handlers_disconnect_by_func(G_OBJECT(itheme),
                                         G_CALLBACK(esdesktop_icon_invalidate_pixbuf),
                                         icon);

    if(icon->priv->file_info)
        g_object_unref(icon->priv->file_info);

    if(icon->priv->filesystem_info)
        g_object_unref(icon->priv->filesystem_info);

    g_object_unref(icon->priv->file);

    g_free(icon->priv->display_name);

    if(icon->priv->tooltip)
        g_free(icon->priv->tooltip);

    if(icon->priv->thumbnail_file)
        g_object_unref(icon->priv->thumbnail_file);

    if(icon->priv->monitor)
        g_object_unref(icon->priv->monitor);

    G_OBJECT_CLASS(esdesktop_regular_file_icon_parent_class)->finalize(obj);
}

#ifdef HAVE_LUNARX
static void
esdesktop_regular_file_icon_tfi_init(LunarxFileInfoIface *iface)
{
    iface->get_name = esdesktop_lunarx_file_info_get_name;
    iface->get_uri = esdesktop_lunarx_file_info_get_uri;
    iface->get_parent_uri = esdesktop_lunarx_file_info_get_parent_uri;
    iface->get_uri_scheme = esdesktop_lunarx_file_info_get_uri_scheme_file;
    iface->get_mime_type = esdesktop_lunarx_file_info_get_mime_type;
    iface->has_mime_type = esdesktop_lunarx_file_info_has_mime_type;
    iface->is_directory = esdesktop_lunarx_file_info_is_directory;
    iface->get_file_info = esdesktop_lunarx_file_info_get_file_info;
    iface->get_filesystem_info = esdesktop_lunarx_file_info_get_filesystem_info;
    iface->get_location = esdesktop_lunarx_file_info_get_location;
}
#endif

static void
esdesktop_regular_file_icon_delete_thumbnail_file(EsdesktopIcon *icon)
{
    EsdesktopRegularFileIcon *file_icon;

    if(!ESDESKTOP_IS_REGULAR_FILE_ICON(icon))
        return;

    file_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);

    if(file_icon->priv->thumbnail_file) {
        g_object_unref(file_icon->priv->thumbnail_file);
        file_icon->priv->thumbnail_file = NULL;
    }

    esdesktop_file_icon_invalidate_icon(ESDESKTOP_FILE_ICON(icon));

    esdesktop_icon_invalidate_pixbuf(icon);
    esdesktop_icon_pixbuf_changed(icon);
}

static void
esdesktop_regular_file_icon_set_thumbnail_file(EsdesktopIcon *icon, GFile *file)
{
    EsdesktopRegularFileIcon *file_icon;

    if(!ESDESKTOP_IS_REGULAR_FILE_ICON(icon))
        return;

    file_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);

    if(file_icon->priv->thumbnail_file)
        g_object_unref(file_icon->priv->thumbnail_file);

    file_icon->priv->thumbnail_file = file;

    esdesktop_file_icon_invalidate_icon(ESDESKTOP_FILE_ICON(icon));

    esdesktop_icon_invalidate_pixbuf(icon);
    esdesktop_icon_pixbuf_changed(icon);
}


static void
cb_show_thumbnails_notify(GObject *gobject,
                          GParamSpec *pspec,
                          gpointer user_data)
{
    EsdesktopRegularFileIcon *regular_file_icon;
    gboolean show_thumbnails = FALSE;

    TRACE("entering");

    if(!user_data || !ESDESKTOP_IS_REGULAR_FILE_ICON(user_data))
        return;

    regular_file_icon = ESDESKTOP_REGULAR_FILE_ICON(user_data);

    g_object_get(regular_file_icon->priv->fmanager, "show-thumbnails", &show_thumbnails, NULL);

    if(regular_file_icon->priv->show_thumbnails != show_thumbnails) {
        XF_DEBUG("show-thumbnails changed! now: %s", show_thumbnails ? "TRUE" : "FALSE");
        regular_file_icon->priv->show_thumbnails = show_thumbnails;
        esdesktop_file_icon_invalidate_icon(ESDESKTOP_FILE_ICON(regular_file_icon));
        esdesktop_icon_invalidate_pixbuf(ESDESKTOP_ICON(regular_file_icon));
        esdesktop_icon_pixbuf_changed(ESDESKTOP_ICON(regular_file_icon));
    }
}


/* builds a folder/file path and then tests if that file is a valid image.
 * returns the file location if it does, NULL if it doesn't */
static gchar *
esdesktop_check_file_is_valid(const gchar *folder, const gchar *file)
{
    gchar *path = g_strconcat(folder, "/", file, NULL);

    if(gdk_pixbuf_get_file_info(path, NULL, NULL) == NULL) {
        g_free(path);
        path = NULL;
    }

    return path;
}

static gchar *
esdesktop_load_icon_location_from_folder(EsdesktopFileIcon *icon)
{
    gchar *icon_file = g_file_get_path(esdesktop_file_icon_peek_file(icon));
    gchar *path;

    g_return_val_if_fail(icon_file, NULL);

    /* So much for standards */
    path = esdesktop_check_file_is_valid(icon_file, "Folder.jpg");
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "folder.jpg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "Folder.JPG");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "folder.JPG");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "folder.jpeg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "folder.JPEG");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "Folder.JPEG");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "Folder.jpeg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "Cover.jpg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "cover.jpg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "Cover.jpeg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "cover.jpeg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "albumart.jpg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "albumart.jpeg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "fanart.jpg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "Fanart.jpg");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "fanart.JPG");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "Fanart.JPG");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "FANART.JPG");
    }
    if(path == NULL) {
        path = esdesktop_check_file_is_valid(icon_file, "FANART.jpg");
    }

    g_free(icon_file);

    /* the file *should* already be a thumbnail */
    return path;
}

static GIcon *
esdesktop_load_icon_from_desktop_file(EsdesktopRegularFileIcon *regular_icon)
{
    GKeyFile *key_file;
    gchar *contents, *icon_name;
    gsize length;
    GIcon *gicon = NULL;
    GtkIconTheme *itheme = gtk_icon_theme_get_default();
    gboolean is_pixmaps = FALSE;

    /* try to load the file into memory */
    if(!g_file_load_contents(regular_icon->priv->file, NULL, &contents, &length, NULL, NULL))
        return NULL;

    /* allocate a new key file */
    key_file = g_key_file_new();

    /* try to parse the key file from the contents of the file */
    if(!g_key_file_load_from_data(key_file, contents, length, 0, NULL)) {
        g_free(contents);
        return NULL;
    }

    /* try to determine the custom icon name */
    icon_name = g_key_file_get_string(key_file,
                                      G_KEY_FILE_DESKTOP_GROUP,
                                      G_KEY_FILE_DESKTOP_KEY_ICON,
                                      NULL);

    /* No icon name in the desktop file */
    if(icon_name == NULL) {
        /* free key file and in-memory data */
        g_key_file_free(key_file);
        g_free(contents);
        return NULL;
    }

    /* icon_name is an absolute path, create it as a file icon */
    if(g_file_test(icon_name, G_FILE_TEST_IS_REGULAR)) {
        gicon = g_file_icon_new(g_file_new_for_path(icon_name));
    }

    /* check if the icon theme includes the icon name as-is */
    if(gicon == NULL) {
        if(gtk_icon_theme_has_icon(itheme, icon_name)) {
            /* load it */
            gicon = g_themed_icon_new(icon_name);
        }
    }

    /* drop any suffix (e.g. '.png') from the icon name and try to load that */
    if(gicon == NULL) {
        gchar *tmp_name = NULL;
        gchar *p = strrchr(icon_name, '.');

        if(p != NULL)
            tmp_name = g_strndup(icon_name, p - icon_name);
        else
            tmp_name = g_strdup(icon_name);

        if(tmp_name)
            gicon = g_themed_icon_new(tmp_name);

        /* check if icon file exists */
        if(gicon != NULL) {
            GtkIconInfo *icon_info = gtk_icon_theme_lookup_by_gicon(itheme,
                                                                    gicon,
                                                                    -1,
                                                                    ITHEME_FLAGS);
            if(icon_info) {
                /* check if icon is located in pixmaps folder */
                const gchar *filename = gtk_icon_info_get_filename(icon_info);
                is_pixmaps = g_strrstr(filename, "pixmaps") ? TRUE : FALSE;

                g_object_unref(icon_info);
            } else {
                /* icon not found*/
                g_object_unref(gicon);
                gicon = NULL;
            }
        }

        g_free(tmp_name);
    }

    /* maybe it points to a file in the pixmaps folder */
    if(gicon == NULL || is_pixmaps) {
        gchar *filename = g_build_filename("pixmaps", icon_name, NULL);
        gchar *tmp_name = expidus_resource_lookup(EXPIDUS_RESOURCE_DATA, filename);


        if(tmp_name) {
            if(gicon != NULL)
                g_object_unref(gicon);
            gicon = g_file_icon_new(g_file_new_for_path(tmp_name));
        }

        g_free(filename);
        g_free(tmp_name);
    }

    /* free key file and in-memory data */
    g_key_file_free(key_file);
    g_free(contents);
    g_free(icon_name);

    return gicon;
}

static GIcon *
esdesktop_regular_file_icon_load_icon(EsdesktopIcon *icon)
{
    EsdesktopRegularFileIcon *regular_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);
    EsdesktopFileIcon *file_icon = ESDESKTOP_FILE_ICON(icon);
    GIcon *gicon = NULL;

    TRACE("entering");

    /* Try to load the icon referenced in the .desktop file */
    if(esdesktop_file_utils_is_desktop_file(regular_icon->priv->file_info)) {
        gicon = esdesktop_load_icon_from_desktop_file(regular_icon);

    } else if(g_file_info_get_file_type(regular_icon->priv->file_info) == G_FILE_TYPE_DIRECTORY) {
        /* Try to load a thumbnail from the standard folder image locations */
        gchar *thumbnail_file = NULL;

        if(regular_icon->priv->show_thumbnails)
            thumbnail_file = esdesktop_load_icon_location_from_folder(file_icon);

        if(thumbnail_file) {
            /* If there's a folder thumbnail, use it */
            regular_icon->priv->thumbnail_file = g_file_new_for_path(thumbnail_file);
            gicon = g_file_icon_new(regular_icon->priv->thumbnail_file);
            g_free(thumbnail_file);
        }

    } else {
        /* If we have a thumbnail then they are enabled, use it. */
        if(regular_icon->priv->thumbnail_file) {
            gchar *file = g_file_get_path(regular_icon->priv->file);
            gchar *mimetype = esdesktop_get_file_mimetype(file);

            /* Don't use thumbnails for svg, use the file itself */
            if(g_strcmp0(mimetype, "image/svg+xml") == 0)
                gicon = g_file_icon_new(regular_icon->priv->file);
            else
                gicon = g_file_icon_new(regular_icon->priv->thumbnail_file);

            g_free(mimetype);
            g_free(file);
        }
    }

    /* If we still don't have an icon, use the default */
    if(!G_IS_ICON(gicon)) {
        gicon = g_file_info_get_icon(regular_icon->priv->file_info);
        if(G_IS_ICON(gicon))
            g_object_ref(gicon);
    }

    g_object_set(file_icon, "gicon", gicon, NULL);

    /* Add any user set emblems */
    gicon = esdesktop_file_icon_add_emblems(file_icon);

    /* load the unreadable emblem if necessary */
    if(!g_file_info_get_attribute_boolean(regular_icon->priv->file_info,
                                          G_FILE_ATTRIBUTE_ACCESS_CAN_READ))
    {
        GIcon *themed_icon = g_themed_icon_new(EMBLEM_UNREADABLE);
        GEmblem *emblem = g_emblem_new(themed_icon);

        g_emblemed_icon_add_emblem(G_EMBLEMED_ICON(gicon), emblem);

        g_object_unref(emblem);
        g_object_unref(themed_icon);
    }
    /* load the read only emblem if necessary */
    else if(!g_file_info_get_attribute_boolean(regular_icon->priv->file_info,
                                               G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
    {
        GIcon *themed_icon = g_themed_icon_new(EMBLEM_READONLY);
        GEmblem *emblem = g_emblem_new(themed_icon);

        g_emblemed_icon_add_emblem(G_EMBLEMED_ICON(gicon), emblem);

        g_object_unref(emblem);
        g_object_unref(themed_icon);
    }

    /* load the symlink emblem if necessary */
    if(g_file_info_get_attribute_boolean(regular_icon->priv->file_info,
                                         G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK))
    {
        GIcon *themed_icon = g_themed_icon_new(EMBLEM_SYMLINK);
        GEmblem *emblem = g_emblem_new(themed_icon);

        g_emblemed_icon_add_emblem(G_EMBLEMED_ICON(gicon), emblem);

        g_object_unref(emblem);
        g_object_unref(themed_icon);
    }

    return gicon;
}

static GdkPixbuf *
esdesktop_regular_file_icon_peek_pixbuf(EsdesktopIcon *icon,
                                        gint width, gint height)
{
    EsdesktopRegularFileIcon *regular_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);
    GIcon *gicon = NULL;
    GdkPixbuf *pix = NULL;

    if(!esdesktop_file_icon_has_gicon(ESDESKTOP_FILE_ICON(icon)))
        gicon = esdesktop_regular_file_icon_load_icon(icon);
    else
        g_object_get(ESDESKTOP_FILE_ICON(icon), "gicon", &gicon, NULL);

    pix = esdesktop_file_utils_get_icon(gicon, width, height,
                                        regular_icon->priv->pix_opacity);

    return pix;
}

static GdkPixbuf *
esdesktop_regular_file_icon_peek_tooltip_pixbuf(EsdesktopIcon *icon,
                                                gint width, gint height)
{
    GIcon *gicon = NULL;
    GdkPixbuf *tooltip_pix = NULL;

    if(!esdesktop_file_icon_has_gicon(ESDESKTOP_FILE_ICON(icon)))
        gicon = esdesktop_regular_file_icon_load_icon(icon);
    else
        g_object_get(ESDESKTOP_FILE_ICON(icon), "gicon", &gicon, NULL);

    tooltip_pix = esdesktop_file_utils_get_icon(gicon, width, height, 100);

    return tooltip_pix;
}

static const gchar *
esdesktop_regular_file_icon_peek_label(EsdesktopIcon *icon)
{
    EsdesktopRegularFileIcon *regular_file_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);

    g_return_val_if_fail(ESDESKTOP_IS_REGULAR_FILE_ICON(icon), NULL);

    return regular_file_icon->priv->display_name;
}

static gchar *
esdesktop_regular_file_icon_get_identifier(EsdesktopIcon *icon)
{
    EsdesktopFileIcon *file_icon = ESDESKTOP_FILE_ICON(icon);

    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), NULL);

    if(esdesktop_file_icon_peek_file(file_icon) == NULL)
        return NULL;

    return g_file_get_path(esdesktop_file_icon_peek_file(file_icon));
}

static GdkDragAction
esdesktop_regular_file_icon_get_allowed_drag_actions(EsdesktopIcon *icon)
{
    GFileInfo *info = esdesktop_file_icon_peek_file_info(ESDESKTOP_FILE_ICON(icon));
    GFile *file = esdesktop_file_icon_peek_file(ESDESKTOP_FILE_ICON(icon));
    GdkDragAction actions = GDK_ACTION_LINK;  /* we can always link */

    if(!info)
        return 0;

    if(g_file_info_get_attribute_boolean(info,
                                         G_FILE_ATTRIBUTE_ACCESS_CAN_READ))
    {
        GFileInfo *parent_info;
        GFile *parent_file;

        actions |= GDK_ACTION_COPY;

        /* we can only move if the parent is writable */
        parent_file = g_file_get_parent(file);
        parent_info = g_file_query_info(parent_file,
                                        ESDESKTOP_FILE_INFO_NAMESPACE,
                                        G_FILE_QUERY_INFO_NONE,
                                        NULL, NULL);
        if(parent_info) {
            if(g_file_info_get_attribute_boolean(parent_info,
                                                 G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE))
            {
                actions |= GDK_ACTION_MOVE;
            }
            g_object_unref(parent_info);
        }
        g_object_unref(parent_file);
    }

    return actions;
}

static GdkDragAction
esdesktop_regular_file_icon_get_allowed_drop_actions(EsdesktopIcon *icon,
                                                     GdkDragAction *suggested_action)
{
    GFileInfo *info = esdesktop_file_icon_peek_file_info(ESDESKTOP_FILE_ICON(icon));

    if(!info) {
        if(suggested_action)
            *suggested_action = 0;
        return 0;
    }

    /* if it's executable we can 'copy'.  if it's a folder we can do anything
     * if it's writable. */
    if(g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY) {
        if(g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE)) {
            if(suggested_action)
                *suggested_action = GDK_ACTION_MOVE;
            return GDK_ACTION_MOVE | GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_ASK;
        }
    } else {
        if(esdesktop_file_utils_file_is_executable(info)) {
            if(suggested_action)
                *suggested_action = GDK_ACTION_COPY;
            return GDK_ACTION_COPY;
        }
    }

    if(suggested_action)
        *suggested_action = 0;

    return 0;
}

gboolean
esdesktop_regular_file_icon_do_drop_dest(EsdesktopIcon *icon,
                                         EsdesktopIcon *src_icon,
                                         GdkDragAction action)
{
    EsdesktopRegularFileIcon *regular_file_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);
    EsdesktopFileIcon *src_file_icon = ESDESKTOP_FILE_ICON(src_icon);
    GFileInfo *src_info;
    GFile *src_file;
    gboolean result = FALSE;

    TRACE("entering");

    g_return_val_if_fail(regular_file_icon && src_file_icon, FALSE);
    g_return_val_if_fail(esdesktop_regular_file_icon_get_allowed_drop_actions(icon, NULL) != 0,
                         FALSE);

    src_file = esdesktop_file_icon_peek_file(src_file_icon);

    src_info = esdesktop_file_icon_peek_file_info(src_file_icon);
    if(!src_info)
        return FALSE;

    if(g_file_info_get_file_type(regular_file_icon->priv->file_info) != G_FILE_TYPE_DIRECTORY
       && esdesktop_file_utils_file_is_executable(regular_file_icon->priv->file_info))
    {
        GList files;

        files.data = src_file;
        files.prev = files.next = NULL;

        esdesktop_file_utils_execute(NULL, regular_file_icon->priv->file, &files,
                                     regular_file_icon->priv->gscreen, NULL);

        result = TRUE;
    } else {
        GFile *parent, *dest_file = NULL;
        gchar *name;

        parent = g_file_get_parent(src_file);
        if(!parent)
            return FALSE;
        g_object_unref(parent);

        name = g_file_get_basename(src_file);
        if(!name)
            return FALSE;

        switch(action) {
            case GDK_ACTION_MOVE:
                dest_file = g_object_ref(regular_file_icon->priv->file);
                break;
            case GDK_ACTION_COPY:
                dest_file = g_file_get_child(regular_file_icon->priv->file, name);
                break;
            case GDK_ACTION_LINK:
                dest_file = g_object_ref(regular_file_icon->priv->file);
                break;
            default:
                g_warning("Unsupported drag action: %d", action);
        }

        if(dest_file) {
            esdesktop_file_utils_transfer_file(action, src_file, dest_file,
                                               regular_file_icon->priv->gscreen);

            g_object_unref(dest_file);

            result = TRUE;
        }

        g_free(name);
    }

    return result;
}

static const gchar *
esdesktop_regular_file_icon_peek_tooltip(EsdesktopIcon *icon)
{
    EsdesktopRegularFileIcon *regular_file_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);

    if(!regular_file_icon->priv->tooltip) {
        GFileInfo *info = esdesktop_file_icon_peek_file_info(ESDESKTOP_FILE_ICON(icon));
        const gchar *content_type, *comment = NULL;
        gchar *description, *size_string, *time_string;
        guint64 size, mtime;
        gboolean is_desktop_file = FALSE;

        if(!info)
            return NULL;

        if(g_content_type_equals(g_file_info_get_content_type(info),
                                 "application/x-desktop"))
        {
            is_desktop_file = TRUE;
        }
        else
        {
          gchar *uri = g_file_get_uri(regular_file_icon->priv->file);
          if(g_str_has_suffix(uri, ".desktop"))
              is_desktop_file = TRUE;
          g_free(uri);
        }

        content_type = g_file_info_get_content_type(info);
        description = g_content_type_get_description(content_type);

        size = g_file_info_get_attribute_uint64(info,
                                                G_FILE_ATTRIBUTE_STANDARD_SIZE);

        size_string = g_format_size(size);

        mtime = g_file_info_get_attribute_uint64(info,
                                                 G_FILE_ATTRIBUTE_TIME_MODIFIED);
        time_string = esdesktop_file_utils_format_time_for_display(mtime);

        regular_file_icon->priv->tooltip =
            g_strdup_printf(_("Name: %s\nType: %s\nSize: %s\nLast modified: %s"),
                            regular_file_icon->priv->display_name,
                            description, size_string, time_string);

        /* Extract the Comment entry from the .desktop file */
        if(is_desktop_file)
        {
            gchar *path = g_file_get_path(regular_file_icon->priv->file);
            ExpidusRc *rcfile = expidus_rc_simple_open(path, TRUE);
            g_free(path);

            if(rcfile) {
                expidus_rc_set_group(rcfile, "Desktop Entry");
                comment = expidus_rc_read_entry(rcfile, "Comment", NULL);
            }
            /* Prepend the comment to the tooltip */
            if(comment != NULL && *comment != '\0') {
                gchar *tooltip = regular_file_icon->priv->tooltip;
                regular_file_icon->priv->tooltip = g_strdup_printf("%s\n%s",
                                                                   comment,
                                                                   tooltip);
                g_free(tooltip);
            }

            expidus_rc_close(rcfile);
        }

        g_free(time_string);
        g_free(size_string);
        g_free(description);
    }

    return regular_file_icon->priv->tooltip;
}

static gboolean
esdesktop_regular_file_can_write_parent(EsdesktopFileIcon *icon)
{
    EsdesktopRegularFileIcon *file_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);
    GFile *parent_file;
    GFileInfo *parent_info;
    gboolean writable;

    g_return_val_if_fail(file_icon, FALSE);

    parent_file = g_file_get_parent(file_icon->priv->file);
    if(!parent_file)
        return FALSE;

    parent_info = g_file_query_info(parent_file,
                                    ESDESKTOP_FILE_INFO_NAMESPACE,
                                    G_FILE_QUERY_INFO_NONE,
                                    NULL, NULL);
    if(!parent_info) {
        g_object_unref(parent_file);
        return FALSE;
    }

    writable = g_file_info_get_attribute_boolean(parent_info,
                                                 G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
    g_object_unref(parent_info);
    g_object_unref(parent_file);

    return writable;

}

static GFileInfo *
esdesktop_regular_file_icon_peek_file_info(EsdesktopFileIcon *icon)
{
    g_return_val_if_fail(ESDESKTOP_IS_REGULAR_FILE_ICON(icon), NULL);
    return ESDESKTOP_REGULAR_FILE_ICON(icon)->priv->file_info;
}

static GFileInfo *
esdesktop_regular_file_icon_peek_filesystem_info(EsdesktopFileIcon *icon)
{
    g_return_val_if_fail(ESDESKTOP_IS_REGULAR_FILE_ICON(icon), NULL);
    return ESDESKTOP_REGULAR_FILE_ICON(icon)->priv->filesystem_info;
}

static GFile *
esdesktop_regular_file_icon_peek_file(EsdesktopFileIcon *icon)
{
    g_return_val_if_fail(ESDESKTOP_IS_REGULAR_FILE_ICON(icon), NULL);
    return ESDESKTOP_REGULAR_FILE_ICON(icon)->priv->file;
}

static void
esdesktop_regular_file_icon_update_file_info(EsdesktopFileIcon *icon,
                                             GFileInfo *info)
{
    EsdesktopRegularFileIcon *regular_file_icon = ESDESKTOP_REGULAR_FILE_ICON(icon);
    const gchar *old_display_name;
    gchar *new_display_name;

    g_return_if_fail(ESDESKTOP_IS_REGULAR_FILE_ICON(icon));
    g_return_if_fail(G_IS_FILE_INFO(info));

    /* release the old file info */
    if(regular_file_icon->priv->file_info) {
        g_object_unref(regular_file_icon->priv->file_info);
        regular_file_icon->priv->file_info = NULL;
    }

    regular_file_icon->priv->file_info = g_object_ref(info);

    if(regular_file_icon->priv->filesystem_info)
        g_object_unref(regular_file_icon->priv->filesystem_info);

    regular_file_icon->priv->filesystem_info = g_file_query_filesystem_info(regular_file_icon->priv->file,
                                                                            ESDESKTOP_FILESYSTEM_INFO_NAMESPACE,
                                                                            NULL, NULL);

    /* get both, old and new display name */
    old_display_name = regular_file_icon->priv->display_name;
    new_display_name = esdesktop_file_utils_get_display_name(regular_file_icon->priv->file,
                                                             regular_file_icon->priv->file_info);

    /* check whether the display name has changed with the info update */
    if(g_strcmp0 (old_display_name, new_display_name) != 0) {
        /* replace the display name */
        g_free (regular_file_icon->priv->display_name);
        regular_file_icon->priv->display_name = new_display_name;

        /* notify listeners of the label change */
        esdesktop_icon_label_changed(ESDESKTOP_ICON(icon));
    } else {
        /* no change, release the new display name */
        g_free (new_display_name);
    }

    /* invalidate the tooltip */
    g_free(regular_file_icon->priv->tooltip);
    regular_file_icon->priv->tooltip = NULL;

    /* not really easy to check if this changed or not, so just invalidate it */
    esdesktop_file_icon_invalidate_icon(ESDESKTOP_FILE_ICON(icon));
    esdesktop_icon_invalidate_pixbuf(ESDESKTOP_ICON(icon));
    esdesktop_icon_pixbuf_changed(ESDESKTOP_ICON(icon));
}

static void
cb_folder_contents_changed(GFileMonitor     *monitor,
                           GFile            *file,
                           GFile            *other_file,
                           GFileMonitorEvent event,
                           gpointer          user_data)
{
    EsdesktopRegularFileIcon *regular_file_icon;
    gchar *thumbnail_file = NULL;

    if(!user_data || !ESDESKTOP_IS_REGULAR_FILE_ICON(user_data))
        return;

    regular_file_icon = ESDESKTOP_REGULAR_FILE_ICON(user_data);

    /* not showing thumbnails */
    if(!regular_file_icon->priv->show_thumbnails)
        return;

    /* Already has a thumbnail */
    if(regular_file_icon->priv->thumbnail_file != NULL)
        return;

    switch(event) {
        case G_FILE_MONITOR_EVENT_CREATED:
                thumbnail_file = esdesktop_load_icon_location_from_folder(ESDESKTOP_FILE_ICON(regular_file_icon));
                if(thumbnail_file) {
                    GFile *thumbnail = g_file_new_for_path(thumbnail_file);
                    /* found a thumbnail file, apply it */
                    esdesktop_regular_file_icon_set_thumbnail_file(ESDESKTOP_ICON(regular_file_icon),
                                                                   thumbnail);
                    g_free(thumbnail_file);
                }
            break;
        default:
            break;
    }
}

/* public API */

EsdesktopRegularFileIcon *
esdesktop_regular_file_icon_new(GFile *file,
                                GFileInfo *file_info,
                                GdkScreen *screen,
                                EsdesktopFileIconManager *fmanager)
{
    EsdesktopRegularFileIcon *regular_file_icon;

    g_return_val_if_fail(G_IS_FILE(file), NULL);
    g_return_val_if_fail(G_IS_FILE_INFO(file_info), NULL);
    g_return_val_if_fail(GDK_IS_SCREEN(screen), NULL);

    regular_file_icon = g_object_new(ESDESKTOP_TYPE_REGULAR_FILE_ICON, NULL);

    regular_file_icon->priv->file = g_object_ref(file);
    regular_file_icon->priv->file_info = g_object_ref(file_info);

    /* set the display name */
    regular_file_icon->priv->display_name = esdesktop_file_utils_get_display_name(file,
                                                                                  file_info);

    /* query file system information from GIO */
    regular_file_icon->priv->filesystem_info = g_file_query_filesystem_info(regular_file_icon->priv->file,
                                                                            ESDESKTOP_FILESYSTEM_INFO_NAMESPACE,
                                                                            NULL, NULL);

    regular_file_icon->priv->gscreen = screen;

    regular_file_icon->priv->fmanager = fmanager;

    g_signal_connect_swapped(G_OBJECT(gtk_icon_theme_get_for_screen(screen)),
                             "changed",
                             G_CALLBACK(esdesktop_icon_invalidate_pixbuf),
                             regular_file_icon);

    if(g_file_info_get_file_type(regular_file_icon->priv->file_info) == G_FILE_TYPE_DIRECTORY) {
        regular_file_icon->priv->monitor = g_file_monitor(regular_file_icon->priv->file,
                                                          G_FILE_MONITOR_NONE,
                                                          NULL,
                                                          NULL);

        g_signal_connect(regular_file_icon->priv->monitor, "changed",
                         G_CALLBACK(cb_folder_contents_changed),
                         regular_file_icon);

        g_object_get(regular_file_icon->priv->fmanager,
                     "show-thumbnails", &regular_file_icon->priv->show_thumbnails,
                     NULL);

        /* Keep an eye on the show-thumbnails property for folder thumbnails */
        g_signal_connect(G_OBJECT(fmanager), "notify::show-thumbnails",
                         G_CALLBACK(cb_show_thumbnails_notify), regular_file_icon);
    }
    return regular_file_icon;
}

void
esdesktop_regular_file_icon_set_pixbuf_opacity(EsdesktopRegularFileIcon *icon,
                                       guint opacity)
{
    g_return_if_fail(ESDESKTOP_IS_REGULAR_FILE_ICON(icon) && opacity <= 100);

    if(opacity == icon->priv->pix_opacity)
        return;

    icon->priv->pix_opacity = opacity;

    esdesktop_icon_invalidate_pixbuf(ESDESKTOP_ICON(icon));
    esdesktop_icon_pixbuf_changed(ESDESKTOP_ICON(icon));
}
