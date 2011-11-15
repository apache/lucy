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

myldflags= -I$(top_srcdir)/src -I. -I$(top_srcdir)/../charmonizer/src -L$(top_srcdir)/src -llucy
AM_LDFLAGS= $(myldflags)
bin_PROGRAMS = 000-sanity-check.t
000_sanity_check_t_SOURCES = 000-sanity-check.t.c
000_sanity_check_t_LDFLAGS = $(myldflags)
000_sanity_check_t_CFLAGS = $(myldflags)


TESTS = $(check_PROGRAMS) test.sh 

test:
	prove *t
