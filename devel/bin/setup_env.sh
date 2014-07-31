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

# Set up the following environment variables to build a Lucy project with
# an uninstalled Lucy source tree. Useful for development.
#
# - LIBRARY_PATH
# - LD_LIBRARY_PATH
# - CLOWNFISH_INCLUDE
# - PERL5LIB
#
# Usage: source setup_env.sh [path to lucy]

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

if [ -n "$1" ]; then
    base_dir="$1"
elif [ -n "$BASH_SOURCE" ]; then
    # Only works with bash.
    script_dir=`dirname "$BASH_SOURCE"`
    base_dir=`cd "$script_dir/../.." && pwd`
else
    echo "Usage: source setup_env.sh path_to_lucy_source"
    return 1 2>/dev/null || exit 1
fi

if [ ! -d "$base_dir/c" ] || [ ! -d "$base_dir/perl" ]
then
    echo "Doesn't look like a Lucy source directory: $base_dir"
    return 1 2>/dev/null || exit 1
fi

export LIBRARY_PATH=`add_to_path "$LIBRARY_PATH" "$base_dir/c"`
export CLOWNFISH_INCLUDE=`add_to_path "$CLOWNFISH_INCLUDE" "$base_dir/core"`
export PERL5LIB=`add_to_path "$PERL5LIB" "$base_dir/perl/blib/arch"`
export PERL5LIB=`add_to_path "$PERL5LIB" "$base_dir/perl/blib/lib"`

case `uname` in
    MINGW*|CYGWIN*)
        export PATH=`add_to_path "$PATH" "$base_dir/c"`
	;;
    Darwin*)
        ;;
    *)
        export LD_LIBRARY_PATH=`add_to_path "$LD_LIBRARY_PATH" "$base_dir/c"`
esac

