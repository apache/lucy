package KinoSearch::Document::Doc;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch     PACKAGE = KinoSearch::Document::Doc

SV*
get_fields(self, ...)
    kino_Doc *self;
CODE:
    CHY_UNUSED_VAR(items);
    RETVAL = newRV_inc( (SV*)Kino_Doc_Get_Fields(self) );
OUTPUT: RETVAL
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';
    my $doc = KinoSearch::Document::Doc->new(
        fields => { foo => 'foo foo', bar => 'bar bar' },
    );
    $doc->{foo} = 'new value for field "foo"';
    $indexer->add_doc($doc);
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $doc = KinoSearch::Document::Doc->new(
        fields => { foo => 'foo foo', bar => 'bar bar' },
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Document::Doc",
    xs_code           => $xs_code,
    bind_constructors => ['new'],
    bind_methods      => [qw( Set_Doc_ID Get_Doc_ID Set_Fields )],
    make_pod          => {
        methods     => [qw( get_fields )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


