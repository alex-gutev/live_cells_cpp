# Initialization
AC_INIT([live_cells], [0.2.3], [alex.gutev@gmail.com])
AC_CONFIG_SRCDIR([src/live_cells.cpp])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AM_SILENT_RULES([yes])
AM_PROG_AR

LT_INIT


## Required Tools

# Compilers

AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(20)

# Unit testing

AX_BOOST_BASE([1.82])
AX_BOOST_UNIT_TEST_FRAMEWORK


AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])
AC_OUTPUT
