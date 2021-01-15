/*
 *  esdesktop - expidus1's desktop manager
 *
 *  Copyright (c) 2004-2009 Brian Tarricone, <bjt23@cornell.edu>
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
 *  X event forwarding code:
 *     Copyright (c) 2004 Nils Rennebarth
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include <libexpidus1util/libexpidus1util.h>

#include "esdesktop-application.h"

int
main(int argc, char **argv)
{
    EsdesktopApplication *app;
    int ret = 0;

#ifdef G_ENABLE_DEBUG
    /* do NOT remove this line. If something doesn't work,
     * fix your code instead! */
    g_log_set_always_fatal(G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

    /* bind gettext textdomain */
    expidus_textdomain(GETTEXT_PACKAGE, LOCALEDIR, "UTF-8");

    app = esdesktop_application_get();

    ret = esdesktop_application_run(app, argc, argv);

    g_object_unref(app);

    return ret;
}
