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

#ifndef __ESDESKTOP_WINDOW_ICON_H__
#define __ESDESKTOP_WINDOW_ICON_H__

#include <glib-object.h>
#include <libexpidus1ui/libexpidus1ui.h>

#include "esdesktop-icon.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_WINDOW_ICON     (esdesktop_window_icon_get_type())
#define ESDESKTOP_WINDOW_ICON(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_WINDOW_ICON, EsdesktopWindowIcon))
#define ESDESKTOP_IS_WINDOW_ICON(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_WINDOW_ICON))

typedef struct _EsdesktopWindowIcon         EsdesktopWindowIcon;
typedef struct _EsdesktopWindowIconClass    EsdesktopWindowIconClass;
typedef struct _EsdesktopWindowIconPrivate  EsdesktopWindowIconPrivate;

struct _EsdesktopWindowIcon
{
    EsdesktopIcon parent;

    /*< private >*/
    EsdesktopWindowIconPrivate *priv;
};

struct _EsdesktopWindowIconClass
{
    EsdesktopIconClass parent;
};

GType esdesktop_window_icon_get_type(void) G_GNUC_CONST;

EsdesktopWindowIcon *esdesktop_window_icon_new(WnckWindow *window,
                                               gint workspace);

gint esdesktop_window_icon_get_workspace(EsdesktopWindowIcon *window_icon);

G_END_DECLS

#endif /* __ESDESKTOP_WINDOW_ICON_H__ */
