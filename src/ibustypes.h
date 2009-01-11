/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __IBUS_TYPES_H_
#define __IBUS_TYPES_H_

typedef enum
{
    IBUS_SHIFT_MASK    = 1 << 0,
    IBUS_LOCK_MASK     = 1 << 1,
    IBUS_CONTROL_MASK  = 1 << 2,
    IBUS_MOD1_MASK     = 1 << 3,
    IBUS_MOD2_MASK     = 1 << 4,
    IBUS_MOD3_MASK     = 1 << 5,
    IBUS_MOD4_MASK     = 1 << 6,
    IBUS_MOD5_MASK     = 1 << 7,
    IBUS_BUTTON1_MASK  = 1 << 8,
    IBUS_BUTTON2_MASK  = 1 << 9,
    IBUS_BUTTON3_MASK  = 1 << 10,
    IBUS_BUTTON4_MASK  = 1 << 11,
    IBUS_BUTTON5_MASK  = 1 << 12,

    /* The next few modifiers are used by XKB, so we skip to the end.
     * Bits 15 - 25 are currently unused. Bit 29 is used internally.
     */

    /* forard mask */
    IBUS_FORWARD_MASK  = 1 << 25,

    IBUS_SUPER_MASK    = 1 << 26,
    IBUS_HYPER_MASK    = 1 << 27,
    IBUS_META_MASK     = 1 << 28,

    IBUS_RELEASE_MASK  = 1 << 30,

    IBUS_MODIFIER_MASK = 0x5c001fff
} IBusModifierType;

typedef enum {
    IBUS_CAP_PREEDIT_TEXT       = 1 << 0,
    IBUS_CAP_AUXILIARY_TEXT     = 1 << 1,
    IBUS_CAP_LOOKUP_TABLE       = 1 << 2,
    IBUS_CAP_FOCUS              = 1 << 3,
    IBUS_CAP_PROPERTY           = 1 << 4,
} IBusCapabilite;

typedef struct _IBusRectangle IBusRectangle;
struct _IBusRectangle {
    gint x;
    gint y;
    gint width;
    gint height;
};

typedef struct _IBusComponent IBusComponent;
typedef struct _IBusEngineInfo IBusEngineInfo;
struct _IBusEngineInfo {
    gchar *name;
    gchar *longname;
    gchar *description;
    gchar *language;
    gchar *license;
    gchar *author;
    gchar *icon;
    gchar *layout;
    IBusComponent *component;
};

typedef struct _IBusObservedPath IBusObservedPath;
struct _IBusObservedPath {
    gchar *path;
    gint   is_exist:1;
    gint   is_dir:1;
    glong  mtime;
};

struct _IBusComponent {
    gchar *name;
    gchar *description;
    gchar *exec;
    gchar *version;
    gchar *author;
    gchar *license;
    gchar *homepage;
    gchar *service_name;
    gchar *filename;
    glong  mtime;
    
    /* text domain for dgettext */
    gchar *textdomain;
    
    /* engines */
    gchar  *engine_exec;
    IBusEngineInfo **engines;
    
    /* observed paths */
    IBusObservedPath **observed_paths;
    
    gboolean is_active;
    GPid     pid;
};

typedef void (* IBusFreeFunc) (gpointer );

#endif

