package KinoSearch::Plan::Float64Type;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $schema       = KinoSearch::Plan::Schema->new;
    my $float64_type = KinoSearch::Plan::FloatType->new;
    $schema->spec_field( name => 'intensity', type => $float64_type );
END_SYNOPSIS
my $constructor = <<'END_CONSTRUCTOR';
    my $float64_type = KinoSearch::Plan::Float64Type->new(
        indexed  => 0     # default true
        stored   => 0,    # default true
        sortable => 1,    # default false
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Plan::Float64Type",
    bind_constructors => ["new|init2"],
    #make_pod          => {
    #    synopsis    => $synopsis,
    #    constructor => { sample => $constructor },
    #},
);


