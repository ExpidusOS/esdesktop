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

#ifndef __ESDESKTOP_ICON_VIEW_H__
#define __ESDESKTOP_ICON_VIEW_H__

#include <gtk/gtk.h>

#include "esdesktop-icon.h"
#include "esdesktop-icon-view-manager.h"

G_BEGIN_DECLS

#define ESDESKTOP_TYPE_ICON_VIEW     (esdesktop_icon_view_get_type())
#define ESDESKTOP_ICON_VIEW(obj)     (G_TYPE_CHECK_INSTANCE_CAST((obj), ESDESKTOP_TYPE_ICON_VIEW, EsdesktopIconView))
#define ESDESKTOP_IS_ICON_VIEW(obj)  (G_TYPE_CHECK_INSTANCE_TYPE((obj), ESDESKTOP_TYPE_ICON_VIEW))

typedef struct _EsdesktopIconView         EsdesktopIconView;
typedef struct _EsdesktopIconViewClass    EsdesktopIconViewClass;
typedef struct _EsdesktopIconViewPrivate  EsdesktopIconViewPrivate;

typedef void (*EsdesktopIconViewIconInitFunc)(EsdesktopIconView *icon_view);
typedef void (*EsdesktopIconViewIconFiniFunc)(EsdesktopIconView *icon_view);

struct _EsdesktopIconView
{
    GtkWidget parent;

    /*< private >*/
    EsdesktopIconViewPrivate *priv;
};

struct _EsdesktopIconViewClass
{
    GtkWidgetClass parent;

    /*< signals >*/
    void (*icon_selection_changed)(EsdesktopIconView *icon_view);
    void (*icon_activated)(EsdesktopIconView *icon_view);

    void (*select_all)(EsdesktopIconView *icon_view);
    void (*unselect_all)(EsdesktopIconView *icon_view);

    void (*select_cursor_item)(EsdesktopIconView *icon_view);
    void (*toggle_cursor_item)(EsdesktopIconView *icon_view);

    gboolean (*activate_selected_items)(EsdesktopIconView *icon_view);

    gboolean (*move_cursor)(EsdesktopIconView *icon_view,
                            GtkMovementStep step,
                            gint count);

    void (*resize_event)(EsdesktopIconView *icon_view);
};

GType esdesktop_icon_view_get_type(void) G_GNUC_CONST;

GtkWidget *esdesktop_icon_view_new(EsdesktopIconViewManager *manager);

void esdesktop_icon_view_add_item(EsdesktopIconView *icon_view,
                                  EsdesktopIcon *icon);

void esdesktop_icon_view_remove_item(EsdesktopIconView *icon_view,
                                     EsdesktopIcon *icon);
void esdesktop_icon_view_remove_all(EsdesktopIconView *icon_view);

void esdesktop_icon_view_set_selection_mode(EsdesktopIconView *icon_view,
                                            GtkSelectionMode mode);
GtkSelectionMode esdesktop_icon_view_get_selection_mode(EsdesktopIconView *icon_view);

void esdesktop_icon_view_enable_drag_source(EsdesktopIconView *icon_view,
                                            GdkModifierType start_button_mask,
                                            const GtkTargetEntry *targets,
                                            gint n_targets,
                                            GdkDragAction actions);
void esdesktop_icon_view_enable_drag_dest(EsdesktopIconView *icon_view,
                                          const GtkTargetEntry *targets,
                                          gint n_targets,
                                          GdkDragAction actions);
void esdesktop_icon_view_unset_drag_source(EsdesktopIconView *icon_view);
void esdesktop_icon_view_unset_drag_dest(EsdesktopIconView *icon_view);

EsdesktopIcon *esdesktop_icon_view_widget_coords_to_item(EsdesktopIconView *icon_view,
                                                         gint wx,
                                                         gint wy);

GList *esdesktop_icon_view_get_selected_items(EsdesktopIconView *icon_view);

void esdesktop_icon_view_select_item(EsdesktopIconView *icon_view,
                                     EsdesktopIcon *icon);
void esdesktop_icon_view_select_all(EsdesktopIconView *icon_view);
void esdesktop_icon_view_unselect_item(EsdesktopIconView *icon_view,
                                       EsdesktopIcon *icon);
void esdesktop_icon_view_unselect_all(EsdesktopIconView *icon_view);

void esdesktop_icon_view_set_icon_size(EsdesktopIconView *icon_view,
                                       guint icon_size);
guint esdesktop_icon_view_get_icon_size(EsdesktopIconView *icon_view);

void esdesktop_icon_view_set_primary(EsdesktopIconView *icon_view,
                                     gboolean primary);

void esdesktop_icon_view_set_font_size(EsdesktopIconView *icon_view,
                                       gdouble font_size_points);
gdouble esdesktop_icon_view_get_font_size(EsdesktopIconView *icon_view);
void esdesktop_icon_view_set_center_text (EsdesktopIconView *icon_view,
                                          gboolean center_text);

GtkWidget *esdesktop_icon_view_get_window_widget(EsdesktopIconView *icon_view);

gboolean
esdesktop_get_workarea_single(EsdesktopIconView *icon_view,
                              guint ws_num,
                              gint *xorigin,
                              gint *yorigin,
                              gint *width,
                              gint *height);

void esdesktop_icon_view_sort_icons(EsdesktopIconView *icon_view);

#if defined(DEBUG) && DEBUG > 0
guint _esdesktop_icon_view_n_items(EsdesktopIconView *icon_view);
#endif

G_END_DECLS

#endif  /* __ESDESKTOP_ICON_VIEW_H__ */
