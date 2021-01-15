/*
 *  backdrop - expidus1's desktop manager
 *
 *  Copyright (c) 2004 Brian Tarricone, <bjt23@cornell.edu>
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

#ifndef _EXPIDUS_BACKDROP_H_
#define _EXPIDUS_BACKDROP_H_

#include <glib-object.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define EXPIDUS_TYPE_BACKDROP              (expidus_backdrop_get_type())
#define EXPIDUS_BACKDROP(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), EXPIDUS_TYPE_BACKDROP, ExpidusBackdrop))
#define EXPIDUS_BACKDROP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), EXPIDUS_TYPE_BACKDROP, ExpidusBackdropClass))
#define EXPIDUS_IS_BACKDROP(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), EXPIDUS_TYPE_BACKDROP))
#define EXPIDUS_IS_BACKDROP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), EXPIDUS_TYPE_BACKDROP))
#define EXPIDUS_BACKDROP_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), EXPIDUS_TYPE_BACKDROP, ExpidusBackdropClass))

typedef struct _ExpidusBackdrop ExpidusBackdrop;
typedef struct _ExpidusBackdropClass ExpidusBackdropClass;
typedef struct _ExpidusBackdropPrivate ExpidusBackdropPrivate;

typedef enum
{
    EXPIDUS_BACKDROP_IMAGE_INVALID = -1,
    EXPIDUS_BACKDROP_IMAGE_NONE = 0,
    EXPIDUS_BACKDROP_IMAGE_CENTERED,
    EXPIDUS_BACKDROP_IMAGE_TILED,
    EXPIDUS_BACKDROP_IMAGE_STRETCHED,
    EXPIDUS_BACKDROP_IMAGE_SCALED,
    EXPIDUS_BACKDROP_IMAGE_ZOOMED,
    EXPIDUS_BACKDROP_IMAGE_SPANNING_SCREENS,
} ExpidusBackdropImageStyle;

typedef enum
{
    EXPIDUS_BACKDROP_COLOR_INVALID = -1,
    EXPIDUS_BACKDROP_COLOR_SOLID = 0,
    EXPIDUS_BACKDROP_COLOR_HORIZ_GRADIENT,
    EXPIDUS_BACKDROP_COLOR_VERT_GRADIENT,
    EXPIDUS_BACKDROP_COLOR_TRANSPARENT,
} ExpidusBackdropColorStyle;

typedef enum
{
    EXPIDUS_BACKDROP_PERIOD_INVALID = -1,
    EXPIDUS_BACKDROP_PERIOD_SECONDS = 0,
    EXPIDUS_BACKDROP_PERIOD_MINUTES,
    EXPIDUS_BACKDROP_PERIOD_HOURS,
    EXPIDUS_BACKDROP_PERIOD_STARTUP,
    EXPIDUS_BACKDROP_PERIOD_HOURLY,
    EXPIDUS_BACKDROP_PERIOD_DAILY,
    EXPIDUS_BACKDROP_PERIOD_CHRONOLOGICAL,
} ExpidusBackdropCyclePeriod;

struct _ExpidusBackdrop
{
    GObject gobject;

    /*< private >*/
    ExpidusBackdropPrivate *priv;
};

struct _ExpidusBackdropClass
{
    GObjectClass parent_class;

    /*< signals >*/
    void (*changed)(ExpidusBackdrop *backdrop);
    void (*cycle)(ExpidusBackdrop *backdrop);
    void (*ready)(ExpidusBackdrop *backdrop);
};

GType expidus_backdrop_get_type             (void) G_GNUC_CONST;

ExpidusBackdrop *expidus_backdrop_new          (GdkVisual *visual);

ExpidusBackdrop *expidus_backdrop_new_with_size(GdkVisual *visual,
                                          gint width,
                                          gint height);

void expidus_backdrop_set_size              (ExpidusBackdrop *backdrop,
                                          gint width,
                                          gint height);

void expidus_backdrop_set_color_style       (ExpidusBackdrop *backdrop,
                                          ExpidusBackdropColorStyle style);
ExpidusBackdropColorStyle expidus_backdrop_get_color_style
                                         (ExpidusBackdrop *backdrop);

void expidus_backdrop_set_first_color       (ExpidusBackdrop *backdrop,
                                          const GdkRGBA *color);
void expidus_backdrop_get_first_color       (ExpidusBackdrop *backdrop,
                                          GdkRGBA *color);

void expidus_backdrop_set_second_color      (ExpidusBackdrop *backdrop,
                                          const GdkRGBA *color);
void expidus_backdrop_get_second_color      (ExpidusBackdrop *backdrop,
                                          GdkRGBA *color);

void expidus_backdrop_set_image_style       (ExpidusBackdrop *backdrop,
                                          ExpidusBackdropImageStyle style);
ExpidusBackdropImageStyle expidus_backdrop_get_image_style
                                         (ExpidusBackdrop *backdrop);

void expidus_backdrop_set_image_filename    (ExpidusBackdrop *backdrop,
                                          const gchar *filename);
const gchar *expidus_backdrop_get_image_filename
                                         (ExpidusBackdrop *backdrop);

void expidus_backdrop_set_cycle_backdrop    (ExpidusBackdrop *backdrop,
                                          gboolean cycle_backdrop);
gboolean expidus_backdrop_get_cycle_backdrop(ExpidusBackdrop *backdrop);

void expidus_backdrop_set_cycle_period      (ExpidusBackdrop *backdrop,
                                          ExpidusBackdropCyclePeriod period);
ExpidusBackdropCyclePeriod expidus_backdrop_get_cycle_period
                                         (ExpidusBackdrop *backdrop);

void expidus_backdrop_set_cycle_timer       (ExpidusBackdrop *backdrop,
                                          guint cycle_timer);
guint expidus_backdrop_get_cycle_timer      (ExpidusBackdrop *backdrop);

void expidus_backdrop_set_random_order      (ExpidusBackdrop *backdrop,
                                          gboolean random_order);
gboolean expidus_backdrop_get_random_order  (ExpidusBackdrop *backdrop);

void expidus_backdrop_force_cycle           (ExpidusBackdrop *backdrop);


GdkPixbuf *expidus_backdrop_get_pixbuf      (ExpidusBackdrop *backdrop);

void expidus_backdrop_generate_async        (ExpidusBackdrop *backdrop);

void expidus_backdrop_clear_cached_image    (ExpidusBackdrop *backdrop);

G_END_DECLS

#endif
