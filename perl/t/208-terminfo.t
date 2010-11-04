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

use Test::More tests => 11;
use Lucy::Test;

my $tinfo = Lucy::Index::TermInfo->new( doc_freq => 10, );
$tinfo->set_post_filepos(20);
$tinfo->set_skip_filepos(40);
$tinfo->set_lex_filepos(50);

my $cloned_tinfo = $tinfo->clone;
ok( !$tinfo->equals($cloned_tinfo),
    "the clone should be a separate C struct" );

is( $tinfo->get_doc_freq,     10, "new sets doc_freq correctly" );
is( $tinfo->get_doc_freq,     10, "... doc_freq cloned" );
is( $tinfo->get_post_filepos, 20, "... post_filepos cloned" );
is( $tinfo->get_skip_filepos, 40, "... skip_filepos cloned" );
is( $tinfo->get_lex_filepos,  50, "... lex_filepos cloned" );

$tinfo->set_doc_freq(5);
is( $tinfo->get_doc_freq,        5,  "set/get doc_freq" );
is( $cloned_tinfo->get_doc_freq, 10, "setting orig doesn't affect clone" );

$tinfo->set_post_filepos(15);
is( $tinfo->get_post_filepos, 15, "set/get post_filepos" );

$tinfo->set_skip_filepos(35);
is( $tinfo->get_skip_filepos, 35, "set/get skip_filepos" );

$tinfo->set_lex_filepos(45);
is( $tinfo->get_lex_filepos, 45, "set/get lex_filepos" );
