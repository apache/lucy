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

package Clownfish::Util;
use Clownfish;

1;

__END__

__POD__

=head1 NAME

Clownfish::Util - Miscellaneous helper functions.

=head1 DESCRIPTION

Clownfish::Util provides a few convenience functions used internally by
other Clownfish modules.

=head1 FUNCTIONS

=head2 slurp_text

    my $foo_contents = slurp_text('foo.txt');

Open a file, read it in (as text), return its contents.

=head2 current

    compile('foo.c') unless current( 'foo.c', 'foo.o' );

Given two elements, which may be either scalars or arrays, verify that
everything in the second group exists and was created later than anything in
the first group.

=head2 verify_args

    verify_args( \%defaults, @_ ) or confess $@;

Verify that named parameters exist in a defaults hash.  Returns false and sets
$@ if a problem is detected.

=head2 strip_c_comments

    my $c_minus_comments = strip_c_comments($c_source_code);

Quick 'n' dirty stripping of C comments.  Will massacre stuff like comments
embedded in string literals, so watch out.

=head2 write_if_changed

    write_if_changed( $path, $content );

Test whether there's a file at C<$path> which already matches C<$content>
exactly.  If something has changed, write the file.  Otherwise do nothing (and
avoid bumping the file's modification time).

=cut

