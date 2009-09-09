package Lucy;
use strict;
use warnings;

use 5.008003;

our $VERSION = 0.01;
$VERSION = eval $VERSION;

use XSLoader;
BEGIN { XSLoader::load( 'Lucy', '0.01' ) }

1;

__END__

__BINDING__

my $lucy_xs_code = <<'END_XS_CODE';
MODULE = Lucy    PACKAGE = Lucy 

BOOT:
    lucy_Lucy_bootstrap();

IV
_dummy_function()
CODE:
    RETVAL = 1;
OUTPUT:
    RETVAL
END_XS_CODE

Boilerplater::Binding::Perl::Class->register(
    parcel     => "Lucy",
    class_name => "Lucy",
    xs_code    => $lucy_xs_code,
);

__POD__

=head1 NAME

Lucy - Search engine library.

=head1 SYNOPSIS

To do.

=head1 DESCRIPTION

To do.

=head1 AUTHOR

Marvin Humphrey E<lt>marvin at rectangular dot comE<gt>

=head1 COPYRIGHT AND LICENSE

    /**
     * Copyright 2006 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

