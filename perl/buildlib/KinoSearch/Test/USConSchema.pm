use strict;
use warnings;

package KinoSearch::Test::USConSchema;
use base 'KinoSearch::Plan::Schema';
use KinoSearch::Analysis::PolyAnalyzer;
use KinoSearch::Plan::FullTextType;
use KinoSearch::Plan::StringType;

sub new {
    my $self = shift->SUPER::new(@_);
    my $analyzer
        = KinoSearch::Analysis::PolyAnalyzer->new( language => 'en' );
    my $title_type
        = KinoSearch::Plan::FullTextType->new( analyzer => $analyzer, );
    my $content_type = KinoSearch::Plan::FullTextType->new(
        analyzer      => $analyzer,
        highlightable => 1,
    );
    my $url_type = KinoSearch::Plan::StringType->new( indexed => 0, );
    my $cat_type = KinoSearch::Plan::StringType->new;
    $self->spec_field( name => 'title',    type => $title_type );
    $self->spec_field( name => 'content',  type => $content_type );
    $self->spec_field( name => 'url',      type => $url_type );
    $self->spec_field( name => 'category', type => $cat_type );
    return $self;
}

1;
