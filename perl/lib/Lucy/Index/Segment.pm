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

package Lucy::Index::Segment;
use Lucy;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # Index-time.
    package MyDataWriter;
    use base qw( Lucy::Index::DataWriter );

    sub finish {
        my $self     = shift;
        my $segment  = $self->get_segment;
        my $metadata = $self->SUPER::metadata();
        $metadata->{foo} = $self->get_foo;
        $segment->store_metadata(
            key       => 'my_component',
            metadata  => $metadata
        );
    }

    # Search-time.
    package MyDataReader;
    use base qw( Lucy::Index::DataReader );

    sub new {
        my $self     = shift->SUPER::new(@_);
        my $segment  = $self->get_segment;
        my $metadata = $segment->fetch_metadata('my_component');
        if ($metadata) {
            $self->set_foo( $metadata->{foo} );
            ...
        }
        return $self;
    }
END_SYNOPSIS

Clownfish::CFC::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Index::Segment",
    bind_methods => [
        qw(
            Add_Field
            _store_metadata|Store_Metadata
            Fetch_Metadata
            Field_Num
            Field_Name
            Get_Name
            Get_Number
            Set_Count
            Get_Count
            Write_File
            Read_File
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [
            qw(
                add_field
                store_metadata
                fetch_metadata
                field_num
                field_name
                get_name
                get_number
                set_count
                get_count
                )
        ],
    },
);


