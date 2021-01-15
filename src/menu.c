/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright (c) 2004-2008 Brian J. Tarricone <bjt23@cornell.edu>
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
 *  Random portions taken from or inspired by the original esdesktop for expidus1:
 *     Copyright (C) 2002-2003 Jasper Huijsmans (huysmans@users.sourceforge.net)
 *     Copyright (C) 2003 Benedikt Meurer <benedikt.meurer@unix-ag.uni-siegen.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <libexpidus1util/libexpidus1util.h>
#include <libexpidus1ui/libexpidus1ui.h>

#include "esdesktop-common.h"
#include "menu.h"
#ifdef USE_DESKTOP_MENU
#include <markon/markon.h>
#include <markon-gtk/markon-gtk.h>
#endif

#ifdef USE_DESKTOP_MENU
static gboolean show_desktop_menu = TRUE;
static gboolean show_desktop_menu_icons = TRUE;
static MarkonMenu *markon_menu = NULL;
#endif

#ifdef USE_DESKTOP_MENU
static void
menu_populate(ExpidusDesktop *desktop,
              GtkMenuShell *menu,
              gpointer user_data)
{
    GtkWidget *mi, *img = NULL;
    GtkIconTheme *itheme = gtk_icon_theme_get_default();
    GtkWidget *desktop_menu = NULL;
    GList *menu_children;

    TRACE("ENTERING");

    if(!show_desktop_menu)
        return;

    /* init markon environment */
    markon_set_environment_xdg(MARKON_ENVIRONMENT_EXPIDUS);

    if(markon_menu == NULL) {
        markon_menu = markon_menu_new_applications();
    }
    desktop_menu = markon_gtk_menu_new (markon_menu);
    XF_DEBUG("show desktop menu icons %s", show_desktop_menu_icons ? "TRUE" : "FALSE");
    markon_gtk_menu_set_show_menu_icons(MARKON_GTK_MENU(desktop_menu), show_desktop_menu_icons);

    /* check to see if the menu is empty.  if not, add the desktop menu
    * to a submenu */
    menu_children = gtk_container_get_children(GTK_CONTAINER(menu));
    if(menu_children) {
        g_list_free(menu_children);
        mi = gtk_separator_menu_item_new();
        gtk_widget_show(mi);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

        if(gtk_icon_theme_has_icon(itheme, "applications-other")) {
            img = gtk_image_new_from_icon_name("applications-other",
                                            GTK_ICON_SIZE_MENU);
            gtk_widget_show(img);
        }

        mi = esdesktop_menu_create_menu_item_with_mnemonic(_("_Applications"), img);
        gtk_widget_show(mi);

        gtk_menu_item_set_submenu (GTK_MENU_ITEM(mi), desktop_menu);

        gtk_menu_shell_append(menu, mi);
    }
    /* just get the menu as a list of toplevel GtkMenuItems instead of
    * a toplevel menu */
    else
    {
        expidus_gtk_menu_popup_until_mapped(GTK_MENU(desktop_menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
    }
}
#endif /* USE_DESKTOP_MENU */

#ifdef USE_DESKTOP_MENU
static void
menu_settings_changed(EsconfChannel *channel,
                      const gchar *property,
                      const GValue *value,
                      gpointer user_data)
{
    if(!strcmp(property, "/desktop-menu/show")) {
        show_desktop_menu = G_VALUE_TYPE(value)
                            ? g_value_get_boolean(value)
                            : TRUE;
    } else if(!strcmp(property, "/desktop-menu/show-icons")) {
        show_desktop_menu_icons = G_VALUE_TYPE(value)
                                  ? g_value_get_boolean(value)
                                  : TRUE;
    }
}
#endif

void
menu_init(EsconfChannel *channel)
{
#ifdef USE_DESKTOP_MENU
    if(!channel || esconf_channel_get_bool(channel, "/desktop-menu/show", TRUE))
    {
        show_desktop_menu = TRUE;
        if(channel) {
            show_desktop_menu_icons = esconf_channel_get_bool(channel,
                                                              "/desktop-menu/show-icons",
                                                              TRUE);
        }
    } else {
        show_desktop_menu = FALSE;
    }

    if(channel) {
        g_signal_connect(G_OBJECT(channel), "property-changed",
                         G_CALLBACK(menu_settings_changed), NULL);
    }
#endif
}

void
menu_attach(ExpidusDesktop *desktop)
{
#ifdef USE_DESKTOP_MENU
    DBG("attached default menu");
    g_signal_connect_after(G_OBJECT(desktop), "populate-root-menu",
                           G_CALLBACK(menu_populate), NULL);
#endif /* USE_DESKTOP_MENU */
}

void
menu_cleanup(void)
{
}
