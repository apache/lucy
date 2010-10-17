package KinoSearch::Plan::StringType;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $type   = KinoSearch::Plan::StringType->new;
    my $schema = KinoSearch::Plan::Schema->new;
    $schema->spec_field( name => 'category', type => $type );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $type = KinoSearch::Plan::StringType->new(
        boost    => 0.1,    # default: 1.0
        indexed  => 1,      # default: true
        stored   => 1,      # default: true
        sortable => 1,      # default: false
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Plan::StringType",
    bind_constructors => ["new|init2"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


