/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright(c) 2006 Brian Tarricone, <bjt23@cornell.edu>
 *  Copyright(c) 2006 Benedikt Meurer, <benny@expidus.org>
 *  Copyright(c) 2010-2011 Jannis Pohlmann, <jannis@expidus.org>
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
 *
 *  esdesktop-thumbnailer is based on thumbnailer code from Ristretto
 *  Copyright (c) Stephan Arts 2009-2011 <stephan@expidus.org>
 */

#ifndef __ESDESKTOP_THUMBNAILER_H__
#define __ESDESKTOP_THUMBNAILER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_THUMBNAILER             (esdesktop_thumbnailer_get_type())
#define ESDESKTOP_THUMBNAILER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_THUMBNAILER, EsdesktopThumbnailer))
#define ESDESKTOP_IS_THUMBNAILER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_THUMBNAILER))
#define ESDESKTOP_THUMBNAILER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ESDESKTOP_TYPE_THUMBNAILER, EsdesktopThumbnailerClass))
#define ESDESKTOP_IS_THUMBNAILER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ESDESKTOP_TYPE_THUMBNAILER()))

typedef struct _EsdesktopThumbnailer EsdesktopThumbnailer;
typedef struct _EsdesktopThumbnailerPriv EsdesktopThumbnailerPriv;

struct _EsdesktopThumbnailer
{
    GObject parent;

    EsdesktopThumbnailerPriv *priv;
};

typedef struct _EsdesktopThumbnailerClass EsdesktopThumbnailerClass;

struct _EsdesktopThumbnailerClass
{
    GObjectClass parent_class;

    /*< signals >*/
    void (*thumbnail_ready)(gchar *src_file, gchar *thumb_file);
};

EsdesktopThumbnailer * esdesktop_thumbnailer_new(void);

GType esdesktop_thumbnailer_get_type(void);

gboolean esdesktop_thumbnailer_service_available(EsdesktopThumbnailer *thumbnailer);

gboolean esdesktop_thumbnailer_is_supported(EsdesktopThumbnailer *thumbnailer,
                                            gchar *file);

gboolean esdesktop_thumbnailer_queue_thumbnail(EsdesktopThumbnailer *thumbnailer,
                                               gchar *file);
void esdesktop_thumbnailer_dequeue_thumbnail(EsdesktopThumbnailer *thumbnailer,
                                             gchar *file);
void esdesktop_thumbnailer_dequeue_all_thumbnails(EsdesktopThumbnailer *thumbnailer);

void esdesktop_thumbnailer_delete_thumbnail(EsdesktopThumbnailer *thumbnailer,
                                            gchar *src_file);

G_END_DECLS

#endif /* __ESDESKTOP_THUMBNAILER_H__ */
