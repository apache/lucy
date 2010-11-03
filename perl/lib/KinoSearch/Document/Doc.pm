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

package KinoSearch::Document::Doc;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch     PACKAGE = KinoSearch::Document::Doc

SV*
get_fields(self, ...)
    lucy_Doc *self;
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = newRV_inc( (SV*)Lucy_Doc_Get_Fields(self) );
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    my $doc = Lucy::Document::Doc->new(
        fields => { foo => 'foo foo', bar => 'bar bar' },
    );
    $indexer->add_doc($doc);

Doc objects allow access to field values via hashref overloading:

    $doc->{foo} = 'new value for field "foo"';
    print "foo: $doc->{foo}\n";
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $doc = Lucy::Document::Doc->new(
        fields => { foo => 'foo foo', bar => 'bar bar' },
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Document::Doc",
    xs_code           => $xs_code,
    bind_constructors => ['new'],
    bind_methods      => [qw( Set_Doc_ID Get_Doc_ID Set_Fields )],
    make_pod          => {
        methods     => [qw( set_doc_id get_doc_id get_fields )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


