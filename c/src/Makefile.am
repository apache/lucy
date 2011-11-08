# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

AM_CFLAGS= -Wall 
# -pg is for profiling
#AM_CFLAGS= -Wall -pg
LIBLUCY_VERSION = @LIBLUCY_VERSION@

myprep: liblucy.c
	cd .. && perl build/liblucy.pl && mv charmony.h src/

all: myprep
	echo "make all complete"

AM_LDFLAGS= -I$(top_srcdir)/src -I. -L$(top_srcdir)/src -llucy
bin_PROGRAMS = lucy-test
lucy_test_SOURCES = lucy-test.c

lib_LTLIBRARIES = liblucy.la
liblucy_la_LDFLAGS = -no-undefined -version-info 0:3:0 
#liblucy_la_LDFLAGS = -no-undefined -version-info 0:3:0 -pg
#include_HEADERS = liblucy.h

liblucy_la_SOURCES = liblucy.c

TESTS = $(check_PROGRAMS) test.sh
