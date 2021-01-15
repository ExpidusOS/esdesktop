/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2010      Jannis Pohlmann <jannis@expidus.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gio/gio.h>

#include <gtk/gtk.h>

#include "esdesktop-clipboard-manager.h"
#include "esdesktop-file-icon.h"
#include "esdesktop-file-icon-manager.h"
#include "esdesktop-file-utils.h"

#ifndef I_
#define I_(str)  g_intern_static_string(str)
#endif


enum
{
  PROP_0,
  PROP_CAN_PASTE,
};

enum
{
  CHANGED,
  LAST_SIGNAL,
};

enum
{
  TARGET_TEXT_URI_LIST,
  TARGET_GNOME_COPIED_FILES,
  TARGET_UTF8_STRING,
};



static void esdesktop_clipboard_manager_class_init        (EsdesktopClipboardManagerClass *klass);
static void esdesktop_clipboard_manager_init              (EsdesktopClipboardManager      *manager);
static void esdesktop_clipboard_manager_finalize          (GObject                        *object);
static void esdesktop_clipboard_manager_get_property      (GObject                        *object,
                                                           guint                           prop_id,
                                                           GValue                         *value,
                                                           GParamSpec                     *pspec);

static void esdesktop_clipboard_manager_file_destroyed    (EsdesktopClipboardManager      *manager,
                                                           EsdesktopFileIcon              *file);
static void esdesktop_clipboard_manager_owner_changed     (GtkClipboard                   *clipboard,
                                                           GdkEventOwnerChange            *event,
                                                           EsdesktopClipboardManager      *manager);

static void esdesktop_clipboard_manager_contents_received (GtkClipboard                   *clipboard,
                                                           GtkSelectionData               *selection_data,
                                                           gpointer                        user_data);

static void esdesktop_clipboard_manager_targets_received  (GtkClipboard                   *clipboard,
                                                           GtkSelectionData               *selection_data,
                                                           gpointer                        user_data);
static void esdesktop_clipboard_manager_get_callback      (GtkClipboard                   *clipboard,
                                                           GtkSelectionData               *selection_data,
                                                           guint                           info,
                                                           gpointer                        user_data);
static void esdesktop_clipboard_manager_clear_callback    (GtkClipboard                   *clipboard,
                                                           gpointer                        user_data);
static void esdesktop_clipboard_manager_transfer_files    (EsdesktopClipboardManager      *manager,
                                                           gboolean                        copy,
                                                           GList                          *files);



struct _EsdesktopClipboardManagerClass
{
  GObjectClass __parent__;

  void (*changed) (EsdesktopClipboardManager *manager);
};

struct _EsdesktopClipboardManager
{
  GObject __parent__;

  GtkClipboard *clipboard;
  gboolean      can_paste;
  GdkAtom       x_special_gnome_copied_files;

  gboolean      files_cutted;
  GList        *files;
};

typedef struct
{
  EsdesktopClipboardManager *manager;
  GFile                     *target_file;
  GtkWidget                 *widget;
  GClosure                  *new_files_closure;
  EsdesktopFileIconManager *fmanager;
} EsdesktopClipboardPasteRequest;



static const GtkTargetEntry clipboard_targets[] =
{
  { "text/uri-list", 0, TARGET_TEXT_URI_LIST },
  { "x-special/gnome-copied-files", 0, TARGET_GNOME_COPIED_FILES },
  { "UTF8_STRING", 0, TARGET_UTF8_STRING }
};

static GObjectClass *esdesktop_clipboard_manager_parent_class;
static GQuark        esdesktop_clipboard_manager_quark = 0;
static guint         manager_signals[LAST_SIGNAL];



GType
esdesktop_clipboard_manager_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GTypeInfo info =
      {
        sizeof (EsdesktopClipboardManagerClass),
        NULL,
        NULL,
        (GClassInitFunc) esdesktop_clipboard_manager_class_init,
        NULL,
        NULL,
        sizeof (EsdesktopClipboardManager),
        0,
        (GInstanceInitFunc) esdesktop_clipboard_manager_init,
        NULL,
      };

      type = g_type_register_static (G_TYPE_OBJECT, I_("EsdesktopClipboardManager"), &info, 0);
    }

  return type;
}



static void
esdesktop_clipboard_manager_class_init (EsdesktopClipboardManagerClass *klass)
{
  GObjectClass *gobject_class;

  /* determine the parent type class */
  esdesktop_clipboard_manager_parent_class = g_type_class_peek_parent (klass);

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = esdesktop_clipboard_manager_finalize;
  gobject_class->get_property = esdesktop_clipboard_manager_get_property;

  /**
   * EsdesktopClipboardManager:can-paste:
   *
   * This property tells whether the current clipboard content of
   * this #EsdesktopClipboardManager can be pasted into the desktop
   * displayed by #EsdesktopIconView.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_CAN_PASTE,
                                   g_param_spec_boolean ("can-paste", "can-pase", "can-paste",
                                                         FALSE,
                                                         G_PARAM_READABLE));
  /**
   * EsdesktopClipboardManager::changed:
   * @manager : a #EsdesktopClipboardManager.
   *
   * This signal is emitted whenever the contents of the
   * clipboard associated with @manager changes.
   **/
  manager_signals[CHANGED] =
    g_signal_new (I_("changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (EsdesktopClipboardManagerClass, changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}



static void
esdesktop_clipboard_manager_init (EsdesktopClipboardManager *manager)
{
  manager->x_special_gnome_copied_files = gdk_atom_intern ("x-special/gnome-copied-files", FALSE);
}



static void
esdesktop_clipboard_manager_finalize (GObject *object)
{
  EsdesktopClipboardManager *manager = ESDESKTOP_CLIPBOARD_MANAGER (object);
  GList                  *lp;

  /* release any pending files */
  for (lp = manager->files; lp != NULL; lp = lp->next)
    {
      g_object_weak_unref(G_OBJECT(lp->data),
                          (GWeakNotify)esdesktop_clipboard_manager_file_destroyed,
                          manager);
      g_object_unref (G_OBJECT (lp->data));
    }
  g_list_free (manager->files);

  /* disconnect from the clipboard */
  g_signal_handlers_disconnect_by_func (G_OBJECT (manager->clipboard),
                                        esdesktop_clipboard_manager_owner_changed,
                                        manager);
  g_object_set_qdata (G_OBJECT (manager->clipboard),
                      esdesktop_clipboard_manager_quark,
                      NULL);
  g_object_unref (G_OBJECT (manager->clipboard));

  (*G_OBJECT_CLASS (esdesktop_clipboard_manager_parent_class)->finalize) (object);
}



static void
esdesktop_clipboard_manager_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  EsdesktopClipboardManager *manager = ESDESKTOP_CLIPBOARD_MANAGER (object);

  switch (prop_id)
    {
    case PROP_CAN_PASTE:
      g_value_set_boolean (value, esdesktop_clipboard_manager_get_can_paste (manager));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
esdesktop_clipboard_manager_file_destroyed (EsdesktopClipboardManager *manager,
                                            EsdesktopFileIcon             *file)
{
  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  g_return_if_fail (g_list_find (manager->files, file) != NULL);

  /* remove the file from our list */
  manager->files = g_list_remove (manager->files, file);

  /* disconnect from the file */
  g_object_weak_unref(G_OBJECT (file),
                      (GWeakNotify)esdesktop_clipboard_manager_file_destroyed,
                      manager);
  g_object_unref (G_OBJECT (file));
}



static void
esdesktop_clipboard_manager_owner_changed (GtkClipboard           *clipboard,
                                           GdkEventOwnerChange    *event,
                                           EsdesktopClipboardManager *manager)
{
  g_return_if_fail (GTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  g_return_if_fail (manager->clipboard == clipboard);

  /* need to take a reference on the manager, because the clipboards
   * "targets received callback" mechanism is not cancellable.
   */
  g_object_ref (G_OBJECT (manager));

  /* request the list of supported targets from the new owner */
  gtk_clipboard_request_contents (clipboard, gdk_atom_intern ("TARGETS", FALSE),
                                  esdesktop_clipboard_manager_targets_received, manager);
}


static void
esdesktop_clipboard_manager_contents_received (GtkClipboard     *clipboard,
                                            GtkSelectionData *selection_data,
                                            gpointer          user_data)
{
  EsdesktopClipboardPasteRequest *request = user_data;
  EsdesktopClipboardManager      *manager = ESDESKTOP_CLIPBOARD_MANAGER (request->manager);
  GtkWindow                      *parent = GTK_WINDOW(gtk_widget_get_toplevel(request->widget));
  gboolean                        path_copy = TRUE;
  GList                          *path_list = NULL;
  GList                          *dest_file_list  = NULL;
  GList                          *l               = NULL;
  gchar                          *data;

  /* check whether the retrieval worked */
  if (G_LIKELY (gtk_selection_data_get_length(selection_data) > 0))
    {
      /* be sure the selection data is zero-terminated */
      data = (gchar *) gtk_selection_data_get_data(selection_data);
      data[gtk_selection_data_get_length(selection_data)] = '\0';

      /* check whether to copy or move */
      if (g_ascii_strncasecmp (data, "copy\n", 5) == 0)
        {
          path_copy = TRUE;
          data += 5;
        }
      else if (g_ascii_strncasecmp (data, "cut\n", 4) == 0)
        {
          path_copy = FALSE;
          data += 4;
        }

      /* determine the path list stored with the selection */
      path_list = esdesktop_file_utils_file_list_from_string (data);
    }

  /* perform the action if possible */
  if (G_LIKELY (path_list != NULL))
    {
      for (l = path_list; l; l = l->next) {
        gchar *dest_basename = g_file_get_basename(l->data);

        if(dest_basename && *dest_basename != '\0') {
          /* If we copy a file, we need to use the new absolute filename
           * as the destination. If we move, we need to use the destination
           * directory. */
           if(path_copy) {
             GFile *dest_file = g_file_get_child(request->target_file, dest_basename);
             dest_file_list = g_list_prepend(dest_file_list, dest_file);
           } else {
             dest_file_list = g_list_prepend(dest_file_list, request->target_file);
           }
         }
         g_free(dest_basename);
      }

      dest_file_list = g_list_reverse(dest_file_list);

      if (G_LIKELY (path_copy))
      {
        esdesktop_file_utils_transfer_files(GDK_ACTION_COPY,
                                            path_list,
                                            dest_file_list,
                                            gtk_widget_get_screen(GTK_WIDGET(parent)));
      } else {
        esdesktop_file_utils_transfer_files(GDK_ACTION_MOVE,
                                            path_list,
                                            dest_file_list,
                                            gtk_widget_get_screen(GTK_WIDGET(parent)));
      }

      /* clear the clipboard if it contained "cutted data"
       * (gtk_clipboard_clear takes care of not clearing
       * the selection if we don't own it)
       */
      if (G_UNLIKELY (!path_copy))
        gtk_clipboard_clear (manager->clipboard);

      /* check the contents of the clipboard again
       * if either the Xserver or our GTK+ version
       * doesn't support the XFixes extension.
       */
      if (!gdk_display_supports_selection_notification (gtk_clipboard_get_display (manager->clipboard)))
        {
          esdesktop_clipboard_manager_owner_changed (manager->clipboard, NULL, manager);
        }
    }

  /* free the request */
  if (G_LIKELY (request->widget != NULL))
    g_object_remove_weak_pointer (G_OBJECT (request->widget), (gpointer) &request->widget);
  if (G_LIKELY (request->new_files_closure != NULL))
    g_closure_unref (request->new_files_closure);
  g_object_unref (G_OBJECT (request->manager));

  g_list_free(dest_file_list);
  g_list_free(path_list);
}



static void
esdesktop_clipboard_manager_targets_received (GtkClipboard     *clipboard,
                                              GtkSelectionData *selection_data,
                                              gpointer          user_data)
{
  EsdesktopClipboardManager *manager = ESDESKTOP_CLIPBOARD_MANAGER (user_data);
  GdkAtom                *targets;
  gint                    n_targets;
  gint                    n;

  g_return_if_fail (GTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  g_return_if_fail (manager->clipboard == clipboard);

  /* reset the "can-paste" state */
  manager->can_paste = FALSE;

  /* check the list of targets provided by the owner */
  if (gtk_selection_data_get_targets (selection_data, &targets, &n_targets))
    {
      for (n = 0; n < n_targets; ++n)
        if (targets[n] == manager->x_special_gnome_copied_files)
          {
            manager->can_paste = TRUE;
            break;
          }

      g_free (targets);
    }

  /* notify listeners that we have a new clipboard state */
  g_signal_emit (G_OBJECT (manager), manager_signals[CHANGED], 0);
  g_object_notify (G_OBJECT (manager), "can-paste");

  /* drop the reference taken for the callback */
  g_object_unref (G_OBJECT (manager));
}


static void
esdesktop_clipboard_manager_get_callback (GtkClipboard     *clipboard,
                                          GtkSelectionData *selection_data,
                                          guint             target_info,
                                          gpointer          user_data)
{
  EsdesktopClipboardManager *manager = ESDESKTOP_CLIPBOARD_MANAGER (user_data);
  GList                  *file_list = NULL;
  gchar                  *string_list;
  gchar                  *data;
  gchar                 **uris;

  g_return_if_fail (GTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  g_return_if_fail (manager->clipboard == clipboard);

  /* determine the file list from the icon list */
  file_list = esdesktop_file_utils_file_icon_list_to_file_list (manager->files);

  /* determine the string representation of the file list */
  string_list = esdesktop_file_utils_file_list_to_string (file_list);

  switch (target_info)
    {
    case TARGET_TEXT_URI_LIST:
      uris = esdesktop_file_utils_file_list_to_uri_array (file_list);
      gtk_selection_data_set_uris (selection_data, uris);
      g_strfreev (uris);
      break;

    case TARGET_GNOME_COPIED_FILES:
      data = g_strconcat (manager->files_cutted ? "cut\n" : "copy\n", string_list, NULL);
      gtk_selection_data_set (selection_data,
                              gtk_selection_data_get_target(selection_data),
                              8,
                              (guchar *) data,
                              strlen (data));
      g_free (data);
      break;

    case TARGET_UTF8_STRING:
      gtk_selection_data_set (selection_data,
                              gtk_selection_data_get_target(selection_data),
                              8,
                              (guchar *) string_list,
                              strlen (string_list));
      break;

    default:
      g_assert_not_reached ();
    }

  /* cleanup */
  esdesktop_file_utils_file_list_free (file_list);
  g_free (string_list);
}



static void
esdesktop_clipboard_manager_clear_callback (GtkClipboard *clipboard,
                                            gpointer      user_data)
{
  EsdesktopClipboardManager *manager = ESDESKTOP_CLIPBOARD_MANAGER (user_data);
  GList                  *lp;

  g_return_if_fail (GTK_IS_CLIPBOARD (clipboard));
  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  g_return_if_fail (manager->clipboard == clipboard);

  /* release the pending files */
  for (lp = manager->files; lp != NULL; lp = lp->next)
    {
      g_object_weak_unref(G_OBJECT (lp->data),
                          (GWeakNotify)esdesktop_clipboard_manager_file_destroyed,
                          manager);
      g_object_unref (G_OBJECT (lp->data));
    }
  g_list_free (manager->files);
  manager->files = NULL;
}



static void
esdesktop_clipboard_manager_transfer_files (EsdesktopClipboardManager *manager,
                                            gboolean                copy,
                                            GList                  *files)
{
  EsdesktopFileIcon *file;
  GList      *lp;

  /* release any pending files */
  for (lp = manager->files; lp != NULL; lp = lp->next)
    {
      g_object_weak_unref(G_OBJECT (lp->data),
                          (GWeakNotify)esdesktop_clipboard_manager_file_destroyed,
                          manager);
      g_object_unref (G_OBJECT (lp->data));
    }
  g_list_free (manager->files);

  /* remember the transfer operation */
  manager->files_cutted = !copy;

  /* setup the new file list */
  for (lp = files, manager->files = NULL; lp != NULL; lp = lp->next)
    {
      file = ESDESKTOP_FILE_ICON(g_object_ref (G_OBJECT (lp->data)));
      manager->files = g_list_prepend (manager->files, file);
      g_object_weak_ref(G_OBJECT(file),
                        (GWeakNotify)esdesktop_clipboard_manager_file_destroyed,
                        manager);
    }

  /* acquire the CLIPBOARD ownership */
  gtk_clipboard_set_with_owner (manager->clipboard, clipboard_targets,
                                G_N_ELEMENTS (clipboard_targets),
                                esdesktop_clipboard_manager_get_callback,
                                esdesktop_clipboard_manager_clear_callback,
                                G_OBJECT (manager));

  /* Need to fake a "owner-change" event here if the Xserver doesn't support clipboard notification */
  if (!gdk_display_supports_selection_notification (gtk_clipboard_get_display (manager->clipboard)))
    {
      esdesktop_clipboard_manager_owner_changed (manager->clipboard, NULL, manager);
    }
}



/**
 * esdesktop_clipboard_manager_get_for_display:
 * @display : a #GdkDisplay.
 *
 * Determines the #EsdesktopClipboardManager that is used to manage
 * the clipboard on the given @display.
 *
 * The caller is responsible for freeing the returned object
 * using g_object_unref() when it's no longer needed.
 *
 * Return value: the #EsdesktopClipboardManager for @display.
 **/
EsdesktopClipboardManager*
esdesktop_clipboard_manager_get_for_display (GdkDisplay *display)
{
  EsdesktopClipboardManager *manager;
  GtkClipboard           *clipboard;

  g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);

  /* generate the quark on-demand */
  if (G_UNLIKELY (esdesktop_clipboard_manager_quark == 0))
    esdesktop_clipboard_manager_quark = g_quark_from_static_string ("esdesktop-clipboard-manager");

  /* figure out the clipboard for the given display */
  clipboard = gtk_clipboard_get_for_display (display, GDK_SELECTION_CLIPBOARD);

  /* check if a clipboard manager exists */
  manager = g_object_get_qdata (G_OBJECT (clipboard), esdesktop_clipboard_manager_quark);
  if (G_LIKELY (manager != NULL))
    {
      g_object_ref (G_OBJECT (manager));
      return manager;
    }

  /* allocate a new manager */
  manager = g_object_new (ESDESKTOP_TYPE_CLIPBOARD_MANAGER, NULL);
  manager->clipboard = GTK_CLIPBOARD (g_object_ref (G_OBJECT (clipboard)));
  g_object_set_qdata (G_OBJECT (clipboard), esdesktop_clipboard_manager_quark, manager);

  /* listen for the "owner-change" signal on the clipboard */
  g_signal_connect (G_OBJECT (manager->clipboard), "owner-change",
                    G_CALLBACK (esdesktop_clipboard_manager_owner_changed), manager);

  return manager;
}



/**
 * esdesktop_clipboard_manager_has_cutted_file:
 * @manager : a #EsdesktopClipboardManager.
 * @file    : a #EsdesktopFile.
 *
 * Checks whether @file was cutted to the given @manager earlier.
 *
 * Return value: %TRUE if @file is on the cutted list of @manager.
 **/
gboolean
esdesktop_clipboard_manager_has_cutted_file (EsdesktopClipboardManager *manager,
                                             const EsdesktopFileIcon       *file)
{
  g_return_val_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager), FALSE);
  g_return_val_if_fail (ESDESKTOP_IS_FILE_ICON (file), FALSE);

  return (manager->files_cutted && g_list_find (manager->files, file) != NULL);
}



/**
 * esdesktop_clipboard_manager_copy_files:
 * @manager : a #EsdesktopClipboardManager.
 * @files   : a list of #EsdesktopFile<!---->s.
 *
 * Sets the clipboard represented by @manager to
 * contain the @files and marks them to be copied
 * when the user pastes from the clipboard.
 **/
void
esdesktop_clipboard_manager_copy_files (EsdesktopClipboardManager *manager,
                                        GList                  *files)
{
  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  esdesktop_clipboard_manager_transfer_files (manager, TRUE, files);
}



/**
 * esdesktop_clipboard_manager_cut_files:
 * @manager : a #EsdesktopClipboardManager.
 * @files   : a list of #EsdesktopFile<!---->s.
 *
 * Sets the clipboard represented by @manager to
 * contain the @files and marks them to be moved
 * when the user pastes from the clipboard.
 **/
void
esdesktop_clipboard_manager_cut_files (EsdesktopClipboardManager *manager,
                                       GList                  *files)
{
  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  esdesktop_clipboard_manager_transfer_files (manager, FALSE, files);
}

gboolean
esdesktop_clipboard_manager_get_can_paste (EsdesktopClipboardManager *manager)
{
    g_return_val_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager), FALSE);
    return manager->can_paste;
}


/**
 * lunar_clipboard_manager_paste_files:
 * @manager           : a #EsdesktopClipboardManager.
 * @target_file       : the #GFile of the folder to which the contents on the clipboard
 *                      should be pasted.
 * @widget            : a #GtkWidget, on which to perform the paste or %NULL if no widget is
 *                      known.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Pastes the contents from the clipboard associated with @manager to the directory
 * referenced by @target_file.
 * Code copied and adapted from lunar-clipboard-manager.c
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@expidus.org>
 **/
void
esdesktop_clipboard_manager_paste_files (EsdesktopClipboardManager *manager,
                                      GFile                  *target_file,
                                      GtkWidget              *widget,
                                      GClosure               *new_files_closure)
{
  EsdesktopClipboardPasteRequest *request;

  g_return_if_fail (ESDESKTOP_IS_CLIPBOARD_MANAGER (manager));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  /* prepare the paste request */
  request = g_slice_new0 (EsdesktopClipboardPasteRequest);
  request->manager = ESDESKTOP_CLIPBOARD_MANAGER (g_object_ref (G_OBJECT (manager)));
  request->target_file = g_object_ref (target_file);
  request->widget = widget;

  /* take a reference on the closure (if any) */
  if (G_LIKELY (new_files_closure != NULL))
    {
      request->new_files_closure = new_files_closure;
      g_closure_ref (new_files_closure);
      g_closure_sink (new_files_closure);
    }

  /* get notified when the widget is destroyed prior to
   * completing the clipboard contents retrieval
   */
  if (G_LIKELY (request->widget != NULL))
    g_object_add_weak_pointer (G_OBJECT (request->widget), (gpointer) &request->widget);

  /* schedule the request */
  gtk_clipboard_request_contents (manager->clipboard, manager->x_special_gnome_copied_files,
                                  esdesktop_clipboard_manager_contents_received, request);
}
