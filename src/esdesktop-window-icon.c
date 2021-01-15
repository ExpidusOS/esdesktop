/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright (c) 2006 Brian Tarricone, <bjt23@cornell.edu>
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

#include <libwnck/libwnck.h>
#include <libexpidus1ui/libexpidus1ui.h>

#include "esdesktop-common.h"
#include "esdesktop-window-icon.h"

struct _EsdesktopWindowIconPrivate
{
    gint workspace;
    gchar *label;
    WnckWindow *window;
};

static void esdesktop_window_icon_finalize(GObject *obj);

static GdkPixbuf *esdesktop_window_icon_peek_pixbuf(EsdesktopIcon *icon,
                                                   gint width, gint height);
static const gchar *esdesktop_window_icon_peek_label(EsdesktopIcon *icon);
static gchar *esdesktop_window_icon_get_identifier(EsdesktopIcon *icon);

static gboolean esdesktop_window_icon_activated(EsdesktopIcon *icon);
static gboolean esdesktop_window_icon_populate_context_menu(EsdesktopIcon *icon,
                                                            GtkWidget *menu);

static void esdesktop_window_name_changed_cb(WnckWindow *window,
                                             gpointer user_data);
static void esdesktop_window_icon_changed_cb(WnckWindow *window,
                                             gpointer user_data);


G_DEFINE_TYPE_WITH_PRIVATE(EsdesktopWindowIcon, esdesktop_window_icon, ESDESKTOP_TYPE_ICON)


static void
esdesktop_window_icon_class_init(EsdesktopWindowIconClass *klass)
{
    GObjectClass *gobject_class = (GObjectClass *)klass;
    EsdesktopIconClass *icon_class = (EsdesktopIconClass *)klass;

    gobject_class->finalize = esdesktop_window_icon_finalize;

    icon_class->peek_pixbuf = esdesktop_window_icon_peek_pixbuf;
    icon_class->peek_label = esdesktop_window_icon_peek_label;
    icon_class->get_identifier = esdesktop_window_icon_get_identifier;
    icon_class->activated = esdesktop_window_icon_activated;
    icon_class->populate_context_menu = esdesktop_window_icon_populate_context_menu;
}

static void
esdesktop_window_icon_init(EsdesktopWindowIcon *icon)
{
    icon->priv = esdesktop_window_icon_get_instance_private(icon);
}

static void
esdesktop_window_icon_finalize(GObject *obj)
{
    EsdesktopWindowIcon *icon = ESDESKTOP_WINDOW_ICON(obj);
    gchar data_name[256];
    gint16 row, col;

    g_free(icon->priv->label);

    /* save previous position */
    if(esdesktop_icon_get_position(ESDESKTOP_ICON(icon), &row, &col)) {
        g_snprintf(data_name, 256, "--esdesktop-last-row-%d",
                   icon->priv->workspace);
        g_object_set_data(G_OBJECT(icon->priv->window),
                          data_name, GUINT_TO_POINTER(row + 1));
        g_snprintf(data_name, 256, "--esdesktop-last-col-%d",
                   icon->priv->workspace);
        g_object_set_data(G_OBJECT(icon->priv->window),
                          data_name, GUINT_TO_POINTER(col + 1));
    }

    g_signal_handlers_disconnect_by_func(G_OBJECT(icon->priv->window),
                                         G_CALLBACK(esdesktop_window_name_changed_cb),
                                         icon);
    g_signal_handlers_disconnect_by_func(G_OBJECT(icon->priv->window),
                                         G_CALLBACK(esdesktop_window_icon_changed_cb),
                                         icon);

    G_OBJECT_CLASS(esdesktop_window_icon_parent_class)->finalize(obj);
}



static void
esdesktop_window_name_changed_cb(WnckWindow *window,
                                 gpointer user_data)
{
    EsdesktopWindowIcon *icon = user_data;

    g_free(icon->priv->label);
    icon->priv->label = NULL;

    esdesktop_icon_label_changed(ESDESKTOP_ICON(icon));
}

static void
esdesktop_window_icon_changed_cb(WnckWindow *window,
                                 gpointer user_data)
{
    EsdesktopWindowIcon *icon = user_data;

    esdesktop_icon_invalidate_pixbuf(ESDESKTOP_ICON(icon));
    esdesktop_icon_label_changed(ESDESKTOP_ICON(icon));
}

static GdkPixbuf *
esdesktop_window_icon_peek_pixbuf(EsdesktopIcon *icon,
                                 gint width, gint height)
{
    EsdesktopWindowIcon *window_icon = ESDESKTOP_WINDOW_ICON(icon);
    GdkPixbuf *pix = NULL;

    pix = wnck_window_get_icon(window_icon->priv->window);
    if(pix) {
        if(gdk_pixbuf_get_height(pix) != height) {
            pix = gdk_pixbuf_scale_simple(pix, height, height, GDK_INTERP_BILINEAR);
        } else
            g_object_ref(G_OBJECT(pix));
    }

    return pix;
}

static const gchar *
esdesktop_window_icon_peek_label(EsdesktopIcon *icon)
{
    EsdesktopWindowIcon *window_icon = ESDESKTOP_WINDOW_ICON(icon);

    if(!window_icon->priv->label)
        window_icon->priv->label = g_strdup(wnck_window_get_name(window_icon->priv->window));

    return window_icon->priv->label;
}

static gchar *
esdesktop_window_icon_get_identifier(EsdesktopIcon *icon)
{
    return g_strdup(esdesktop_window_icon_peek_label(icon));
}

static gboolean
esdesktop_window_icon_activated(EsdesktopIcon *icon)
{
    EsdesktopWindowIcon *window_icon = ESDESKTOP_WINDOW_ICON(icon);

    wnck_window_activate(window_icon->priv->window,
                         gtk_get_current_event_time());

    return TRUE;
}

static gboolean
esdesktop_window_icon_populate_context_menu(EsdesktopIcon *icon,
                                            GtkWidget *menu)
{
    EsdesktopWindowIcon *window_icon = ESDESKTOP_WINDOW_ICON(icon);
    GtkWidget *amenu = wnck_action_menu_new(window_icon->priv->window);
    GtkWidget *mi, *img;

    img = gtk_image_new_from_icon_name("", GTK_ICON_SIZE_MENU);
    mi = esdesktop_menu_create_menu_item_with_mnemonic(_("_Window Actions"), img);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM(mi), amenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
    gtk_widget_show_all(mi);

    return TRUE;
}



EsdesktopWindowIcon *
esdesktop_window_icon_new(WnckWindow *window,
                          gint workspace)
{
    EsdesktopWindowIcon *icon = g_object_new(ESDESKTOP_TYPE_WINDOW_ICON, NULL);
    gchar data_name[256];
    gint row, col;

    icon->priv->window = window;
    icon->priv->workspace = workspace;

    /* check for availability of old position (if any) */
    g_snprintf(data_name, 256, "--esdesktop-last-row-%d", workspace);
    row = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(window),
                                             data_name));
    g_snprintf(data_name, 256, "--esdesktop-last-col-%d", workspace);
    col = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(window),
                                             data_name));
    if(row > 0 && col > 0)
        esdesktop_icon_set_position(ESDESKTOP_ICON(icon), row - 1, col - 1);

    g_signal_connect(G_OBJECT(window), "name-changed",
                     G_CALLBACK(esdesktop_window_name_changed_cb),
                     icon);
    g_signal_connect(G_OBJECT(window), "icon-changed",
                     G_CALLBACK(esdesktop_window_icon_changed_cb),
                     icon);

    return icon;
}

gint
esdesktop_window_icon_get_workspace(EsdesktopWindowIcon *window_icon)
{
    g_return_val_if_fail(ESDESKTOP_IS_WINDOW_ICON(window_icon), -1);
    return window_icon->priv->workspace;
}
