package KinoSearch::Search::LeafQuery;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    package MyQueryParser;
    use base qw( KinoSearch::Search::QueryParser );

    sub expand_leaf {
        my ( $self, $leaf_query ) = @_;
        if ( $leaf_query->get_text =~ /.\*\s*$/ ) {
            return PrefixQuery->new(
                query_string => $leaf_query->get_text,
                field        => $leaf_query->get_field,
            );
        }
        else {
            return $self->SUPER::expand_leaf($leaf_query);
        }
    }
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $leaf_query = KinoSearch::Search::LeafQuery->new(
        text  => '"three blind mice"',    # required
        field => 'content',               # default: undef
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::LeafQuery",
    bind_methods      => [qw( Get_Field Get_Text )],
    bind_constructors => ["new"],
    make_pod          => {
        methods     => [qw( get_field get_text )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


