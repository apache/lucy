package KinoSearch::Index::Posting::RichPosting;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # RichPosting is used indirectly, by specifying in FieldType subclass.
    package MySchema::Category;
    use base qw( KinoSearch::Plan::FullTextType );
    sub posting {
        my $self = shift;
        return KinoSearch::Index::Posting::RichPosting->new(@_);
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::Posting::RichPosting",
    bind_constructors => ["new"],
#    make_pod => {
#        synopsis => $synopsis,
#    }
);


