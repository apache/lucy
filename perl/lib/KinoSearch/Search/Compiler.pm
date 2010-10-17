package KinoSearch::Search::Compiler;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # (Compiler is an abstract base class.)
    package MyCompiler;
    use base qw( KinoSearch::Search::Compiler );

    sub make_matcher {
        my $self = shift;
        return MyMatcher->new( @_, compiler => $self );
    }
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR_CODE_SAMPLE';
    my $compiler = MyCompiler->SUPER::new(
        parent     => $my_query,
        searcher   => $searcher,
        similarity => $sim,        # default: undef
        boost      => undef,       # default: see below
    );
END_CONSTRUCTOR_CODE_SAMPLE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::Compiler",
    bind_methods => [
        qw(
            Make_Matcher
            Get_Parent
            Get_Similarity
            Get_Weight
            Sum_Of_Squared_Weights
            Apply_Norm_Factor
            Normalize
            Highlight_Spans
            )
    ],
    bind_constructors => ["do_new"],
    make_pod          => {
        methods => [
            qw(
                make_matcher
                get_weight
                sum_of_squared_weights
                apply_norm_factor
                normalize
                get_parent
                get_similarity
                highlight_spans
                )
        ],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


