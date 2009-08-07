use strict;
use warnings;

use Test::More tests => 14;
use Boilerplater::Util qw(
    slurp_file
    current
    verify_args
    strip_c_comments
    a_isa_b
);

my $foo_txt = 'foo.txt';
unlink $foo_txt;
open( my $fh, '>', $foo_txt ) or die "Can't open '$foo_txt': $!";
print $fh "foo";
close $fh or die "Can't close '$foo_txt': $!";
is( slurp_file($foo_txt), "foo", "slurp_file" );

ok( current( $foo_txt, $foo_txt ), "current" );
ok( !current( $foo_txt, 't' ), "not current" );
ok( !current( 'foo.txt', "nonexistent_file" ),
    "not current when dest file mising"
);

unlink $foo_txt;

my $comment    = "/* I have nothing to say to you, world. */\n";
my $no_comment = "\n";
is( strip_c_comments($comment), $no_comment, "strip_c_comments" );

my %defaults = ( foo => undef );
sub test_verify_args { return verify_args( \%defaults, @_ ) }

ok( test_verify_args( foo => 'foofoo' ), "args verified" );
ok( !test_verify_args('foo'), "odd args fail verification" );
like( $@, qr/odd/, 'set $@ on odd arg failure' );
ok( !test_verify_args( bar => 'nope' ), "bad param doesn't verify" );
like( $@, qr/param/, 'set $@ on invalid param failure' );

my $foo = bless {}, 'Foo';
ok( a_isa_b( $foo, 'Foo' ), "a_isa_b true" );
ok( !a_isa_b( $foo,  'Bar' ), "a_isa_b false" );
ok( !a_isa_b( 'Foo', 'Foo' ), "a_isa_b not blessed" );
ok( !a_isa_b( undef, 'Foo' ), "a_isa_b undef" );

