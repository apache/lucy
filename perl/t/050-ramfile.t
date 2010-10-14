use strict;
use warnings;

use Test::More tests => 8;
use KinoSearch::Test;

my ( $ram_file, $outstream, $instream, $foo );

$ram_file = KinoSearch::Store::RAMFile->new;
$outstream = KinoSearch::Store::OutStream->open( file => $ram_file )
    or die KinoSearch->error;
$outstream->print("foo");
$outstream->flush;
is( $ram_file->get_contents, "foo", '$ramfile->get_contents' );

my $long_string = 'a' x 5000;
$outstream->print($long_string);
$outstream->flush;

is( $ram_file->get_contents, "foo$long_string",
    "store a string spread out over several buffers" );

$instream = KinoSearch::Store::InStream->open( file => $ram_file )
    or die KinoSearch->error;
$instream->read( $foo, 3 );
is( $foo, 'foo', "instream reads ramfile properly" );

my $long_dupe;
$instream->read( $long_dupe, 5000 );
is( $long_dupe, $long_string, "read string spread out over several buffers" );

eval { my $blah; $instream->read( $blah, 3 ); };
like( $@, qr/EOF/, "reading past EOF throws an error" );

$ram_file = KinoSearch::Store::RAMFile->new;
$outstream = KinoSearch::Store::OutStream->open( file => $ram_file )
    or die KinoSearch->error;
my $BUF_SIZE  = KinoSearch::Store::FileHandle::_BUF_SIZE();
my $rep_count = $BUF_SIZE - 1;
$outstream->print( 'a' x $rep_count );
$outstream->print('foo');
$outstream->close;
$instream = KinoSearch::Store::InStream->open( file => $ram_file )
    or die KinoSearch->error;
$instream->read( $long_dupe, $rep_count );
undef $foo;
$instream->read( $foo, 3 );
is( $foo, 'foo', "read across buffer boundary " );

$ram_file = KinoSearch::Store::RAMFile->new;
$outstream = KinoSearch::Store::OutStream->open( file => $ram_file )
    or die KinoSearch->error;
$outstream->print( 'a' x 1024 );
$outstream->print('foo');
$outstream->close;

$instream = KinoSearch::Store::InStream->open( file => $ram_file )
    or die KinoSearch->error;
$instream->seek(1024);
undef $foo;
$instream->read( $foo, 3 );
is( $foo, 'foo', "InStream seek" );

my $dupe = $instream->reopen(
    filename => 'foo',
    offset   => 1023,
    len      => 4
);
undef $foo;
$dupe->read( $foo, 4 );
is( $foo, 'afoo', "reopened instream" );

