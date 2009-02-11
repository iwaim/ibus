/* vim:set et sts=4: */
/* IBus - The Input Bus
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
/**
 * SECTION: ibusattribute
 * @short_description: Attributes of preedit and auxiliary text.
 * 
 */

#ifndef __IBUS_ATTRIBUTE_H_
#define __IBUS_ATTRIBUTE_H_

#include "ibusserializable.h"

/*
 * Type macros.
 */
/* define IBusAttribute macros */
#define IBUS_TYPE_ATTRIBUTE             \
    (ibus_attribute_get_type ())
#define IBUS_ATTRIBUTE(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_ATTRIBUTE, IBusAttribute))
#define IBUS_ATTRIBUTE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_ATTRIBUTE, IBusAttributeClass))
#define IBUS_IS_ATTRIBUTE(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_ATTRIBUTE))
#define IBUS_IS_ATTRIBUTE_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_ATTRIBUTE))
#define IBUS_ATTRIBUTE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_ATTRIBUTE, IBusAttributeClass))

/* define IBusAttrList macros */
#define IBUS_TYPE_ATTR_LIST             \
    (ibus_attr_list_get_type ())
#define IBUS_ATTR_LIST(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_ATTR_LIST, IBusAttrList))
#define IBUS_ATTR_LIST_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_ATTR_LIST, IBusAttrListClass))
#define IBUS_IS_ATTR_LIST(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_ATTR_LIST))
#define IBUS_IS_ATTR_LIST_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_ATTR_LIST))
#define IBUS_ATTR_LIST_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_ATTR_LIST, IBusAttrListClass))

typedef enum {
    IBUS_ATTR_TYPE_UNDERLINE    = 1,
    IBUS_ATTR_TYPE_FOREGROUND   = 2,
    IBUS_ATTR_TYPE_BACKGROUND   = 3,
} IBusAttrType;

typedef enum {
    IBUS_ATTR_UNDERLINE_NONE    = 0,
    IBUS_ATTR_UNDERLINE_SINGLE  = 1,
    IBUS_ATTR_UNDERLINE_DOUBLE  = 2,
    IBUS_ATTR_UNDERLINE_LOW     = 3,
} IBusAttrUnderline;

G_BEGIN_DECLS

typedef struct _IBusAttribute IBusAttribute;
typedef struct _IBusAttributeClass IBusAttributeClass;
typedef struct _IBusAttrList IBusAttrList;
typedef struct _IBusAttrListClass IBusAttrListClass;

struct _IBusAttribute {
    IBusSerializable parent;

    /* members */
    guint type;
    guint value;
    guint start_index;
    guint end_index;
};

struct _IBusAttributeClass {
    IBusSerializableClass parent;
};

struct _IBusAttrList {
    IBusSerializable parent;

    /* members */
    GArray *attributes;
};

struct _IBusAttrListClass {
    IBusSerializableClass parent;
};

GType                ibus_attribute_get_type    ();
IBusAttribute       *ibus_attribute_new         (guint           type,
                                                 guint           value,
                                                 guint           start_index,
                                                 guint           end_index);
IBusAttribute       *ibus_attr_underline_new    (guint           underline_type,
                                                 guint           start_index,
                                                 guint           end_index);
IBusAttribute       *ibus_attr_foreground_new   (guint           color,
                                                 guint           start_index,
                                                 guint           end_index);
IBusAttribute       *ibus_attr_background_new   (guint           color,
                                                 guint           start_index,
                                                 guint           end_index);


GType                ibus_attr_list_get_type    ();
IBusAttrList        *ibus_attr_list_new         ();
void                 ibus_attr_list_append      (IBusAttrList   *attr_list,
                                                 IBusAttribute  *attr);
IBusAttribute       *ibus_attr_list_get         (IBusAttrList   *attr_list,
                                                 guint           index);

G_END_DECLS
#endif

