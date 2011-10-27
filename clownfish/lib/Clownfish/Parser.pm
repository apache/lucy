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

package Clownfish::Parser;

use Clownfish::Parcel;
use Clownfish::Type;
use Clownfish::Variable;
use Clownfish::DocuComment;
use Clownfish::Function;
use Clownfish::Method;
use Clownfish::Class;
use Clownfish::CBlock;
use Clownfish::File;
use Carp;

1;

__END__

__POD__

=head1 NAME

Clownfish::Parser - Parse Clownfish header files.

=head1 SYNOPSIS

     my $class_def = $parser->class($class_text);

=head1 DESCRIPTION

Clownfish::Parser is a combined lexer/parser which parses Clownfish header
files.  It is not at all strict, as it relies heavily on the C parser to pick
up errors such as misspelled type names.

=head1 METHODS

=head2 new

Constructor, takes no arguments.

=cut

