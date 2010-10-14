use strict;
use warnings;

use Test::More tests => 11;
use KinoSearch::Test;

my $tinfo = KinoSearch::Index::TermInfo->new( doc_freq => 10, );
$tinfo->set_post_filepos(20);
$tinfo->set_skip_filepos(40);
$tinfo->set_lex_filepos(50);

my $cloned_tinfo = $tinfo->clone;
ok( !$tinfo->equals($cloned_tinfo),
    "the clone should be a separate C struct" );

is( $tinfo->get_doc_freq,     10, "new sets doc_freq correctly" );
is( $tinfo->get_doc_freq,     10, "... doc_freq cloned" );
is( $tinfo->get_post_filepos, 20, "... post_filepos cloned" );
is( $tinfo->get_skip_filepos, 40, "... skip_filepos cloned" );
is( $tinfo->get_lex_filepos,  50, "... lex_filepos cloned" );

$tinfo->set_doc_freq(5);
is( $tinfo->get_doc_freq,        5,  "set/get doc_freq" );
is( $cloned_tinfo->get_doc_freq, 10, "setting orig doesn't affect clone" );

$tinfo->set_post_filepos(15);
is( $tinfo->get_post_filepos, 15, "set/get post_filepos" );

$tinfo->set_skip_filepos(35);
is( $tinfo->get_skip_filepos, 35, "set/get skip_filepos" );

$tinfo->set_lex_filepos(45);
is( $tinfo->get_lex_filepos, 45, "set/get lex_filepos" );
