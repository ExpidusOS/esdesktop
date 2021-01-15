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

#ifndef __ESDESKTOP_FILE_ICON_MANAGER_H__
#define __ESDESKTOP_FILE_ICON_MANAGER_H__

#include <glib.h>
#include <esconf/esconf.h>

#include "esdesktop-special-file-icon.h"
#include "esdesktop-icon-view-manager.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_FILE_ICON_MANAGER     (esdesktop_file_icon_manager_get_type())
#define ESDESKTOP_FILE_ICON_MANAGER(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_FILE_ICON_MANAGER, EsdesktopFileIconManager))
#define ESDESKTOP_IS_FILE_ICON_MANAGER(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_FILE_ICON_MANAGER))

typedef struct _EsdesktopFileIconManager         EsdesktopFileIconManager;
typedef struct _EsdesktopFileIconManagerClass    EsdesktopFileIconManagerClass;
typedef struct _EsdesktopFileIconManagerPrivate  EsdesktopFileIconManagerPrivate;

struct _EsdesktopFileIconManager
{
    GObject parent;

    /*< private >*/
    EsdesktopFileIconManagerPrivate *priv;
};

struct _EsdesktopFileIconManagerClass
{
    GObjectClass parent;

    /*< signals >*/
    void (*hidden_state_changed)(EsdesktopFileIconManager *fmanager);
};

GType esdesktop_file_icon_manager_get_type(void) G_GNUC_CONST;

EsdesktopIconViewManager *esdesktop_file_icon_manager_new(GFile *folder,
                                                          EsconfChannel *channel);

void esdesktop_file_icon_save(gpointer user_data);

gboolean esdesktop_file_icon_manager_get_cached_icon_position(
                                                    EsdesktopFileIconManager *fmanager,
                                                    const gchar *name,
                                                    const gchar *identifier,
                                                    gint16 *row,
                                                    gint16 *col);

void esdesktop_dnd_menu (EsdesktopIconViewManager *manager,
                         EsdesktopIcon *drop_icon,
                         GdkDragContext *context,
                         GdkDragAction *action,
                         gint16 row,
                         gint16 col,
                         guint time_);

G_END_DECLS

#endif  /* __ESDESKTOP_FILE_ICON_MANAGER_H__ */
