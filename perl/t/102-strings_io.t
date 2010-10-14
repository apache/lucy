use strict;
use warnings;

use Test::More tests => 3;
use KinoSearch::Test;

my ( @items, $packed, $template, $buf, $file, $out, $in, $correct );

$file = KinoSearch::Store::RAMFile->new;
$out = KinoSearch::Store::OutStream->open( file => $file )
    or die KinoSearch->error;
$out->write_c64(10000);
$out->close;
$in = KinoSearch::Store::InStream->open( file => $file )
    or die KinoSearch->error;
$in->read_raw_c64($buf);
$correct = $file->get_contents;
is( $buf, $correct, "read_raw_c64" );

$file = KinoSearch::Store::RAMFile->new;
$out = KinoSearch::Store::OutStream->open( file => $file )
    or die KinoSearch->error;
$out->print("mint");
$out->close;
$buf = "funny";
$in = KinoSearch::Store::InStream->open( file => $file )
    or die KinoSearch->error;
$in->read( $buf, 1 );
is( $buf, "munny", 'read' );

$file = KinoSearch::Store::RAMFile->new;
$out = KinoSearch::Store::OutStream->open( file => $file )
    or die KinoSearch->error;
$out->print("cute");
$out->close;
$in = KinoSearch::Store::InStream->open( file => $file )
    or die KinoSearch->error;
$buf = "buzz";
$in->read( $buf, 3, 4 );
is( $buf, "buzzcut", 'read with offset' );
