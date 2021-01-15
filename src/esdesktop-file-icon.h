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

#ifndef __ESDESKTOP_FILE_ICON_H__
#define __ESDESKTOP_FILE_ICON_H__

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "esdesktop-icon.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_FILE_ICON            (esdesktop_file_icon_get_type())
#define ESDESKTOP_FILE_ICON(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_FILE_ICON, EsdesktopFileIcon))
#define ESDESKTOP_IS_FILE_ICON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_FILE_ICON))
#define ESDESKTOP_FILE_ICON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), ESDESKTOP_TYPE_FILE_ICON, EsdesktopFileIconClass))

typedef struct _EsdesktopFileIcon        EsdesktopFileIcon;
typedef struct _EsdesktopFileIconClass   EsdesktopFileIconClass;
typedef struct _EsdesktopFileIconPrivate EsdesktopFileIconPrivate;

struct _EsdesktopFileIcon
{
    EsdesktopIcon parent;

    /*< private >*/
    EsdesktopFileIconPrivate *priv;
};

struct _EsdesktopFileIconClass
{
    EsdesktopIconClass parent;

    /*< virtual functions >*/
    GFileInfo *(*peek_file_info)(EsdesktopFileIcon *icon);
    GFileInfo *(*peek_filesystem_info)(EsdesktopFileIcon *icon);
    GFile *(*peek_file)(EsdesktopFileIcon *icon);
    void (*update_file_info)(EsdesktopFileIcon *icon, GFileInfo *info);

    gboolean (*can_rename_file)(EsdesktopFileIcon *icon);
    gboolean (*can_delete_file)(EsdesktopFileIcon *icon);
};

GType esdesktop_file_icon_get_type(void) G_GNUC_CONST;

GFileInfo *esdesktop_file_icon_peek_file_info(EsdesktopFileIcon *icon);
GFileInfo *esdesktop_file_icon_peek_filesystem_info(EsdesktopFileIcon *icon);
GFile *esdesktop_file_icon_peek_file(EsdesktopFileIcon *icon);
void esdesktop_file_icon_update_file_info(EsdesktopFileIcon *icon,
                                          GFileInfo *info);

gboolean esdesktop_file_icon_can_rename_file(EsdesktopFileIcon *icon);

gboolean esdesktop_file_icon_can_delete_file(EsdesktopFileIcon *icon);

GIcon *esdesktop_file_icon_add_emblems(EsdesktopFileIcon *icon);

void esdesktop_file_icon_invalidate_icon(EsdesktopFileIcon *icon);

gboolean esdesktop_file_icon_has_gicon(EsdesktopFileIcon *icon);

G_END_DECLS

#endif  /* __ESDESKTOP_FILE_ICON_H__ */
