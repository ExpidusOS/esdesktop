/*
 *  esdesktop - expidus1's desktop manager
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

#ifndef _EXPIDUS_DESKTOP_H_
#define _EXPIDUS_DESKTOP_H_

#include <gtk/gtk.h>
#include <esconf/esconf.h>

#include "expidus-backdrop.h"

G_BEGIN_DECLS

#define EXPIDUS_TYPE_DESKTOP              (expidus_desktop_get_type())
#define EXPIDUS_DESKTOP(object)           (G_TYPE_CHECK_INSTANCE_CAST((object), EXPIDUS_TYPE_DESKTOP, ExpidusDesktop))
#define EXPIDUS_DESKTOP_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), EXPIDUS_TYPE_DESKTOP, ExpidusDesktopClass))
#define EXPIDUS_IS_DESKTOP(object)        (G_TYPE_CHECK_INSTANCE_TYPE((object), EXPIDUS_TYPE_DESKTOP))
#define EXPIDUS_IS_DESKTOP_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass), EXPIDUS_TYPE_DESKTOP))
#define EXPIDUS_DESKTOP_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS((object), EXPIDUS_TYPE_DESKTOP, ExpidusDesktopClass))

typedef struct _ExpidusDesktop ExpidusDesktop;
typedef struct _ExpidusDesktopClass ExpidusDesktopClass;
typedef struct _ExpidusDesktopPrivate ExpidusDesktopPrivate;

typedef void (*SessionLogoutFunc)();

typedef enum
{
    EXPIDUS_DESKTOP_ICON_STYLE_NONE = 0,
    EXPIDUS_DESKTOP_ICON_STYLE_WINDOWS,
    EXPIDUS_DESKTOP_ICON_STYLE_FILES,
} ExpidusDesktopIconStyle;

struct _ExpidusDesktop
{
    GtkWindow window;

    /*< private >*/
    ExpidusDesktopPrivate *priv;
};

struct _ExpidusDesktopClass
{
    GtkWindowClass parent_class;

    /*< signals >*/

    /* for the app menu/file context menu */
    void (*populate_root_menu)(ExpidusDesktop *desktop,
                               GtkMenuShell *menu);

    /* for the windowlist menu */
    void (*populate_secondary_root_menu)(ExpidusDesktop *desktop,
                                         GtkMenuShell *menu);
};

GType expidus_desktop_get_type                     (void) G_GNUC_CONST;

GtkWidget *expidus_desktop_new(GdkScreen *gscreen,
                            EsconfChannel *channel,
                            const gchar *property_prefix);

gint expidus_desktop_get_n_monitors(ExpidusDesktop *desktop);

void expidus_desktop_set_icon_style(ExpidusDesktop *desktop,
                                 ExpidusDesktopIconStyle style);
ExpidusDesktopIconStyle expidus_desktop_get_icon_style(ExpidusDesktop *desktop);

void expidus_desktop_set_icon_size(ExpidusDesktop *desktop,
                                guint icon_size);

void expidus_desktop_set_primary(ExpidusDesktop *desktop,
                              gboolean primary);

void expidus_desktop_set_use_icon_font_size(ExpidusDesktop *desktop,
                                         gboolean use_system);
void expidus_desktop_set_icon_font_size(ExpidusDesktop *desktop,
                                     guint font_size_points);

void expidus_desktop_set_center_text(ExpidusDesktop *desktop,
                                  gboolean center_text);

void expidus_desktop_set_session_logout_func(ExpidusDesktop *desktop,
                                          SessionLogoutFunc logout_func);

void expidus_desktop_freeze_updates(ExpidusDesktop *desktop);
void expidus_desktop_thaw_updates(ExpidusDesktop *desktop);


void expidus_desktop_popup_root_menu(ExpidusDesktop *desktop,
                                  guint button,
                                  guint activate_time);
void expidus_desktop_popup_secondary_root_menu(ExpidusDesktop *desktop,
                                            guint button,
                                            guint activate_time);

void expidus_desktop_refresh(ExpidusDesktop *desktop,
                          gboolean advance_wallpaper,
                          gboolean all_monitors);

void expidus_desktop_arrange_icons(ExpidusDesktop *desktop);

gboolean expidus_desktop_get_cycle_backdrop(ExpidusDesktop *desktop);

G_END_DECLS

#endif
