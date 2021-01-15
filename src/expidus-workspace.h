/*
 *  xfworkspace - expidus1's desktop manager
 *
 *  Copyright (c) 2004-2007 Brian Tarricone, <bjt23@cornell.edu>
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

#ifndef _EXPIDUS_WORKSPACE_H_
#define _EXPIDUS_WORKSPACE_H_

#include <gtk/gtk.h>
#include <esconf/esconf.h>

#include "expidus-backdrop.h"

G_BEGIN_DECLS

#define EXPIDUS_TYPE_WORKSPACE              (expidus_workspace_get_type())
#define EXPIDUS_WORKSPACE(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), EXPIDUS_TYPE_WORKSPACE, ExpidusWorkspace))
#define EXPIDUS_WORKSPACE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), EXPIDUS_TYPE_WORKSPACE, ExpidusWorkspaceClass))
#define EXPIDUS_IS_WORKSPACE(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), EXPIDUS_TYPE_WORKSPACE))
#define EXPIDUS_IS_WORKSPACE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), EXPIDUS_TYPE_WORKSPACE))
#define EXPIDUS_WORKSPACE_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), EXPIDUS_TYPE_WORKSPACE, ExpidusWorkspaceClass))

typedef struct _ExpidusWorkspace ExpidusWorkspace;
typedef struct _ExpidusWorkspaceClass ExpidusWorkspaceClass;
typedef struct _ExpidusWorkspacePrivate ExpidusWorkspacePrivate;

struct _ExpidusWorkspace
{
    GObject gobject;

    /*< private >*/
    ExpidusWorkspacePrivate *priv;
};

struct _ExpidusWorkspaceClass
{
    GObjectClass parent_class;

    /*< signals >*/
    void (*changed)(ExpidusWorkspace *workspace, ExpidusBackdrop *backdrop);
};

GType expidus_workspace_get_type                     (void) G_GNUC_CONST;

ExpidusWorkspace *expidus_workspace_new(GdkScreen *gscreen,
                                  EsconfChannel *channel,
                                  const gchar *property_prefix,
                                  gint number);

gint expidus_workspace_get_workspace_num(ExpidusWorkspace *workspace);
void expidus_workspace_set_workspace_num(ExpidusWorkspace *workspace, gint number);

void expidus_workspace_monitors_changed(ExpidusWorkspace *workspace,
                                     GdkScreen *gscreen);

gboolean expidus_workspace_get_xinerama_stretch(ExpidusWorkspace *workspace);


ExpidusBackdrop *expidus_workspace_get_backdrop(ExpidusWorkspace *workspace,
                                          guint monitor);

G_END_DECLS

#endif
