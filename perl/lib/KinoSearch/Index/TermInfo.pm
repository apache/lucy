package KinoSearch::Index::TermInfo;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::TermInfo",
    bind_methods => [
        qw(
            Get_Doc_Freq
            Get_Lex_FilePos
            Get_Post_FilePos
            Get_Skip_FilePos
            Set_Doc_Freq
            Set_Lex_FilePos
            Set_Post_FilePos
            Set_Skip_FilePos
            Reset
            )
    ],
    bind_constructors => ["new"],
);


