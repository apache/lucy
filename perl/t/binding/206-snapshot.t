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

use strict;
use warnings;
use Test::More tests => 4;
use Lucy::Test;

my $folder   = Lucy::Store::RAMFolder->new;
my $snapshot = Lucy::Index::Snapshot->new;
$snapshot->add_entry("foo");
$snapshot->add_entry("bar");
ok( $snapshot->delete_entry("bar"), "delete_entry" );
is_deeply( $snapshot->list, ['foo'], "add_entry, list" );
$snapshot->write_file( folder => $folder );
is( $snapshot->read_file( folder => $folder ),
    $snapshot, "write_file, read_file" );
$snapshot->set_path("snapfile");
is( $snapshot->get_path, "snapfile", "set_path, get_path" );

