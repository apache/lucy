package KinoSearch::Index::Posting;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::Posting",
    bind_methods => [qw( Get_Doc_ID )],
#    make_pod => {
#        synopsis => "    # Abstract base class.\n",
#    },
);


