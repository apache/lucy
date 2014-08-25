#!/bin/bash

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

if [ ! -e LICENSE ]; then
    echo "$0 must be run from the top level of the Lucy repository"
    exit 1
fi
export TOPLEVEL=`pwd`

echo "==============================="
echo "Test all host bindings for Lucy"
echo "==============================="
echo

# Exit status codes indicating that the target has built and tested
# successfully.
LUCY_C_STATUS=1
LUCY_PERL_STATUS=1

# Make a shallow clone of the clownfish repo.
if [ -z "$CLOWNFISH" ]; then
    export CLOWNFISH=https://git-wip-us.apache.org/repos/asf/lucy-clownfish.git
fi
git clone --depth=1 $CLOWNFISH clownfish
if [ $? -ne 0 ]; then
    echo FATAL: failed to clone Clownfish repository.
    exit 1
fi
source $TOPLEVEL/clownfish/devel/bin/setup_env.sh
export CFLOG=$TOPLEVEL/clownfish.build.log
rm -f $CFLOG

# C bindings
echo "Building and testing Clownfish dependencies for Lucy C bindings..."
cd $TOPLEVEL/clownfish/compiler/c
./configure >> $CFLOG && make -j >> $CFLOG && make -j test >> $CFLOG
CFC_C_STATUS=$?
cd $TOPLEVEL/clownfish/runtime/c
./configure >> $CFLOG && make -j >> $CFLOG && make -j test >> $CFLOG
CFRUNTIME_C_STATUS=$?
if [ $CFC_C_STATUS -ne 0 ] || [ $CFRUNTIME_C_STATUS -ne 0 ]; then
    cat $CFLOG
    echo "Clownfish C build failed -- skipping Lucy C bindings."
else
    echo "Clownfish C build succeeded."
    cd $TOPLEVEL/c
    ./configure && make -j && make -j test
    LUCY_C_STATUS=$?
    make distclean
    cd $TOPLEVEL/clownfish/runtime/c
    make distclean
fi

## Perl bindings
cd $TOPLEVEL
echo "Building and testing Clownfish dependencies for Lucy Perl bindings..."
rm -f $CFLOG
mkdir perl5lib
cd $TOPLEVEL/clownfish/compiler/perl
perl Build.PL --install_base=$TOPLEVEL/perl5lib >> $CFLOG \
 && ./Build test >> $CFLOG \
 && ./Build install >> $CFLOG
./Build realclean >> $CFLOG
cd $TOPLEVEL/clownfish/runtime/perl
perl Build.PL --install_base=$TOPLEVEL/perl5lib >> $CFLOG \
 && ./Build test >> $CFLOG \
 && ./Build install >> $CFLOG
CFRESULT=$?
./Build realclean >> $CFLOG
if [ $CFRESULT -ne 0 ]; then
    cat $CFLOG
    echo "Clownfish Perl build failed -- skipping Lucy Perl bindings."
else
    echo "Clownfish Perl build succeeded."
    cd $TOPLEVEL/perl
    export PERL5LIB=$TOPLEVEL/perl5lib/lib/perl5
    perl Build.PL
    ./Build test
    LUCY_PERL_STATUS=$?
    ./Build realclean
fi
rm -rf $TOPLEVEL/perl5lib

# Clean up.
rm -rf $TOPLEVEL/clownfish
rm -f $CFLOG

# Print report and exit.
RESULT=0
function check_target {
    if [ $2 == 0 ]; then
        echo "Success:" $1
    else
        echo "FAIL:   " $1
        RESULT=1
    fi
}
echo "==============================="
echo "         Test results          "
echo "==============================="
check_target "c" $LUCY_C_STATUS
check_target "perl" $LUCY_PERL_STATUS
exit $RESULT


