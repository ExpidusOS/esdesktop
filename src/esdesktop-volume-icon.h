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

#ifndef __ESDESKTOP_VOLUME_ICON_H__
#define __ESDESKTOP_VOLUME_ICON_H__

#include <gio/gio.h>

#include "esdesktop-file-icon.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_VOLUME_ICON     (esdesktop_volume_icon_get_type())
#define ESDESKTOP_VOLUME_ICON(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_VOLUME_ICON, EsdesktopVolumeIcon))
#define ESDESKTOP_IS_VOLUME_ICON(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_VOLUME_ICON))

typedef struct _EsdesktopVolumeIcon         EsdesktopVolumeIcon;
typedef struct _EsdesktopVolumeIconClass    EsdesktopVolumeIconClass;
typedef struct _EsdesktopVolumeIconPrivate  EsdesktopVolumeIconPrivate;

struct _EsdesktopVolumeIcon
{
    EsdesktopFileIcon parent;

    /*< private >*/
    EsdesktopVolumeIconPrivate *priv;
};

struct _EsdesktopVolumeIconClass
{
    EsdesktopFileIconClass parent;
};

GType esdesktop_volume_icon_get_type(void) G_GNUC_CONST;

EsdesktopVolumeIcon *esdesktop_volume_icon_new(GVolume *volume,
                                               GdkScreen *screen);

GVolume *esdesktop_volume_icon_peek_volume(EsdesktopVolumeIcon *icon);


G_END_DECLS

#endif /* __ESDESKTOP_VOLUME_ICON_H__ */
