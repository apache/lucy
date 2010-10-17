package KinoSearch::Util::Stepper;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Util::Stepper",
    bind_methods => [qw( Read_Record )],
);


