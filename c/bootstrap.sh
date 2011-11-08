#! /bin/sh

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


echo "$0 - for initialization of liblucy build environemnt"
echo "     not needed for user building"

#add --include-deps if you want to bootstrap with any other compiler than gcc
#automake --add-missing --copy --include-deps

set -x

if [ ! $SVNDIR ]; then
    SVNDIR=.
fi

# Mac OS X 10.6 no longer has libtoolize but glibtoolize instead
LIBTOOLIZE=`which libtoolize`
if [ ! $LIBTOOLIZE ]; then 
    LIBTOOLIZE=`which glibtoolize`
fi

#echo "libtoolize = $LIBTOOLIZE"

aclocal \
 && $LIBTOOLIZE --force --copy \
 && automake --add-missing --include-deps --copy --foreign \
 && autoconf \
 && rm -f config.cache

