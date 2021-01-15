/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright (c) 2006      Brian Tarricone, <bjt23@cornell.edu>
 *  Copyright (c) 2010-2011 Jannis Pohlmann, <jannis@expidus.org>
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

#include <gio/gio.h>

#include <libexpidus1ui/libexpidus1ui.h>

#include "esdesktop-file-utils.h"
#include "esdesktop-file-icon.h"

struct _EsdesktopFileIconPrivate
{
    GIcon *gicon;
};

static void esdesktop_file_icon_finalize(GObject *obj);

static gboolean esdesktop_file_icon_activated(EsdesktopIcon *icon);

static void esdesktop_file_icon_set_property(GObject *object,
                                             guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec);
static void esdesktop_file_icon_get_property(GObject *object,
                                             guint property_id,
                                             GValue *value,
                                             GParamSpec *pspec);

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(EsdesktopFileIcon, esdesktop_file_icon,
                                    ESDESKTOP_TYPE_ICON)

enum
{
    PROP_0,
    PROP_GICON,
};

static void
esdesktop_file_icon_class_init(EsdesktopFileIconClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    EsdesktopIconClass *icon_class = (EsdesktopIconClass *)klass;

    gobject_class->finalize = esdesktop_file_icon_finalize;
    gobject_class->set_property = esdesktop_file_icon_set_property;
    gobject_class->get_property = esdesktop_file_icon_get_property;

    icon_class->activated = esdesktop_file_icon_activated;

    g_object_class_install_property(gobject_class,
                                    PROP_GICON,
                                    g_param_spec_pointer("gicon",
                                                         "gicon",
                                                         "gicon",
                                                         G_PARAM_READWRITE));
}

static void
esdesktop_file_icon_init(EsdesktopFileIcon *icon)
{
    icon->priv = esdesktop_file_icon_get_instance_private(icon);
}

static void
esdesktop_file_icon_finalize(GObject *obj)
{
    EsdesktopFileIcon *icon = ESDESKTOP_FILE_ICON(obj);

    esdesktop_file_icon_invalidate_icon(icon);

    G_OBJECT_CLASS(esdesktop_file_icon_parent_class)->finalize(obj);
}

static void
esdesktop_file_icon_set_property(GObject *object,
                                 guint property_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
    EsdesktopFileIcon *file_icon = ESDESKTOP_FILE_ICON(object);

    switch(property_id) {
        case PROP_GICON:
            esdesktop_file_icon_invalidate_icon(file_icon);
            file_icon->priv->gicon = g_value_get_pointer(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void
esdesktop_file_icon_get_property(GObject *object,
                                 guint property_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
    EsdesktopFileIcon *file_icon = ESDESKTOP_FILE_ICON(object);

    switch(property_id) {
        case PROP_GICON:
            g_value_set_pointer(value, file_icon->priv->gicon);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static gboolean
esdesktop_file_icon_activated(EsdesktopIcon *icon)
{
    EsdesktopFileIcon *file_icon = ESDESKTOP_FILE_ICON(icon);
    GFileInfo *info = esdesktop_file_icon_peek_file_info(file_icon);
    GFile *file = esdesktop_file_icon_peek_file(file_icon);
    GtkWidget *icon_view, *toplevel;
    GdkScreen *gscreen;

    TRACE("entering");

    if(!info)
        return FALSE;

    icon_view = esdesktop_icon_peek_icon_view(icon);
    toplevel = gtk_widget_get_toplevel(icon_view);
    gscreen = gtk_widget_get_screen(icon_view);

    if(g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY)
        esdesktop_file_utils_open_folder(file, gscreen, GTK_WINDOW(toplevel));
    else if(esdesktop_file_utils_file_is_executable(info))
        esdesktop_file_utils_execute(NULL, file, NULL, gscreen, GTK_WINDOW(toplevel));
    else
        esdesktop_file_utils_launch(file, gscreen, GTK_WINDOW(toplevel));

    return TRUE;
}


GFileInfo *
esdesktop_file_icon_peek_file_info(EsdesktopFileIcon *icon)
{
    EsdesktopFileIconClass *klass;

    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), NULL);

    klass = ESDESKTOP_FILE_ICON_GET_CLASS(icon);

    if(klass->peek_file_info)
       return klass->peek_file_info(icon);
    else
        return NULL;
}

GFileInfo *
esdesktop_file_icon_peek_filesystem_info(EsdesktopFileIcon *icon)
{
    EsdesktopFileIconClass *klass;

    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), NULL);

    klass = ESDESKTOP_FILE_ICON_GET_CLASS(icon);

    if(klass->peek_filesystem_info)
       return klass->peek_filesystem_info(icon);
    else
        return NULL;
}

GFile *
esdesktop_file_icon_peek_file(EsdesktopFileIcon *icon)
{
    EsdesktopFileIconClass *klass;

    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), NULL);

    klass = ESDESKTOP_FILE_ICON_GET_CLASS(icon);

    if(klass->peek_file)
       return klass->peek_file(icon);
    else
        return NULL;
}

void
esdesktop_file_icon_update_file_info(EsdesktopFileIcon *icon,
                                     GFileInfo *info)
{
    EsdesktopFileIconClass *klass;

    g_return_if_fail(ESDESKTOP_IS_FILE_ICON(icon));

    klass = ESDESKTOP_FILE_ICON_GET_CLASS(icon);

    if(klass->update_file_info)
       klass->update_file_info(icon, info);
}

gboolean
esdesktop_file_icon_can_rename_file(EsdesktopFileIcon *icon)
{
    EsdesktopFileIconClass *klass;

    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), FALSE);

    klass = ESDESKTOP_FILE_ICON_GET_CLASS(icon);

    if(klass->can_rename_file)
       return klass->can_rename_file(icon);
    else
        return FALSE;
}

gboolean
esdesktop_file_icon_can_delete_file(EsdesktopFileIcon *icon)
{
    EsdesktopFileIconClass *klass;

    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), FALSE);

    klass = ESDESKTOP_FILE_ICON_GET_CLASS(icon);

    if(klass->can_delete_file)
       return klass->can_delete_file(icon);
    else
        return FALSE;
}

GIcon *
esdesktop_file_icon_add_emblems(EsdesktopFileIcon *icon)
{
    GIcon *emblemed_icon = NULL;
    gchar **emblem_names;

    TRACE("entering");

    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), NULL);

    if(G_IS_EMBLEMED_ICON(icon->priv->gicon))
        emblemed_icon = icon->priv->gicon;
    else if(G_IS_ICON(icon->priv->gicon))
        emblemed_icon = g_emblemed_icon_new(icon->priv->gicon, NULL);
    else
        return NULL;

    if(!G_IS_FILE_INFO(esdesktop_file_icon_peek_file_info(icon)))
        return icon->priv->gicon = emblemed_icon;

    /* Get the list of emblems */
    emblem_names = g_file_info_get_attribute_stringv(esdesktop_file_icon_peek_file_info(icon),
                                                     "metadata::emblems");

    if(emblem_names != NULL) {
        /* for each item in the list create an icon, pack it into an emblem,
         * and attach it to our icon. */
        for (; *emblem_names != NULL; ++emblem_names) {
            GIcon *themed_icon = g_themed_icon_new(*emblem_names);
            GEmblem *emblem = g_emblem_new(themed_icon);

            g_emblemed_icon_add_emblem(G_EMBLEMED_ICON(emblemed_icon), emblem);

            g_object_unref(emblem);
            g_object_unref(themed_icon);
        }
    }

    /* Clear out the old icon and set the new one */
    esdesktop_file_icon_invalidate_icon(icon);
    return icon->priv->gicon = emblemed_icon;
}

void
esdesktop_file_icon_invalidate_icon(EsdesktopFileIcon *icon)
{
    g_return_if_fail(ESDESKTOP_IS_FILE_ICON(icon));

    if(G_IS_ICON(icon->priv->gicon)) {
        g_object_unref(icon->priv->gicon);
        icon->priv->gicon = NULL;
    }
}

gboolean
esdesktop_file_icon_has_gicon(EsdesktopFileIcon *icon)
{
    g_return_val_if_fail(ESDESKTOP_IS_FILE_ICON(icon), FALSE);

    return G_IS_ICON(icon->priv->gicon);
}
