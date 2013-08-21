#!/bin/sh

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

if test ! -f devel/bin/test_all.sh
then
    echo Error: can only run from root dir of repository
    exit 1
fi

# C
cd clownfish/compiler/c
./configure && make -j && make -j test
C_CFC_RESULT=$?
cd ../../runtime/c
./configure && make -j && make -j test
C_CFISH_RUNTIME_RESULT=$?
cd ../../../c
./configure && make -j && make -j test
C_LUCY_RESULT=$?
make distclean

# Perl
cd ../clownfish/compiler/perl
perl Build.PL && ./Build test
PERL_CFC_RESULT=$?
cd ../../runtime/perl
perl Build.PL && ./Build test
PERL_CFISH_RUNTIME_RESULT=$?
cd ../../../perl
perl Build.PL && ./Build test
PERL_LUCY_RESULT=$?
./Build realclean

# Exit with a failing value if any test failed.
if     [ $C_CFC_RESULT -ne 0 ] \
    || [ $C_CFISH_RUNTIME_RESULT -ne 0 ] \
    || [ $C_LUCY_RESULT -ne 0 ] \
    || [ $PERL_CFC_RESULT -ne 0 ] \
    || [ $PERL_CFISH_RUNTIME_RESULT -ne 0 ] \
    || [ $PERL_LUCY_RESULT -ne 0 ]
then
    exit 1
fi
exit 0

