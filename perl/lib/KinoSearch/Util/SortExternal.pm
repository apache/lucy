package KinoSearch::Util::SortExternal;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch    PACKAGE = KinoSearch::Util::SortExternal

IV
_DEFAULT_MEM_THRESHOLD()
CODE:
    RETVAL = KINO_SORTEX_DEFAULT_MEM_THRESHOLD;
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Util::SortExternal",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Flush
            Flip
            Add_Run
            Refill
            Sort_Cache
            Cache_Count
            Clear_Cache
            Set_Mem_Thresh
            )
    ],
);


