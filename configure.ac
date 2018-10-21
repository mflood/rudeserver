# Process this file with autoconf to produce a configure script.
# 
# Copyright (C) 2002, 2003, 2004, 2005 Matthew Flood
# See file AUTHORS for contact information
# 
# This file is part of RudeServer.
# 
# RudeServer is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
# 
# RudeServer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with RudeServer; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#------------------------------------------------------------------------

AC_INIT(RudeServer, 1.2, bug-rudeserver@rudeserver.com, rudeserver)
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE(rudeserver, 1.2)


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strrchr])




AC_DEFUN([CHECK_SABLOT],
[AC_MSG_CHECKING(if Sablot is wanted)
AC_ARG_WITH(sablot,
[  --with-sablot enable Sablot XSLT Engine [will check /usr/local/sablot
                            /usr/lib/sablot /usr/sablot /usr/pkg /usr/local /usr ]
],
[   AC_MSG_RESULT(yes)
    for dir in $withval /usr/local/sablot /usr/lib/sablot /usr/sablot /usr/pkg /usr/local /usr; do
        sablotdir="$dir"
        if test -f "$dir/include/sablot.h"; then
            found_sablot="yes";
            CFLAGS="$CFLAGS -I$sablotdir/include -DUSING_SABLOT";
            CXXFLAGS="$CXXFLAGS -I$sablotdir/include -DUSING_SABLOT";
            break;
        fi
        if test -f "$dir/include/sablot.h"; then
            found_sablot="yes";
            CFLAGS="$CFLAGS -I$sablotdir/include/ -DUSING_SABLOT";
            CXXFLAGS="$CXXFLAGS -I$sablotdir/include/ -DUSING_SABLOT";
            break
        fi
    done
    if test x_$found_sablot != x_yes; then
        AC_MSG_ERROR(Cannot find sablot libraries)
    else
        printf "Sablot found in $ssldir\n";
        LIBS="$LIBS -lsablot -lexpat";
        LDFLAGS="$LDFLAGS -L$sablot/lib";
        USING_SABLOT=yes
    fi
    AC_SUBST(USING_SABLOT)
],
[
    AC_MSG_RESULT(no)
])
])dnl





AC_DEFUN([CHECK_XALAN],
[AC_MSG_CHECKING(if Xalan is wanted)
AC_ARG_WITH(xalan,
[  --with-xalan enable Xalan 1.7 XSLT Engine [will use xalan] 
],
[   AC_MSG_RESULT(yes)

	printf "Using XALAN";
   CFLAGS="$CFLAGS -DUSING_XALAN_1_7";
   CXXFLAGS="$CXXFLAGS -DUSING_XALAN_1_7";
	LIBS="$LIBS -lxalan-c -lxerces-c";
	USING_XALAN_1_7=yes
	AC_SUBST(USING_XALAN_1_7)
],
[
    AC_MSG_RESULT(no)
])
])dnl

AC_DEFUN([CHECK_LIBXSLT],
[AC_MSG_CHECKING(if LibXSLT is wanted)
AC_ARG_WITH(libxslt,
[  --with-xalan enable libxslt Engine (Gnome) [will use libxslt] 
],
[   AC_MSG_RESULT(yes)

	printf "Using GNOME libXSLT";
   CFLAGS="$CFLAGS -I/usr/include/libxml2 -DUSING_LIBXSLT";
   CXXFLAGS="$CXXFLAGS -I/usr/include/libxml2 -DUSING_LIBXSLT";
	LIBS="$LIBS -lxslt -lxml2";
	USING_LIBXSLT=yes
	AC_SUBST(USING_LIBXSLT)
],
[
    AC_MSG_RESULT(no)
])
])dnl





CHECK_SABLOT()
CHECK_XALAN()
CHECK_LIBXSLT()


AC_OUTPUT(Makefile )









