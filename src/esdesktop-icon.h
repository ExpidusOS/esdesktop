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

#ifndef __ESDESKTOP_ICON_H__
#define __ESDESKTOP_ICON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_ICON            (esdesktop_icon_get_type())
#define ESDESKTOP_ICON(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_ICON, EsdesktopIcon))
#define ESDESKTOP_ICON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), ESDESKTOP_TYPE_ICON, EsdesktopIconClass))
#define ESDESKTOP_IS_ICON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_ICON))
#define ESDESKTOP_ICON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), ESDESKTOP_TYPE_ICON, EsdesktopIconClass))

typedef struct _EsdesktopIcon        EsdesktopIcon;
typedef struct _EsdesktopIconClass   EsdesktopIconClass;
typedef struct _EsdesktopIconPrivate EsdesktopIconPrivate;

struct _EsdesktopIcon
{
    GObject parent;

    /*< private >*/
    EsdesktopIconPrivate *priv;
};

struct _EsdesktopIconClass
{
    GObjectClass parent;

    /*< signals >*/
    void (*pixbuf_changed)(EsdesktopIcon *icon);
    void (*label_changed)(EsdesktopIcon *icon);

    void (*position_changed)(EsdesktopIcon *icon);

    void (*selected)(EsdesktopIcon *icon);
    /* XfdektopIcon::activated has weird semantics: you should NEVER connect to
     * this signal normally: always use g_signal_connect_after(), as the default
     * signal handler may do some special setup for the icon.  this is lame;
     * you should be able to use normal g_signal_connect(), but signal handlers
     * with return values are (for some unknown reason) not allowed to be
     * G_SIGNAL_RUN_FIRST.  go figure. */
    gboolean (*activated)(EsdesktopIcon *icon);

    /*< virtual functions >*/
    GdkPixbuf *(*peek_pixbuf)(EsdesktopIcon *icon, gint width, gint height);
    const gchar *(*peek_label)(EsdesktopIcon *icon);

    GdkDragAction (*get_allowed_drag_actions)(EsdesktopIcon *icon);

    GdkDragAction (*get_allowed_drop_actions)(EsdesktopIcon *icon, GdkDragAction *suggested_action);
    gboolean (*do_drop_dest)(EsdesktopIcon *icon, EsdesktopIcon *src_icon, GdkDragAction action);

    GdkPixbuf *(*peek_tooltip_pixbuf)(EsdesktopIcon *icon, gint width, gint height);
    const gchar *(*peek_tooltip)(EsdesktopIcon *icon);

    gchar *(*get_identifier)(EsdesktopIcon *icon);

    void (*set_thumbnail_file)(EsdesktopIcon *icon, GFile *file);
    void (*delete_thumbnail_file)(EsdesktopIcon *icon);

    gboolean (*populate_context_menu)(EsdesktopIcon *icon,
                                      GtkWidget *menu);
};

GType esdesktop_icon_get_type(void) G_GNUC_CONST;

/* esdesktop virtual function accessors */

GdkPixbuf *esdesktop_icon_peek_pixbuf(EsdesktopIcon *icon,
                                     gint width,
                                     gint height);
const gchar *esdesktop_icon_peek_label(EsdesktopIcon *icon);
GdkPixbuf *esdesktop_icon_peek_tooltip_pixbuf(EsdesktopIcon *icon,
                                              gint width,
                                              gint height);
const gchar *esdesktop_icon_peek_tooltip(EsdesktopIcon *icon);

/* returns a unique identifier for the icon, free when done using it */
gchar *esdesktop_icon_get_identifier(EsdesktopIcon *icon);

void esdesktop_icon_set_position(EsdesktopIcon *icon,
                                 gint16 row,
                                 gint16 col);
gboolean esdesktop_icon_get_position(EsdesktopIcon *icon,
                                     gint16 *row,
                                     gint16 *col);

GdkDragAction esdesktop_icon_get_allowed_drag_actions(EsdesktopIcon *icon);

GdkDragAction esdesktop_icon_get_allowed_drop_actions(EsdesktopIcon *icon,
                                                      GdkDragAction *suggested_action);
gboolean esdesktop_icon_do_drop_dest(EsdesktopIcon *icon,
                                     EsdesktopIcon *src_icon,
                                     GdkDragAction action);

gboolean esdesktop_icon_populate_context_menu(EsdesktopIcon *icon,
                                              GtkWidget *menu);

GtkWidget *esdesktop_icon_peek_icon_view(EsdesktopIcon *icon);

void esdesktop_icon_set_thumbnail_file(EsdesktopIcon *icon, GFile *file);
void esdesktop_icon_delete_thumbnail(EsdesktopIcon *icon);

void esdesktop_icon_invalidate_regular_pixbuf(EsdesktopIcon *icon);
void esdesktop_icon_invalidate_tooltip_pixbuf(EsdesktopIcon *icon);
void esdesktop_icon_invalidate_pixbuf(EsdesktopIcon *icon);

/*< signal triggers >*/

void esdesktop_icon_pixbuf_changed(EsdesktopIcon *icon);
void esdesktop_icon_label_changed(EsdesktopIcon *icon);
void esdesktop_icon_position_changed(EsdesktopIcon *icon);

void esdesktop_icon_selected(EsdesktopIcon *icon);
gboolean esdesktop_icon_activated(EsdesktopIcon *icon);

/*< private-ish; only for use by EsdesktopIconView >*/
void esdesktop_icon_set_extents(EsdesktopIcon *icon,
                                const GdkRectangle *pixbuf_extents,
                                const GdkRectangle *text_extents,
                                const GdkRectangle *total_extents);
gboolean esdesktop_icon_get_extents(EsdesktopIcon *icon,
                                    GdkRectangle *pixbuf_extents,
                                    GdkRectangle *text_extents,
                                    GdkRectangle *total_extents);

G_END_DECLS

#endif  /* __ESDESKTOP_ICON_H__ */
