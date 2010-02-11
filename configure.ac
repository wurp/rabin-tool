#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(config.h.in)
AM_INIT_AUTOMAKE(rabinpoly, 1.0a, hakim@cs.cmu.edu)
AM_CONFIG_HEADER([config.h])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AM_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([bzero])

CPPFLAGS="$CPPFLAGS"
AC_OUTPUT(Makefile src/Makefile)
