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

set -e

usage() {
    cat <<EOF

Run this script from a directory containing the Lucy Git repository in
subdir "lucy" and the Clownfish repo in subdir "clownfish". You can fetch
the repos with:

git clone https://git-wip-us.apache.org/repos/asf/lucy-clownfish.git \\
    clownfish
git clone https://git-wip-us.apache.org/repos/asf/lucy.git
EOF
}

root="$(pwd)"
cfish_dir="$root/clownfish"
lucy_dir="$root/lucy"
tmp_dir="$lucy_dir/test_tmp"

if [ ! -f "$cfish_dir/runtime/core/Clownfish.cfp" ]; then
    echo "Clownfish not found in $cfish_dir"
    usage
    exit 1
fi

if [ ! -f "$lucy_dir/core/Lucy.cfp" ]; then
    echo "Lucy not found in $lucy_dir"
    usage
    exit 1
fi

set -x

rm -rf "$tmp_dir"

if [ -z "$1" -o "$1" = go ]; then
    if [ -z "$GOPATH" ]; then
        export GOPATH="$tmp_dir/go"
    else
        export GOPATH="$tmp_dir/go:$GOPATH"
    fi
    mkdir -p "$tmp_dir/go/src/git-wip-us.apache.org/repos/asf"
    ln -s "$cfish_dir" \
        "$tmp_dir/go/src/git-wip-us.apache.org/repos/asf/lucy-clownfish.git"
    ln -s "$lucy_dir" \
        "$tmp_dir/go/src/git-wip-us.apache.org/repos/asf/lucy.git"

    cd "$cfish_dir/compiler/go"
    go run build.go test
    go run build.go install

    cd ../../runtime/go
    go run build.go test
    go run build.go install

    cd "$lucy_dir/go"
    go run build.go test

    go run build.go clean
    cd "$cfish_dir/runtime/go"
    go run build.go clean
    cd "$cfish_dir/compiler/go"
    go run build.go clean

    cd "$root"
fi

if [ -z "$1" -o "$1" = perl ]; then
    export PERL5LIB="$tmp_dir/perl/lib/perl5:$PERL5LIB"

    cd "$cfish_dir/compiler/perl"
    perl Build.PL
    ./Build test
    ./Build install --install-base "$tmp_dir/perl"

    cd ../../runtime/perl
    perl Build.PL
    ./Build test
    ./Build install --install-base "$tmp_dir/perl"
    ./Build realclean

    cd "$lucy_dir/perl"
    perl Build.PL
    ./Build test
    ./Build realclean

    cd "$root"
fi

if [ -z "$1" -o "$1" = c ]; then
    cd "$cfish_dir/compiler/c"
    ./configure --prefix="$tmp_dir/c"
    make -j test
    make install

    cd ../../runtime/c
    ./configure --prefix="$tmp_dir/c"
    make -j test
    make install
    make distclean

    cd "$lucy_dir/c"
    ./configure --prefix="$tmp_dir/c" --clownfish-prefix "$tmp_dir/c"
    make -j test
    make install
    make distclean

    cd "$root"
fi

rm -rf "$tmp_dir"

