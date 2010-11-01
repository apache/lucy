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

package KinoSearch::Analysis::Tokenizer;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $whitespace_tokenizer
        = KinoSearch::Analysis::Tokenizer->new( pattern => '\S+' );

    # or...
    my $word_char_tokenizer
        = KinoSearch::Analysis::Tokenizer->new( pattern => '\w+' );

    # or...
    my $apostrophising_tokenizer = KinoSearch::Analysis::Tokenizer->new;

    # Then... once you have a tokenizer, put it into a PolyAnalyzer:
    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        analyzers => [ $case_folder, $word_char_tokenizer, $stemmer ], );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $word_char_tokenizer = KinoSearch::Analysis::Tokenizer->new(
        pattern => '\w+',    # required
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::Tokenizer",
    bind_methods      => [qw( Set_Token_RE )],
    bind_constructors => ["_new"],
    make_pod          => {
        constructor => { sample => $constructor },
        synopsis    => $synopsis,
    },
);


