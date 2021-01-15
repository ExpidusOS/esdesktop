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

#ifndef __ESDESKTOP_SPECIAL_FILE_ICON_H__
#define __ESDESKTOP_SPECIAL_FILE_ICON_H__

#include <glib-object.h>

#include "esdesktop-file-icon.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_SPECIAL_FILE_ICON     (esdesktop_special_file_icon_get_type())
#define ESDESKTOP_SPECIAL_FILE_ICON(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_SPECIAL_FILE_ICON, EsdesktopSpecialFileIcon))
#define ESDESKTOP_IS_SPECIAL_FILE_ICON(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_SPECIAL_FILE_ICON))

typedef struct _EsdesktopSpecialFileIcon         EsdesktopSpecialFileIcon;
typedef struct _EsdesktopSpecialFileIconClass    EsdesktopSpecialFileIconClass;
typedef struct _EsdesktopSpecialFileIconPrivate  EsdesktopSpecialFileIconPrivate;

struct _EsdesktopSpecialFileIcon
{
    EsdesktopFileIcon parent;

    /*< private >*/
    EsdesktopSpecialFileIconPrivate *priv;
};

struct _EsdesktopSpecialFileIconClass
{
    EsdesktopFileIconClass parent;
};

typedef enum
{
    ESDESKTOP_SPECIAL_FILE_ICON_HOME = 0,
    ESDESKTOP_SPECIAL_FILE_ICON_FILESYSTEM,
    ESDESKTOP_SPECIAL_FILE_ICON_TRASH,
} EsdesktopSpecialFileIconType;

GType esdesktop_special_file_icon_get_type(void) G_GNUC_CONST;

EsdesktopSpecialFileIcon *esdesktop_special_file_icon_new(EsdesktopSpecialFileIconType type,
                                                          GdkScreen *screen);

EsdesktopSpecialFileIconType esdesktop_special_file_icon_get_icon_type(EsdesktopSpecialFileIcon *icon);

G_END_DECLS

#endif /* __ESDESKTOP_SPECIAL_FILE_ICON_H__ */
