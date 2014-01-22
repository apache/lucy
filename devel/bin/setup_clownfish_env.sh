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

# Set up PATH, LIBRARY_PATH, LD_LIBRARY_PATH and CLOWNFISH_INCLUDE to build
# Lucy with an uninstalled Clownfish source tree. Useful for development.
#
# Usage: . setup_clownfish_env.sh path_to_clownfish_source

contains() {
    string="$1"
    substring="$2"
    test "${string#*$substring}" != "$string"
}

add_to_path() {
    path="$1"
    dir="$2"
    if [ -z "$path" ]; then
        echo "$dir"
    elif ! contains ":$path:" ":$dir:"; then
        echo "$dir:$path"
    else
        echo "$path"
    fi
}

if [ -z "$1" ]; then
    echo "Usage: . setup_clownfish_env.sh path_to_clownfish_source"
    return 1 2>/dev/null || exit 1
fi

if [ ! -d "$1" ]; then
    echo "Not a directory: $1"
    return 1 2>/dev/null || exit 1
fi

src_dir=`cd "$1" && pwd`

if [ ! -d "$src_dir/compiler" ] || [ ! -d "$src_dir/runtime" ]
then
    echo "Doesn't look like a Clownfish source directory: $src_dir"
    return 1 2>/dev/null || exit 1
fi

compiler_dir=$src_dir/compiler/c
runtime_dir=$src_dir/runtime/c
core_dir=$src_dir/runtime/core

export PATH=`add_to_path "$PATH" "$compiler_dir"`
export LIBRARY_PATH=`add_to_path "$LIBRARY_PATH" "$runtime_dir"`
export CLOWNFISH_INCLUDE=`add_to_path "$CLOWNFISH_INCLUDE" "$core_dir"`

if [ `uname` != Darwin ]; then
    export LD_LIBRARY_PATH=`add_to_path "$LD_LIBRARY_PATH" "$runtime_dir"`
fi

