use strict;
use warnings;
use 5.008003;
use Module::Build;

use lib 'charmonizer/interpreters/perl';
use Charmonizer;

my $charmonizer = Charmonizer->new(
    modules => [
        'charmonizer/modules/start.charm',
        'charmonizer/modules/integers.charm',
        'charmonizer/modules/end.charm',
    ],
    out_path => 'lucyport.h'
);

$charmonizer->run;

# TODO: Write Lucy. :)  Then we can install it.

__END__


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

