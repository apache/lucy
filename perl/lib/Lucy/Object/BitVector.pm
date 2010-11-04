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

package Lucy::Object::BitVector;
use Lucy;

1;

__END__

__BINDING__

my $synopsis    = <<'END_SYNOPSIS';
    my $bit_vec = Lucy::Object::BitVector->new( capacity => 8 );
    my $other   = Lucy::Object::BitVector->new( capacity => 8 );
    $bit_vec->set($_) for ( 0, 2, 4, 6 );
    $other->set($_)   for ( 1, 3, 5, 7 );
    $bit_vec->or($other);
    print "$_\n" for @{ $bit_vec->to_array };    # prints 0 through 7.
END_SYNOPSIS
my $constructor = <<'END_CONSTRUCTOR';
    my $bit_vec = Lucy::Object::BitVector->new( 
        capacity => $doc_max + 1,   # default 0,
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "Lucy",
    class_name   => "Lucy::Object::BitVector",
    bind_methods => [
        qw( Get
            Set
            Clear
            Clear_All
            And
            Or
            And_Not
            Xor
            Flip
            Flip_Block
            Next_Hit
            To_Array
            Grow
            Count
            Get_Capacity
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw( get
                set
                clear
                clear_all
                and
                or
                and_not
                xor
                flip
                flip_block
                next_hit
                to_array
                grow
                count
                )
        ],
    }
);


