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

package Lucy::Test::USConSchema;
use base 'Lucy::Plan::Schema';
use Lucy::Analysis::PolyAnalyzer;
use Lucy::Plan::FullTextType;
use Lucy::Plan::StringType;

sub new {
    my $self       = shift->SUPER::new(@_);
    my $analyzer   = Lucy::Analysis::PolyAnalyzer->new( language => 'en' );
    my $title_type = Lucy::Plan::FullTextType->new( analyzer => $analyzer, );
    my $content_type = Lucy::Plan::FullTextType->new(
        analyzer      => $analyzer,
        highlightable => 1,
    );
    my $url_type = Lucy::Plan::StringType->new( indexed => 0, );
    my $cat_type = Lucy::Plan::StringType->new;
    $self->spec_field( name => 'title',    type => $title_type );
    $self->spec_field( name => 'content',  type => $content_type );
    $self->spec_field( name => 'url',      type => $url_type );
    $self->spec_field( name => 'category', type => $cat_type );
    return $self;
}

1;
