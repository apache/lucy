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

package KinoSearch::Analysis::Stopalizer;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $stopalizer = Lucy::Analysis::Stopalizer->new(
        language => 'fr',
    );
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $case_folder, $tokenizer, $stopalizer, $stemmer ],
    );

This class uses L<Lingua::StopWords> for its default stoplists, so it supports
the same set of languages.
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $stopalizer = Lucy::Analysis::Stopalizer->new(
        language => 'de',
    );
    
    # or...
    my $stopalizer = Lucy::Analysis::Stopalizer->new(
        stoplist => \%stoplist,
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::Stopalizer",
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor }
    },
);


