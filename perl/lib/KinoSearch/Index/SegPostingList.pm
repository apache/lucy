package KinoSearch::Index::SegPostingList;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::SegPostingList",
    bind_methods      => [qw( Get_Post_Stream Get_Count )],
    bind_constructors => ["new"],
);


