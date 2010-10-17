package KinoSearch::Plan::FieldType;
use KinoSearch;

1;

__END__

__BINDING__

my $synopis = <<'END_SYNOPSIS';

    my @sortable;
    for my $field ( @{ $schema->all_fields } ) {
        my $type = $schema->fetch_type($field);
        next unless $type->sortable;
        push @sortable, $field;
    }

END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Plan::FieldType",
    bind_methods => [
        qw(
            Get_Boost
            Indexed
            Stored
            Sortable
            Binary
            Compare_Values
            )
    ],
    bind_constructors => ["new|init2"],
    make_pod          => {
        synopsis => $synopis,
        methods  => [
            qw(
                get_boost
                indexed
                stored
                sortable
                binary
                )
        ],
    }
);


