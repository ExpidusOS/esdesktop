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
 *
 *  Thumbnailer Spec
 *  https://wiki.gnome.org/action/show/DraftSpecs/ThumbnailerSpec
 *  Thumbnail Managing Standard
 *  https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html
 */

#include <config.h>

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include <libexpidus1util/libexpidus1util.h>
#include "esdesktop-thumbnailer.h"
#include "esdesktop-marshal.h"
#include "esdesktop-common.h"
#include "tumbler.h"

static void esdesktop_thumbnailer_init(GObject *);
static void esdesktop_thumbnailer_class_init(GObjectClass *);

static void esdesktop_thumbnailer_dispose(GObject *object);
static void esdesktop_thumbnailer_finalize(GObject *object);

static void esdesktop_thumbnailer_request_finished_dbus(TumblerThumbnailer1 *proxy,
                                                        guint arg_handle,
                                                        gpointer data);

static void esdesktop_thumbnailer_thumbnail_ready_dbus(TumblerThumbnailer1 *proxy,
                                                       guint handle,
                                                       const gchar *const *uri,
                                                       gpointer data);

static gboolean esdesktop_thumbnailer_queue_request_timer(gpointer user_data);

static GObjectClass *parent_class = NULL;
static EsdesktopThumbnailer *thumbnailer_object = NULL;

enum
{
    THUMBNAIL_READY,
    LAST_SIGNAL,
};

static guint thumbnailer_signals[LAST_SIGNAL] = { 0, };

GType
esdesktop_thumbnailer_get_type(void)
{
    static GType esdesktop_thumbnailer_type = 0;

    if(!esdesktop_thumbnailer_type) {
        static const GTypeInfo esdesktop_thumbnailer_info =
        {
            sizeof (EsdesktopThumbnailerClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) esdesktop_thumbnailer_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            sizeof (EsdesktopThumbnailer),
            0,
            (GInstanceInitFunc) esdesktop_thumbnailer_init,
            NULL
        };

        esdesktop_thumbnailer_type = g_type_register_static(
                                                    G_TYPE_OBJECT,
                                                    "EsdesktopThumbnailer",
                                                    &esdesktop_thumbnailer_info,
                                                    0);
    }
    return esdesktop_thumbnailer_type;
}

struct _EsdesktopThumbnailerPriv
{
    TumblerThumbnailer1      *proxy;

    GSList                   *queue;
    gchar                   **supported_mimetypes;
    gboolean                  big_thumbnails;
    guint                     handle;

    gint                      request_timer_id;
};

static void
esdesktop_thumbnailer_init(GObject *object)
{
    EsdesktopThumbnailer *thumbnailer;
    GDBusConnection      *connection;

    thumbnailer = ESDESKTOP_THUMBNAILER(object);

    thumbnailer->priv = g_new0(EsdesktopThumbnailerPriv, 1);

    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

    if(connection) {
        thumbnailer->priv->proxy = tumbler_thumbnailer1_proxy_new_sync(
                                    connection,
                                    G_DBUS_PROXY_FLAGS_NONE,
                                    "org.freedesktop.thumbnails.Thumbnailer1",
                                    "/org/freedesktop/thumbnails/Thumbnailer1",
                                    NULL,
                                    NULL);

        if(thumbnailer->priv->proxy) {
            gchar **supported_uris = NULL;
            gchar **supported_flavors = NULL;


            g_signal_connect(thumbnailer->priv->proxy,
                             "finished",
                             G_CALLBACK (esdesktop_thumbnailer_request_finished_dbus),
                             thumbnailer);
            g_signal_connect(thumbnailer->priv->proxy,
                             "ready",
                             G_CALLBACK(esdesktop_thumbnailer_thumbnail_ready_dbus),
                             thumbnailer);

            tumbler_thumbnailer1_call_get_supported_sync(thumbnailer->priv->proxy,
                                                         &supported_uris,
                                                         &thumbnailer->priv->supported_mimetypes,
                                                         NULL,
                                                         NULL);

            tumbler_thumbnailer1_call_get_flavors_sync(thumbnailer->priv->proxy,
                                                       &supported_flavors,
                                                       NULL,
                                                       NULL);

            if(supported_flavors != NULL) {
                gint n;
                for(n = 0; supported_flavors[n] != NULL; ++n) {
                    if(g_strcmp0(supported_flavors[n], "large")) {
                        thumbnailer->priv->big_thumbnails = TRUE;
                    }
                }
            } else {
                thumbnailer->priv->big_thumbnails = FALSE;
                g_warning("Thumbnailer failed calling GetFlavors");
            }

            g_strfreev(supported_flavors);
            g_strfreev(supported_uris);
        }

        g_object_unref(connection);
    }
}

static void
esdesktop_thumbnailer_class_init (GObjectClass *object_class)
{
    EsdesktopThumbnailerClass *thumbnailer_class = ESDESKTOP_THUMBNAILER_CLASS(object_class);

    parent_class = g_type_class_peek_parent(thumbnailer_class);

    object_class->dispose = esdesktop_thumbnailer_dispose;
    object_class->finalize = esdesktop_thumbnailer_finalize;

    thumbnailer_signals[THUMBNAIL_READY] = g_signal_new (
                        "thumbnail-ready",
                        G_OBJECT_CLASS_TYPE (object_class),
                        G_SIGNAL_RUN_LAST,
                        G_STRUCT_OFFSET(EsdesktopThumbnailerClass, thumbnail_ready),
                        NULL, NULL,
                        esdesktop_marshal_VOID__STRING_STRING,
                        G_TYPE_NONE, 2,
                        G_TYPE_STRING, G_TYPE_STRING);
}

/**
 * esdesktop_thumbnailer_dispose:
 * @object:
 *
 */
static void
esdesktop_thumbnailer_dispose(GObject *object)
{
    EsdesktopThumbnailer *thumbnailer = ESDESKTOP_THUMBNAILER(object);

    if(thumbnailer->priv) {
        if(thumbnailer->priv->proxy)
            g_object_unref(thumbnailer->priv->proxy);

        if(thumbnailer->priv->supported_mimetypes)
            g_strfreev(thumbnailer->priv->supported_mimetypes);

        g_free(thumbnailer->priv);
        thumbnailer->priv = NULL;
    }

    thumbnailer_object = NULL;
}

/**
 * esdesktop_thumbnailer_finalize:
 * @object:
 *
 */
static void
esdesktop_thumbnailer_finalize(GObject *object)
{
}

/**
 * esdesktop_thumbnailer_new:
 *
 *
 * Singleton
 */
EsdesktopThumbnailer *
esdesktop_thumbnailer_new(void)
{
    if(thumbnailer_object == NULL) {
        thumbnailer_object = g_object_new(ESDESKTOP_TYPE_THUMBNAILER, NULL);
    } else {
        g_object_ref(thumbnailer_object);
    }

    return thumbnailer_object;
}

gboolean esdesktop_thumbnailer_service_available(EsdesktopThumbnailer *thumbnailer)
{
    g_return_val_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer), FALSE);

    if(thumbnailer->priv->proxy == NULL)
        return FALSE;

    return TRUE;
}

gboolean
esdesktop_thumbnailer_is_supported(EsdesktopThumbnailer *thumbnailer,
                                   gchar *file)
{
    guint        n;
    gchar       *mime_type = NULL;

    g_return_val_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer), FALSE);
    g_return_val_if_fail(file != NULL, FALSE);

    mime_type = esdesktop_get_file_mimetype(file);

    if(mime_type == NULL) {
        XF_DEBUG("File %s has no mime type", file);
        return FALSE;
    }

    if(thumbnailer->priv->supported_mimetypes != NULL) {
        for(n = 0; thumbnailer->priv->supported_mimetypes[n] != NULL; ++n) {
            if(g_content_type_is_a (mime_type, thumbnailer->priv->supported_mimetypes[n])) {
                g_free(mime_type);
                return TRUE;
            }
        }
    }

    g_free(mime_type);
    return FALSE;
}

/**
 * esdesktop_thumbnailer_queue_thumbnail:
 *
 * Queues a file for thumbnail creation.
 * A "thumbnail-ready" signal will be emitted when the thumbnail is ready.
 * The signal will pass 2 parameters: a gchar *file which will be file
 * that's passed in here and a gchar *thumbnail_file which will be the
 * location of the thumbnail.
 */
gboolean
esdesktop_thumbnailer_queue_thumbnail(EsdesktopThumbnailer *thumbnailer,
                                      gchar *file)
{
    g_return_val_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer), FALSE);
    g_return_val_if_fail(file != NULL, FALSE);

    if(!esdesktop_thumbnailer_is_supported(thumbnailer, file)) {
        XF_DEBUG("file: %s not supported", file);
        return FALSE;
    }
    if(thumbnailer->priv->request_timer_id) {
        g_source_remove(thumbnailer->priv->request_timer_id);

        if(thumbnailer->priv->handle && thumbnailer->priv->proxy != NULL) {
            if(tumbler_thumbnailer1_call_dequeue_sync(thumbnailer->priv->proxy,
                                                      thumbnailer->priv->handle,
                                                      NULL,
                                                      NULL) == FALSE)
            {
                /* If this fails it usually means there's a thumbnail already
                 * being processed, no big deal */
                XF_DEBUG("Dequeue of thumbnailer->priv->handle: %d failed",
                         thumbnailer->priv->handle);
            }

            thumbnailer->priv->handle = 0;
        }
    }

    if(g_slist_find(thumbnailer->priv->queue, file) == NULL) {
        thumbnailer->priv->queue = g_slist_prepend(thumbnailer->priv->queue,
                                                   g_strdup(file));
    }

    thumbnailer->priv->request_timer_id = g_timeout_add_full(
                        G_PRIORITY_LOW,
                        300,
                        esdesktop_thumbnailer_queue_request_timer,
                        thumbnailer,
                        NULL);

    return TRUE;
}

static void
esdesktop_thumbnailer_dequeue_foreach(gpointer data, gpointer user_data)
{
    esdesktop_thumbnailer_dequeue_thumbnail(user_data, data);
}

/**
 * esdesktop_thumbnailer_dequeue_thumbnail:
 *
 * Removes a file from the list of pending thumbnail creations.
 * This is not guaranteed to always remove the file, if processing
 * of that thumbnail has started it won't stop.
 */
void
esdesktop_thumbnailer_dequeue_thumbnail(EsdesktopThumbnailer *thumbnailer,
                                        gchar *file)
{
    GSList *item;

    g_return_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer));
    g_return_if_fail(file != NULL);

    if(thumbnailer->priv->request_timer_id) {
        g_source_remove(thumbnailer->priv->request_timer_id);

        if(thumbnailer->priv->handle && thumbnailer->priv->proxy) {
            if(tumbler_thumbnailer1_call_dequeue_sync(thumbnailer->priv->proxy,
                                                      thumbnailer->priv->handle,
                                                      NULL,
                                                      NULL) == FALSE)
            {
                /* If this fails it usually means there's a thumbnail already
                 * being processed, no big deal */
                XF_DEBUG("Dequeue of thumbnailer->priv->handle: %d failed",
                         thumbnailer->priv->handle);
            }
        }
        thumbnailer->priv->handle = 0;
    }

    item = g_slist_find(thumbnailer->priv->queue, file);
    if(item != NULL) {
        g_free(item->data);
        thumbnailer->priv->queue = g_slist_remove(thumbnailer->priv->queue,
                                                  file);
    }

    thumbnailer->priv->request_timer_id = g_timeout_add_full(
                        G_PRIORITY_LOW,
                        300,
                        esdesktop_thumbnailer_queue_request_timer,
                        thumbnailer,
                        NULL);
}

void esdesktop_thumbnailer_dequeue_all_thumbnails(EsdesktopThumbnailer *thumbnailer)
{
    g_return_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer));

    g_slist_foreach(thumbnailer->priv->queue, (GFunc)esdesktop_thumbnailer_dequeue_foreach, thumbnailer);
}

static gboolean
esdesktop_thumbnailer_queue_request_timer(gpointer user_data)
{
    EsdesktopThumbnailer *thumbnailer = user_data;
    gchar **uris;
    gchar **mimetypes;
    GSList *iter;
    gint i = 0;
    GFile *file;
    GError *error = NULL;
    gchar *thumbnail_flavor;

    g_return_val_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer), FALSE);

    uris = g_new0(gchar *,
                  g_slist_length(thumbnailer->priv->queue) + 1);
    mimetypes = g_new0(gchar *,
                       g_slist_length (thumbnailer->priv->queue) + 1);

    iter = thumbnailer->priv->queue;
    while(iter) {
        if(iter->data) {
            file = g_file_new_for_path(iter->data);
            uris[i] = g_file_get_uri(file);
            mimetypes[i] = esdesktop_get_file_mimetype(iter->data);

            g_object_unref(file);
        }
        iter = g_slist_next(iter);
        i++;
    }

    if(thumbnailer->priv->big_thumbnails == TRUE)
        thumbnail_flavor = "large";
    else
        thumbnail_flavor = "normal";

    if(thumbnailer->priv->proxy != NULL) {
        if(tumbler_thumbnailer1_call_queue_sync(thumbnailer->priv->proxy,
                                                (const gchar * const*)uris,
                                                (const gchar * const*)mimetypes,
                                                thumbnail_flavor,
                                                "default",
                                                0,
                                                &thumbnailer->priv->handle,
                                                NULL,
                                                &error) == FALSE)
        {
            if(error != NULL)
                g_warning("DBUS-call failed: %s", error->message);
        }
    }

    /* Free the memory */
    i = 0;
    iter = thumbnailer->priv->queue;
    while(iter) {
        if(iter->data) {
            g_free(uris[i]);
            g_free(mimetypes[i]);
        }
        iter = g_slist_next(iter);
        i++;
    }

    g_free(uris);
    g_free(mimetypes);
    g_clear_error(&error);

    thumbnailer->priv->request_timer_id = 0;

    return FALSE;
}

static void
esdesktop_thumbnailer_request_finished_dbus(TumblerThumbnailer1 *proxy,
                                            guint arg_handle,
                                            gpointer data)
{
    EsdesktopThumbnailer *thumbnailer = ESDESKTOP_THUMBNAILER(data);

    g_return_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer));

    thumbnailer->priv->handle = 0;
}

static void
esdesktop_thumbnailer_thumbnail_ready_dbus(TumblerThumbnailer1 *proxy,
                                           guint handle,
                                           const gchar *const *uri,
                                           gpointer data)
{
    EsdesktopThumbnailer *thumbnailer = ESDESKTOP_THUMBNAILER(data);
    gchar *thumbnail_location;
    GFile *file;
    GSList *iter = thumbnailer->priv->queue;
    gchar *f_uri, *f_uri_checksum, *filename, *temp;
    gchar *thumbnail_flavor;
    gint x = 0;

    g_return_if_fail(ESDESKTOP_IS_THUMBNAILER(thumbnailer));

    while(iter) {
        if((uri[x] == NULL) || (iter->data == NULL)) {
            break;
        }

        file = g_file_new_for_path(iter->data);
        f_uri = g_file_get_uri(file);

        if(strcmp (uri[x], f_uri) == 0) {
            /* The thumbnail is in the format/location
             * $XDG_CACHE_HOME/thumbnails/(nromal|large)/MD5_Hash_Of_URI.png
             * for version 0.8.0 if XDG_CACHE_HOME is defined, otherwise
             * /homedir/.thumbnails/(normal|large)/MD5_Hash_Of_URI.png
             * will be used, which is also always used for versions prior
             * to 0.7.0.
             */
            f_uri_checksum = g_compute_checksum_for_string(G_CHECKSUM_MD5,
                                                           f_uri, strlen (f_uri));

            if(thumbnailer->priv->big_thumbnails == TRUE)
                thumbnail_flavor = "large";
            else
                thumbnail_flavor = "normal";

            filename = g_strconcat(f_uri_checksum, ".png", NULL);

            /* build and check if the thumbnail is in the new location */
            thumbnail_location = g_build_path("/", g_get_user_cache_dir(),
                                              "thumbnails", thumbnail_flavor,
                                              filename, NULL);

            if(!g_file_test(thumbnail_location, G_FILE_TEST_EXISTS)) {
                /* Fallback to old version */
                g_free(thumbnail_location);

                thumbnail_location = g_build_path("/", g_get_home_dir(),
                                                  ".thumbnails", thumbnail_flavor,
                                                  filename, NULL);
            }

            XF_DEBUG("thumbnail-ready src: %s thumbnail: %s",
                     (char*)iter->data,
                     thumbnail_location);

            if(g_file_test(thumbnail_location, G_FILE_TEST_EXISTS)) {
                g_signal_emit(G_OBJECT(thumbnailer),
                              thumbnailer_signals[THUMBNAIL_READY],
                              0,
                              iter->data,
                              thumbnail_location);
            }

            temp = iter->data;
            thumbnailer->priv->queue = g_slist_remove(thumbnailer->priv->queue,
                                                      temp);

            iter = thumbnailer->priv->queue;
            x++;

            g_free(filename);
            g_free(f_uri_checksum);
            g_free(thumbnail_location);
            g_free(temp);
        } else {
            iter = g_slist_next(iter);
        }

        g_object_unref(file);
        g_free(f_uri);
    }
}

/**
 * esdesktop_thumbnailer_delete_thumbnail:
 *
 * Tells the thumbnail service the src_file will be deleted.
 * This function should be called when the file is deleted or moved so
 * the thumbnail file doesn't take up space on the user's drive.
 */
void
esdesktop_thumbnailer_delete_thumbnail(EsdesktopThumbnailer *thumbnailer, gchar *src_file)
{
    GDBusConnection *connection;
    GVariantBuilder builder;
    GFile *file;
    GError *error = NULL;
    static GDBusProxy *cache = NULL;

    if(!cache) {
        connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
        if (connection != NULL) {
            cache = g_dbus_proxy_new_sync(connection,
                                          G_DBUS_PROXY_FLAGS_NONE,
                                          NULL,
                                          "org.freedesktop.thumbnails.Cache1",
                                          "/org/freedesktop/thumbnails/Cache1",
                                          "org.freedesktop.thumbnails.Cache1",
                                          NULL,
                                          NULL);

        g_object_unref(connection);
        }
    }

    file = g_file_new_for_path(src_file);

    if(cache) {
        g_variant_builder_init(&builder, G_VARIANT_TYPE ("as"));
        g_variant_builder_add(&builder, "s", g_file_get_uri(file));
        g_dbus_proxy_call_sync(cache,
                               "Delete",
                               g_variant_new("(as)", &builder),
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,
                               NULL,
                               &error);
        if(error != NULL) {
            g_warning("DBUS-call failed:%s", error->message);
        }
    }

    g_object_unref(file);
    g_clear_error(&error);
}
