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

#ifndef __ESDESKTOP_APPLICATION_H__
#define __ESDESKTOP_APPLICATION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_APPLICATION            (esdesktop_application_get_type())
#define ESDESKTOP_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_APPLICATION, EsdesktopApplication))
#define ESDESKTOP_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), ESDESKTOP_TYPE_APPLICATION, EsdesktopApplicationClass))
#define ESDESKTOP_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_APPLICATION))
#define ESDESKTOP_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), ESDESKTOP_TYPE_APPLICATION, EsdesktopApplicationClass))

typedef struct _EsdesktopApplication        EsdesktopApplication;
typedef struct _EsdesktopApplicationClass   EsdesktopApplicationClass;


GType esdesktop_application_get_type(void) G_GNUC_CONST;

EsdesktopApplication *esdesktop_application_get(void);

gint esdesktop_application_run(EsdesktopApplication *app, int argc, char **argv);

G_END_DECLS

#endif  /* __ESDESKTOP_ICON_H__ */
