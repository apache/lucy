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
use File::Spec::Functions qw( catfile catdir );
use Encode qw( encode );
use Text::Wrap qw( wrap );

# Don't use tabs.  Wrap at 78 columns.
$Text::Wrap::unexpand = 0;
$Text::Wrap::columns  = 78;

if ( @ARGV != 2 ) {
    die "Usage: perl update_snowstop.pl SNOWBALL_SVN_CO LUCY_SNOWSTOP_DIR";
}
my ( $snow_co_dir, $dest_dir ) = @ARGV;

# Update to a particular rev of the Snowball repository.
die("Not a directory: '$snow_co_dir'") unless -d $snow_co_dir;
my $retval = system( "svn", "update", "-r", "541", $snow_co_dir );
die "svn update failed" if ( $retval >> 8 );

# Open destination C file and print start of file.
my $outpath = catfile( $dest_dir, 'source', 'snowball_stoplists.c' );
open( my $out_fh, '>', $outpath ) or die "Can't open '$outpath': $!";
print $out_fh <<'END_STUFF';
/* Auto-generated file -- DO NOT EDIT!
 *
 * The words in this file are taken from stoplists provided by the Snowball
 * project.
 */

#include "Lucy/Analysis/SnowballStopFilter.h"

END_STUFF

my %languages = (
    da => "danish",
    de => "german",
    en => "english",
    es => "spanish",
    fi => "finnish",
    fr => "french",
    hu => "hungarian",
    it => "italian",
    nl => "dutch",
    no => "norwegian",
    pt => "portuguese",
    ru => "russian",
    sv => "swedish",
);

for my $iso ( sort keys %languages ) {
    my $language = $languages{$iso};

    # Grab stoplists from Snowball source files.
    my $stop_path = "$snow_co_dir/website/algorithms/$language/stop.txt";
    my $source_enc = $iso eq 'ru' ? 'koi8-r' : 'iso-8859-1';
    open( my $stopfile_fh, "<:encoding($source_enc)", $stop_path )
        or die "Couldn't open file '$stop_path': $!";
    my @words;
    while ( defined( my $line = <$stopfile_fh> ) ) {
        $line =~ s/\|.*//g;
        next unless length($line);
        push @words, split( /\s+/, $line );
    }

    # Encode as UTF-8, change all non-ASCII bytes to octal escapes, and format
    # as C string literals.
    my @escaped = map { '"' . encode( 'UTF-8', $_ ) . '"' } @words;
    s/([\x80-\xFF])/octal_escape($1)/ge for @escaped;

    # Wrap text and print to outfile.
    my $joined = join( ', ', @escaped, 'NULL' );
    my $wrapped = wrap( '    ', '    ', $joined );
    print $out_fh <<END_STUFF;
static const char *words_${iso}[] = {
$wrapped
};
const uint8_t **lucy_SnowStop_snow_${iso} = (const uint8_t**)words_$iso;

END_STUFF
}

sub octal_escape {
    my $ord = ord( $_[0] );
    return sprintf( "\\%.3o", $ord );
}

