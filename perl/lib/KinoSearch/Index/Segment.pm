package KinoSearch::Index::Segment;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # Index-time.
    package MyDataWriter;
    use base qw( KinoSearch::Index::DataWriter );

    sub finish {
        my $self     = shift;
        my $segment  = $self->get_segment;
        my $metadata = $self->SUPER::metadata();
        $metadata->{foo} = $self->get_foo;
        $segment->store_metadata(
            key       => 'my_component',
            metadata  => $metadata
        );
    }

    # Search-time.
    package MyDataReader;
    use base qw( KinoSearch::Index::DataReader );

    sub new {
        my $self     = shift->SUPER::new(@_);
        my $segment  = $self->get_segment;
        my $metadata = $segment->fetch_metadata('my_component');
        if ($metadata) {
            $self->set_foo( $metadata->{foo} );
            ...
        }
        return $self;
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::Segment",
    bind_methods => [
        qw(
            Add_Field
            _store_metadata|Store_Metadata
            Fetch_Metadata
            Field_Num
            Field_Name
            Get_Name
            Get_Number
            Set_Count
            Get_Count
            Write_File
            Read_File
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [
            qw(
                add_field
                store_metadata
                fetch_metadata
                field_num
                field_name
                get_name
                get_number
                set_count
                get_count
                )
        ],
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

