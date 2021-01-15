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

#ifndef __ESDESKTOP_WINDOW_ICON_MANAGER_H__
#define __ESDESKTOP_WINDOW_ICON_MANAGER_H__

#include <glib.h>
#include <gdk/gdk.h>

#include "esdesktop-icon-view-manager.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_WINDOW_ICON_MANAGER     (esdesktop_window_icon_manager_get_type())
#define ESDESKTOP_WINDOW_ICON_MANAGER(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_WINDOW_ICON_MANAGER, EsdesktopWindowIconManager))
#define ESDESKTOP_IS_WINDOW_ICON_MANAGER(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_WINDOW_ICON_MANAGER))

typedef struct _EsdesktopWindowIconManager         EsdesktopWindowIconManager;
typedef struct _EsdesktopWindowIconManagerClass    EsdesktopWindowIconManagerClass;
typedef struct _EsdesktopWindowIconManagerPrivate  EsdesktopWindowIconManagerPrivate;

struct _EsdesktopWindowIconManager
{
    GObject parent;

    /*< private >*/
    EsdesktopWindowIconManagerPrivate *priv;
};

struct _EsdesktopWindowIconManagerClass
{
    GObjectClass parent;
};

GType esdesktop_window_icon_manager_get_type(void) G_GNUC_CONST;

EsdesktopIconViewManager *esdesktop_window_icon_manager_new(GdkScreen *gscreen);

G_END_DECLS

#endif  /* __ESDESKTOP_WINDOW_ICON_MANAGER_H__ */
