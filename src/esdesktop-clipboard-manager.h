/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2010 Jannis Pohlmann <jannis@expidus.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * Copied, renamed, and hacked to pieces by Brian Tarricone <bjt23@cornell.edu>.
 * Original code from Lunar.
 */

#ifndef __ESDESKTOP_CLIPBOARD_MANAGER_H__
#define __ESDESKTOP_CLIPBOARD_MANAGER_H__

#include <gio/gio.h>

G_BEGIN_DECLS;

/* fwd decl */
struct _EsdesktopFileIcon;

typedef struct _EsdesktopClipboardManagerClass EsdesktopClipboardManagerClass;
typedef struct _EsdesktopClipboardManager      EsdesktopClipboardManager;

#define ESDESKTOP_TYPE_CLIPBOARD_MANAGER             (esdesktop_clipboard_manager_get_type ())
#define ESDESKTOP_CLIPBOARD_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ESDESKTOP_TYPE_CLIPBOARD_MANAGER, EsdesktopClipboardManager))
#define ESDESKTOP_CLIPBOARD_MANAGER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((obj), ESDESKTOP_TYPE_CLIPBOARD_MANAGER, EsdesktopClipboardManagerClass))
#define ESDESKTOP_IS_CLIPBOARD_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ESDESKTOP_TYPE_CLIPBOARD_MANAGER))
#define ESDESKTOP_IS_CLIPBAORD_MANAGER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ESDESKTOP_TYPE_CLIPBOARD_MANAGER))
#define ESDESKTOP_CLIPBOARD_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ESDESKTOP_TYPE_CLIPBAORD_MANAGER, EsdesktopClipboardManagerClass))

GType                      esdesktop_clipboard_manager_get_type        (void) G_GNUC_CONST;

EsdesktopClipboardManager *esdesktop_clipboard_manager_get_for_display (GdkDisplay                      *display);

gboolean                   esdesktop_clipboard_manager_get_can_paste   (EsdesktopClipboardManager       *manager);

gboolean                   esdesktop_clipboard_manager_has_cutted_file (EsdesktopClipboardManager       *manager,
                                                                        const struct _EsdesktopFileIcon *file);

void                       esdesktop_clipboard_manager_copy_files      (EsdesktopClipboardManager       *manager,
                                                                        GList                           *files);
void                       esdesktop_clipboard_manager_cut_files       (EsdesktopClipboardManager       *manager,
                                                                        GList                           *files);
void                       esdesktop_clipboard_manager_paste_files     (EsdesktopClipboardManager       *manager,
                                                                        GFile                           *target_file,
                                                                        GtkWidget                       *widget,
                                                                        GClosure                        *new_files_closure);

G_END_DECLS;

#endif /* !__ESDESKTOP_CLIPBOARD_MANAGER_H__ */
