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

#ifndef __ESDESKTOP_ICON_VIEW_MANAGER_H__
#define __ESDESKTOP_ICON_VIEW_MANAGER_H__

#include <glib-object.h>
#include <gtk/gtk.h>

#include "esdesktop-icon.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_ICON_VIEW_MANAGER            (esdesktop_icon_view_manager_get_type())
#define ESDESKTOP_ICON_VIEW_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_ICON_VIEW_MANAGER, EsdesktopIconViewManager))
#define ESDESKTOP_IS_ICON_VIEW_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_ICON_VIEW_MANAGER))
#define ESDESKTOP_ICON_VIEW_MANAGER_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE((obj), ESDESKTOP_TYPE_ICON_VIEW_MANAGER, EsdesktopIconViewManagerIface))

typedef struct _EsdesktopIconViewManagerIface EsdesktopIconViewManagerIface;
typedef struct _EsdesktopIconViewManager EsdesktopIconViewManager;  /* dummy */

/* fwd decl - meh */
struct _EsdesktopIconView;

struct _EsdesktopIconViewManagerIface
{
    GTypeInterface g_iface;

    /*< virtual functions >*/
    gboolean (*manager_init)(EsdesktopIconViewManager *manager,
                             struct _EsdesktopIconView *icon_view);
    void (*manager_fini)(EsdesktopIconViewManager *manager);

    gboolean (*drag_drop)(EsdesktopIconViewManager *manager,
                          EsdesktopIcon *drop_icon,
                          GdkDragContext *context,
                          gint16 row,
                          gint16 col,
                          guint time_);
    void (*drag_data_received)(EsdesktopIconViewManager *manager,
                               EsdesktopIcon *drop_icon,
                               GdkDragContext *context,
                               gint16 row,
                               gint16 col,
                               GtkSelectionData *data,
                               guint info,
                               guint time_);
    void (*drag_data_get)(EsdesktopIconViewManager *manager,
                          GList *drag_icons,
                          GdkDragContext *context,
                          GtkSelectionData *data,
                          guint info,
                          guint time_);
    GdkDragAction (*propose_drop_action)(EsdesktopIconViewManager *manager,
                                         EsdesktopIcon *drop_icon,
                                         GdkDragAction action,
                                         GdkDragContext *context,
                                         GtkSelectionData *data,
                                         guint info);
};

GType esdesktop_icon_view_manager_get_type(void) G_GNUC_CONST;

/* virtual function accessors */

gboolean esdesktop_icon_view_manager_init(EsdesktopIconViewManager *manager,
                                          struct _EsdesktopIconView *icon_view);
void esdesktop_icon_view_manager_fini(EsdesktopIconViewManager *manager);

gboolean esdesktop_icon_view_manager_drag_drop(EsdesktopIconViewManager *manager,
                                               EsdesktopIcon *drop_icon,
                                               GdkDragContext *context,
                                               gint16 row,
                                               gint16 col,
                                               guint time_);
void esdesktop_icon_view_manager_drag_data_received(EsdesktopIconViewManager *manager,
                                                    EsdesktopIcon *drop_icon,
                                                    GdkDragContext *context,
                                                    gint16 row,
                                                    gint16 col,
                                                    GtkSelectionData *data,
                                                    guint info,
                                                    guint time_);
void esdesktop_icon_view_manager_drag_data_get(EsdesktopIconViewManager *manager,
                                               GList *drag_icons,
                                               GdkDragContext *context,
                                               GtkSelectionData *data,
                                               guint info,
                                               guint time_);
GdkDragAction esdesktop_icon_view_manager_propose_drop_action(EsdesktopIconViewManager *manager,
                                                              EsdesktopIcon *drop_icon,
                                                              GdkDragAction action,
                                                              GdkDragContext *context,
                                                              GtkSelectionData *data,
                                                              guint info);



G_END_DECLS

#endif  /* __ESDESKTOP_ICON_VIEW_MANAGER_H__ */
