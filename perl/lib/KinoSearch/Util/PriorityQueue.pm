package KinoSearch::Util::PriorityQueue;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Util::PriorityQueue",
    bind_methods => [
        qw(
            Less_Than
            Insert
            Pop
            Pop_All
            Peek
            Get_Size
            )
    ],
    bind_constructors => ["new"],
);


