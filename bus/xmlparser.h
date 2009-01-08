/* vim:set et sts=4: */
/* bus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#include <glib.h>

typedef struct {
    gchar *name;
    gchar *value;
} XMLAttribute;

typedef struct {
    gchar  *name;
    gchar  *text;
    gchar  **attributes;
    GSList *sub_nodes;
} XMLNode;

XMLNode *xml_parse_file (const gchar    *name);
void     xml_free_node  (XMLNode        *node);
void     xml_output     (const XMLNode  *node,
                         GString        *output);
#endif
