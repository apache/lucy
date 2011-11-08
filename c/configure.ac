dnl Licensed to the Apache Software Foundation (ASF) under one or more
dnl contributor license agreements.  See the NOTICE file distributed with
dnl this work for additional information regarding copyright ownership.
dnl The ASF licenses this file to You under the Apache License, Version 2.0
dnl (the "License"); you may not use this file except in compliance with
dnl the License.  You may obtain a copy of the License at
dnl
dnl     http://www.apache.org/licenses/LICENSE-2.0
dnl
dnl Unless required by applicable law or agreed to in writing, software
dnl distributed under the License is distributed on an "AS IS" BASIS,
dnl WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
dnl See the License for the specific language governing permissions and
dnl limitations under the License.

AC_INIT([liblucy], [0.3.0], [The Apache Lucy Project <lucy-dev@incubator.apache.org], [http://incubator.apache.org/lucy/])
dnl AM_CONFIG_HEADER(src/acconfig.h)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR(config)
AC_SUBST(ac_aux_dir)
AC_CONFIG_FILES([
                Makefile
                src/Makefile
                liblucy.pc
                liblucy-config
                ])
PACKAGE=liblucy

MAJOR_VERSION=0
MINOR_VERSION=3
MICRO_VERSION=0
INTERFACE_AGE=0
BINARY_AGE=0
VERSION=$MAJOR_VERSION.$MINOR_VERSION.$MICRO_VERSION

AC_ARG_ENABLE(daystamp,
             AC_HELP_STRING([--enable-daystamp], [Adds today's date to version]),
             daystamp=yes,)

if test x$daystamp = xyes; then
        TODAY=`/bin/date +%Y%m%d`
        VERSION="$VERSION-$TODAY"
fi
LIBLUCY_VERSION=$VERSION
AC_SUBST(LIBLUCY_VERSION)
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
                
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_OUTPUT
