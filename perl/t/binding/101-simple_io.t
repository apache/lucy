use strict;
use warnings;
use lib 'buildlib';

use Lucy;
use Test::More tests => 27;
use Encode qw( _utf8_off );
use bytes;
no bytes;

my ( @nums, $packed, $template, $ram_file );

sub check_round_trip {
    my ( $type, $expected ) = @_;
    my $write_method = "write_$type";
    my $read_method  = "read_$type";
    my $file         = Lucy::Store::RAMFile->new;
    my $outstream    = Lucy::Store::OutStream->open( file => $file )
        or die Lucy->error;
    $outstream->$write_method($_) for @$expected;
    $outstream->close;
    my $instream = Lucy::Store::InStream->open( file => $file )
        or die Lucy->error;
    my @got;
    push @got, $instream->$read_method for @$expected;
    is_deeply( \@got, $expected, $type );
    return $file;
}

sub check_round_trip_bytes {
    my ( $message, $expected ) = @_;
    my $file = Lucy::Store::RAMFile->new;
    my $outstream = Lucy::Store::OutStream->open( file => $file );
    for (@$expected) {
        $outstream->write_c32( bytes::length($_) );
        $outstream->print($_);
    }
    $outstream->close;
    my $instream = Lucy::Store::InStream->open( file => $file )
        or die Lucy->error;
    my @got;
    for (@$expected) {
        my $buf;
        my $len = $instream->read_c32;
        $instream->read( $buf, $len );
        push @got, $buf;
    }
    is_deeply( \@got, $expected, $message );
    return $file;
}

@nums     = ( -128 .. 127 );
$packed   = pack( 'c256', @nums );
$ram_file = check_round_trip( 'i8', \@nums );
is( $ram_file->get_contents, $packed,
    "pack and write_i8 handle signed bytes identically" );

@nums     = ( 0 .. 255 );
$packed   = pack( 'C*', @nums );
$ram_file = check_round_trip( 'u8', \@nums );
is( $ram_file->get_contents, $packed,
    "pack and write_u8 handle unsigned bytes identically" );

@nums = map { $_ * 1_000_000 + int( rand() * 1_000_000 ) } -1000 .. 1000;
push @nums, ( -1 * ( 2**31 ), 2**31 - 1 );
check_round_trip( 'i32', \@nums );

@nums = map { $_ * 1_000_000 + int( rand() * 1_000_000 ) } 1000 .. 3000;
push @nums, ( 0, 1, 2**32 - 1 );
$packed = pack( 'N*', @nums );
$ram_file = check_round_trip( 'u32', \@nums );
is( $ram_file->get_contents, $packed,
    "pack and write_u32 handle unsigned int32s identically" );

@nums = map { $_ * 2 } 0 .. 5;
check_round_trip( 'u64', \@nums );

@nums = map { $_ * 2**31 } 0 .. 2000;
$_ += int( rand( 2**16 ) ) for @nums;
check_round_trip( 'u64', \@nums );

@nums = ( 0 .. 127 );
check_round_trip( 'c32', \@nums );

@nums     = ( 128 .. 500 );
$packed   = pack( 'w*', @nums );
$ram_file = check_round_trip( 'c32', \@nums );
is( $ram_file->get_contents, $packed, "C32 is equivalent to Perl's pack w" );

@nums = ( 0 .. 127 );
check_round_trip( 'c64', \@nums );

@nums     = ( 128 .. 500 );
$packed   = pack( 'w*', @nums );
$ram_file = check_round_trip( 'c64', \@nums );
is( $ram_file->get_contents, $packed, "C64 is equivalent to Perl's pack w" );

@nums = map { $_ * 2**31 } 0 .. 2000;
$_ += int( rand( 2**16 ) ) for @nums;
check_round_trip( 'c64', \@nums );

# rand (always?) has 64-bit precision, but we need 32-bit - so truncate via
# pack/unpack.
@nums = map {rand} 0 .. 100;
$packed = pack( 'f*', @nums );
@nums = unpack( 'f*', $packed );
check_round_trip( 'f32', \@nums );

@nums = map {rand} 0 .. 100;
check_round_trip( 'f64', \@nums );

my @items;
for ( 0, 22, 300 ) {
    @items = ( 'a' x $_ );
    check_round_trip_bytes( "buf of length $_", \@items );
    check_round_trip( 'string', \@items );
}

{
    my @stuff = ( qw( a b c d 1 ), "\n", "\0", " ", " ", "\xf0\x9d\x84\x9e" );
    my @items = ();
    for ( 1 .. 50 ) {
        my $string_len = int( rand() * 5 );
        my $str        = '';
        $str .= $stuff[ rand @stuff ] for 1 .. $string_len;
        push @items, $str;
    }
    check_round_trip_bytes( "50 binary bufs", \@items );
}

my $latin = "ma\x{f1}ana";
$ram_file = check_round_trip( "string", [$latin] );
my $unibytes = $latin;
utf8::upgrade($unibytes);
_utf8_off($unibytes);
my $slurped = $ram_file->get_contents;
substr( $slurped, 0, 1, "" );    # ditch c32 at head of string;
is( $slurped, $unibytes, "write_string upgrades to utf8" );

