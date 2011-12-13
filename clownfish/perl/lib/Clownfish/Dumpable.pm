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

package Clownfish::Dumpable;
use Clownfish;

1;

__END__

__POD__

=head1 NAME

Clownfish::Dumpable - Auto-generate code for "dumpable" classes.

=head1 SYNOPSIS

    my $dumpable = Clownfish::Dumpable->new;
    for my $class ( grep { $_->has_attribute('dumpable') } @classes ) {
        $dumpable->add_dumpables($class);
    }

=head1 DESCRIPTION

If a class declares that it has the attribute "dumpable", but does not declare
either Dump or Load(), Clownfish::Dumpable will attempt to auto-generate
those methods if methods inherited from the parent class do not suffice.

    class Foo::Bar inherits Foo : dumpable {
        Thing *thing;

        public inert incremented Bar*
        new();

        void
        Destroy(Bar *self);
    }

=head1 METHODS

=head2 new

    my $dumpable = Clownfish::Dumpable->new;

Constructor.  Takes no arguments.

=head2 add_dumpables

    $dumpable->add_dumpables($dumpable_class);

Analyze a class with the attribute "dumpable" and add Dump() or Load() methods
as necessary.

=cut
