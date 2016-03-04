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

# Exit if any command returns non-zero.
set -e

# Print all commands before executing.
set -x

test_c() {
    # Install Clownfish.
    git clone -q -b 0.5 --depth 1 https://git-wip-us.apache.org/repos/asf/lucy-clownfish.git
    cd lucy-clownfish/runtime/c
    ./configure
    make -j
    sudo ./install.sh --prefix /usr/local
    sudo ldconfig

    cd ../../../c
    ./configure
    make -j test
}

test_perl() {
    perlbrew switch $PERL_VERSION

    # Install Clownfish.
    git clone -q -b 0.5 --depth 1 https://git-wip-us.apache.org/repos/asf/lucy-clownfish.git
    cd lucy-clownfish/runtime/perl
    perl Build.PL
    ./Build
    sudo ./Build install
    cd ../../compiler/perl
    sudo ./Build install

    cd ../../../perl
    perl Build.PL
    ./Build test
}

test_go() {
    # TODO
    exit 1
}

case $CLOWNFISH_HOST in
    perl)
        test_perl
        ;;
    c)
        test_c
        ;;
    go)
        test_go
        ;;
    *)
        echo "unknown CLOWNFISH_HOST: $CLOWNFISH_HOST"
        exit 1
esac

