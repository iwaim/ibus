#!/usr/bin/env perl

# Updates http://svn.gnome.org/viewcvs/gtk%2B/trunk/gdk/gdkkeysyms.h?view=log from upstream (X.org 7.x),
# from http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob_plain;f=keysymdef.h
#
# Author  : Simos Xenitellis <simos at gnome dot org>.
# Version : 1.2
#
# Input   : http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob_plain;f=keysymdef.h
# Output  : http://svn.gnome.org/svn/gtk+/trunk/gdk/gdkkeysyms.h
#
# Notes   : It downloads keysymdef.h from the Internet, if not found locally,
# Notes   : and creates an updated gdkkeysyms.h
# Notes   : This version updates the source of gdkkeysyms.h from CVS to the GIT server.

use strict;

# Used for reading the keysymdef symbols.
my @keysymelements;

if ( ! -f "keysymdef.h" )
{
	print "Trying to download keysymdef.h from\n";
	print "http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob_plain;f=keysymdef.h\n";
	die "Unable to download keysymdef.h from http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob_plain;f=keysymdef.h\n"
		unless system("wget -c -O keysymdef.h \"http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob_plain;f=keysymdef.h\"") == 0;
	print " done.\n\n";
}
else
{
	print "We are using existing keysymdef.h found in this directory.\n";
	print "It is assumed that you took care and it is a recent version\n";
	print "as found at http://gitweb.freedesktop.org/?p=xorg/proto/x11proto.git;a=blob;f=keysymdef.h\n\n";
}


if ( -f "ibuskeysyms.h" )
{
	print "There is already a ibuskeysyms.h file in this directory. We are not overwriting it.\n";
	print "Please move it somewhere else in order to run this script.\n";
	die "Exiting...\n\n";
}

# Source: http://cvs.freedesktop.org/xorg/xc/include/keysymdef.h
die "Could not open file keysymdef.h: $!\n" unless open(IN_KEYSYMDEF, "<:utf8", "keysymdef.h");

# Output: gtk+/gdk/gdkkeysyms.h
die "Could not open file ibuskeysyms.h: $!\n" unless open(OUT_IBUSKEYSYMS, ">:utf8", "ibuskeysyms.h");

print OUT_IBUSKEYSYMS<<EOF;
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang\@gmail.com>
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

#ifndef __IBUS_KEYSYMS_H__
#define __IBUS_KEYSYMS_H__


EOF


while (<IN_KEYSYMDEF>)
{
	next if ( ! /^#define / );

	@keysymelements = split(/\s+/);
	die "Internal error, no \@keysymelements: $_\n" unless @keysymelements;

	$_ = $keysymelements[1];
	die "Internal error, was expecting \"XC_*\", found: $_\n" if ( ! /^XK_/ );

	$_ = $keysymelements[2];
	die "Internal error, was expecting \"0x*\", found: $_\n" if ( ! /^0x/ );

	$keysymelements[1] =~ s/^XK_/IBUS_/g;

	printf OUT_IBUSKEYSYMS "#define %s 0x%03x\n", $keysymelements[1], hex($keysymelements[2]);
}

#$ibussyms{"0"} = "0000";

close IN_KEYSYMDEF;


print OUT_IBUSKEYSYMS<<EOF;

#endif /* __IBUS_KEYSYMS_H__ */
EOF

printf "We just finished converting keysymdef.h to ibuskeysyms.h\nThank you\n";
