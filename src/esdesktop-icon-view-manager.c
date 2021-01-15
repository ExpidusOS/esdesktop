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

#include <glib-object.h>

#include "esdesktop-icon-view-manager.h"
#include "esdesktop-icon-view.h"

GType
esdesktop_icon_view_manager_get_type(void)
{
    static GType manager_type = 0;

    if(!manager_type) {
        static const GTypeInfo manager_info = {
            sizeof(EsdesktopIconViewManagerIface),
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            0,
            NULL,
        };

        manager_type = g_type_register_static(G_TYPE_INTERFACE,
                                            "EsdesktopIconViewManager",
                                            &manager_info, 0);
        g_type_interface_add_prerequisite(manager_type, G_TYPE_OBJECT);
    }

    return manager_type;
}

gboolean
esdesktop_icon_view_manager_init(EsdesktopIconViewManager *manager,
                                 EsdesktopIconView *icon_view)
{
    EsdesktopIconViewManagerIface *iface;

    g_return_val_if_fail(ESDESKTOP_IS_ICON_VIEW_MANAGER(manager)
                         && ESDESKTOP_IS_ICON_VIEW(icon_view), FALSE);

    iface = ESDESKTOP_ICON_VIEW_MANAGER_GET_IFACE(manager);
    g_return_val_if_fail(iface->manager_init, FALSE);

    return iface->manager_init(manager, icon_view);
}

void
esdesktop_icon_view_manager_fini(EsdesktopIconViewManager *manager)
{
    EsdesktopIconViewManagerIface *iface;

    g_return_if_fail(ESDESKTOP_IS_ICON_VIEW_MANAGER(manager));

    iface = ESDESKTOP_ICON_VIEW_MANAGER_GET_IFACE(manager);
    g_return_if_fail(iface->manager_fini);

    iface->manager_fini(manager);
}

gboolean
esdesktop_icon_view_manager_drag_drop(EsdesktopIconViewManager *manager,
                                      EsdesktopIcon *drop_icon,
                                      GdkDragContext *context,
                                      gint16 row,
                                      gint16 col,
                                      guint time_)
{
    EsdesktopIconViewManagerIface *iface;

    g_return_val_if_fail(ESDESKTOP_IS_ICON_VIEW_MANAGER(manager), FALSE);

    iface = ESDESKTOP_ICON_VIEW_MANAGER_GET_IFACE(manager);
    g_return_val_if_fail(iface->drag_drop, FALSE);

    return iface->drag_drop(manager, drop_icon, context, row, col, time_);
}

void
esdesktop_icon_view_manager_drag_data_received(EsdesktopIconViewManager *manager,
                                               EsdesktopIcon *drop_icon,
                                               GdkDragContext *context,
                                               gint16 row,
                                               gint16 col,
                                               GtkSelectionData *data,
                                               guint info,
                                               guint time_)
{
    EsdesktopIconViewManagerIface *iface;

    g_return_if_fail(ESDESKTOP_IS_ICON_VIEW_MANAGER(manager));

    iface = ESDESKTOP_ICON_VIEW_MANAGER_GET_IFACE(manager);
    g_return_if_fail(iface->drag_data_received);

    iface->drag_data_received(manager, drop_icon, context, row, col, data, info,
                              time_);
}

void
esdesktop_icon_view_manager_drag_data_get(EsdesktopIconViewManager *manager,
                                          GList *drag_icons,
                                          GdkDragContext *context,
                                          GtkSelectionData *data,
                                          guint info,
                                          guint time_)
{
    EsdesktopIconViewManagerIface *iface;

    g_return_if_fail(ESDESKTOP_IS_ICON_VIEW_MANAGER(manager));

    iface = ESDESKTOP_ICON_VIEW_MANAGER_GET_IFACE(manager);
    g_return_if_fail(iface->drag_data_get);

    iface->drag_data_get(manager, drag_icons, context, data, info, time_);
}

GdkDragAction
esdesktop_icon_view_manager_propose_drop_action(EsdesktopIconViewManager *manager,
                                                EsdesktopIcon *drop_icon,
                                                GdkDragAction action,
                                                GdkDragContext *context,
                                                GtkSelectionData *data,
                                                guint info)
{
    EsdesktopIconViewManagerIface *iface;

    g_return_val_if_fail(ESDESKTOP_IS_ICON_VIEW_MANAGER(manager), action);

    iface = ESDESKTOP_ICON_VIEW_MANAGER_GET_IFACE(manager);
    g_return_val_if_fail(iface->propose_drop_action, action);

    return iface->propose_drop_action(manager, drop_icon, action, context, data,
                                      info);
}
