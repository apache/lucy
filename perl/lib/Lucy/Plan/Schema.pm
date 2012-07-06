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

package Lucy::Plan::Schema;
use Lucy;
our $VERSION = '0.003002';
$VERSION = eval $VERSION;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use Lucy::Plan::Schema;
    use Lucy::Plan::FullTextType;
    use Lucy::Analysis::PolyAnalyzer;
    
    my $schema = Lucy::Plan::Schema->new;
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new( 
        language => 'en',
    );
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => $polyanalyzer,
    );
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $schema = Lucy::Plan::Schema->new;
END_CONSTRUCTOR

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Plan::Schema",
    bind_methods => [
        qw(
            Architecture
            Get_Architecture
            Get_Similarity
            Fetch_Type
            Fetch_Analyzer
            Fetch_Sim
            Num_Fields
            All_Fields
            Spec_Field
            Write
            Eat
            )
    ],
    bind_constructors => [qw( new )],
    make_pod          => {
        methods => [
            qw(
                spec_field
                num_fields
                all_fields
                fetch_type
                fetch_sim
                architecture
                get_architecture
                get_similarity
                )
        ],
        synopsis     => $synopsis,
        constructors => [ { sample => $constructor } ],
    },
);


