package KinoSearch::Index::Posting::MatchPosting;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # MatchPosting is used indirectly, by specifying in FieldType subclass.
    package MySchema::Category;
    use base qw( KinoSearch::Plan::FullTextType );
    sub posting {
        my $self = shift;
        return KinoSearch::Index::Posting::MatchPosting->new(@_);
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::Posting::MatchPosting",
    bind_constructors => ["new"],
    bind_methods      => [qw( Get_Freq )],
#    make_pod => {
#        synopsis => $synopsis,
#    }
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

