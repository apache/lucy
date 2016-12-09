# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
package Lucy::Build::Binding::Analysis;
use strict;
use warnings;

our $VERSION = '0.006001';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_analyzer;
    $class->bind_casefolder;
    $class->bind_easyanalyzer;
    $class->bind_inversion;
    $class->bind_normalizer;
    $class->bind_polyanalyzer;
    $class->bind_regextokenizer;
    $class->bind_snowballstemmer;
    $class->bind_snowballstopfilter;
    $class->bind_standardtokenizer;
    $class->bind_token;
}

sub bind_analyzer {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $constructor = <<'END_CONSTRUCTOR';
=head2 new

    package MyAnalyzer;
    use base qw( Lucy::Analysis::Analyzer );
    our %foo;
    sub new {
        my $self = shift->SUPER::new;
        my %args = @_;
        $foo{$$self} = $args{foo};
        return $self;
    }

Abstract constructor.  Takes no arguments.
END_CONSTRUCTOR
    $pod_spec->set_synopsis("    # Abstract base class.\n");
    $pod_spec->add_constructor( pod => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::Analyzer",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_casefolder {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $case_folder = Lucy::Analysis::CaseFolder->new;

    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $tokenizer, $case_folder, $stemmer ],
    );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $case_folder = Lucy::Analysis::CaseFolder->new;
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::CaseFolder",
    );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Analysis::CaseFolder');
    $binding->add_class_alias('KinoSearch::Analysis::LCNormalizer');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_easyanalyzer {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $schema = Lucy::Plan::Schema->new;
    my $analyzer = Lucy::Analysis::EasyAnalyzer->new(
        language => 'en',
    );
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => $analyzer,
    );
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $analyzer = Lucy::Analysis::EasyAnalyzer->new(
        language  => 'es',
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::EasyAnalyzer",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_inversion {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $result = Lucy::Analysis::Inversion->new;

    while (my $token = $inversion->next) {
        $result->append($token);
    }
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $inversion = Lucy::Analysis::Inversion->new(
        $seed,  # optional
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $xs = <<'END_XS';
MODULE = Lucy   PACKAGE = Lucy::Analysis::Inversion

SV*
new(...)
CODE:
{
    static const XSBind_ParamSpec param_specs[1] = {
        XSBIND_PARAM("text", false)
    };
    int32_t     locations[1];
    SV         *text_sv       = NULL;
    lucy_Token *starter_token = NULL;

    XSBind_locate_args(aTHX_ &ST(0), 1, items, param_specs, locations, 1);

    text_sv = locations[0] < items ? ST(locations[0]) : NULL;
    if (XSBind_sv_defined(aTHX_ text_sv)) {
        STRLEN len;
        char *text = SvPVutf8(text_sv, len);
        STRLEN length = utf8_length((U8*)text, (U8*)text + len);
        starter_token = lucy_Token_new(text, len, 0, length, 1.0, 1);
    }

    RETVAL = CFISH_OBJ_TO_SV_NOINC(lucy_Inversion_new(starter_token));
    CFISH_DECREF(starter_token);
}
OUTPUT: RETVAL
END_XS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::Inversion",
    );
    $binding->set_pod_spec($pod_spec);
    $binding->append_xs($xs);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_normalizer {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $normalizer = Lucy::Analysis::Normalizer->new;
    
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $tokenizer, $normalizer, $stemmer ],
    );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $normalizer = Lucy::Analysis::Normalizer->new(
        normalization_form => 'NFKC',
        case_fold          => 1,
        strip_accents      => 0,
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::Normalizer",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_polyanalyzer {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $schema = Lucy::Plan::Schema->new;
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new( 
        analyzers => \@analyzers,
    );
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => $polyanalyzer,
    );
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $tokenizer    = Lucy::Analysis::StandardTokenizer->new;
    my $normalizer   = Lucy::Analysis::Normalizer->new;
    my $stemmer      = Lucy::Analysis::SnowballStemmer->new( language => 'en' );
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $tokenizer, $normalizer, $stemmer, ], );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::PolyAnalyzer",
    );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Analysis::PolyAnalyzer');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_regextokenizer {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $whitespace_tokenizer
        = Lucy::Analysis::RegexTokenizer->new( pattern => '\S+' );

    # or...
    my $word_char_tokenizer
        = Lucy::Analysis::RegexTokenizer->new( pattern => '\w+' );

    # or...
    my $apostrophising_tokenizer = Lucy::Analysis::RegexTokenizer->new;

    # Then... once you have a tokenizer, put it into a PolyAnalyzer:
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $word_char_tokenizer, $normalizer, $stemmer ], );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $word_char_tokenizer = Lucy::Analysis::RegexTokenizer->new(
        pattern => '\w+',    # required
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::RegexTokenizer",
    );
    $binding->bind_constructor( alias => '_new' );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Analysis::Tokenizer');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_snowballstemmer {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $stemmer = Lucy::Analysis::SnowballStemmer->new( language => 'es' );
    
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $tokenizer, $normalizer, $stemmer ],
    );

This class is a wrapper around the Snowball stemming library, so it supports
the same languages.  
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $stemmer = Lucy::Analysis::SnowballStemmer->new( language => 'es' );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::SnowballStemmer",
    );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Analysis::Stemmer');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_snowballstopfilter {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $stopfilter = Lucy::Analysis::SnowballStopFilter->new(
        language => 'fr',
    );
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $tokenizer, $normalizer, $stopfilter, $stemmer ],
    );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $stopfilter = Lucy::Analysis::SnowballStopFilter->new(
        language => 'de',
    );
    
    # or...
    my $stopfilter = Lucy::Analysis::SnowballStopFilter->new(
        stoplist => \%stoplist,
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::SnowballStopFilter",
    );
    $binding->set_pod_spec($pod_spec);
    $binding->add_class_alias('KinoSearch::Analysis::Stopalizer');

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_standardtokenizer {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $tokenizer = Lucy::Analysis::StandardTokenizer->new;

    # Then... once you have a tokenizer, put it into a PolyAnalyzer:
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $tokenizer, $normalizer, $stemmer ], );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $tokenizer = Lucy::Analysis::StandardTokenizer->new;
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::StandardTokenizer",
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_token {
    my @hand_rolled = qw(
        Set_Text
        Get_Text
    ); 

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
        my $token = Lucy::Analysis::Token->new(
            text         => 'blind',
            start_offset => 8,
            end_offset   => 13,
        );

        $token->set_text('mice');
END_SYNOPSIS
    my $constructor_pod = <<'END_CONSTRUCTOR_POD';
=head2 new

    my $token = Lucy::Analysis::Token->new(
        text         => $text,          # required
        start_offset => $start_offset,  # required
        end_offset   => $end_offset,    # required
        boost        => 1.0,            # optional
        pos_inc      => 1,              # optional
    );

=over

=item *

B<text> - A string.

=item *

B<start_offset> - Start offset into the original document in Unicode
code points.

=item *

B<start_offset> - End offset into the original document in Unicode
code points.

=item *

B<boost> - Per-token weight.

=item *

B<pos_inc> - Position increment for phrase matching.

=back
END_CONSTRUCTOR_POD
    my $get_text_pod = <<'END_GET_TEXT_POD';
=head2 get_text

    my $text = $token->get_text;

Get the token's text.
END_GET_TEXT_POD
    my $set_text_pod = <<'END_SET_TEXT_POD';
=head2 set_text

    $token->set_text($text);

Set the token's text.
END_SET_TEXT_POD
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', pod => $constructor_pod );
    $pod_spec->add_method( alias => 'Get_Text', pod => $get_text_pod);
    $pod_spec->add_method( alias => 'Set_Text', pod => $set_text_pod);

    my $xs = <<'END_XS';
MODULE = Lucy    PACKAGE = Lucy::Analysis::Token

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    static const XSBind_ParamSpec param_specs[5] = {
        XSBIND_PARAM("text", true),
        XSBIND_PARAM("start_offset", true),
        XSBIND_PARAM("end_offset", true),
        XSBIND_PARAM("pos_inc", false),
        XSBIND_PARAM("boost", false)
    };
    int32_t     locations[5];
    uint32_t    start_off = 0;
    uint32_t    end_off   = 0;
    int32_t     pos_inc   = 1;
    float       boost     = 1.0f;
    STRLEN      len       = 0;
    char       *text      = NULL;
    lucy_Token *self      = NULL;

    XSBind_locate_args(aTHX_ &ST(0), 1, items, param_specs, locations, 5);

    text      = SvPVutf8(ST(locations[0]), len);
    start_off = (uint32_t)SvUV(ST(locations[1]));
    end_off   = (uint32_t)SvUV(ST(locations[2]));
    pos_inc   = locations[3] < items ? (int32_t)SvIV(ST(locations[3])) : 1;
    boost     = locations[4] < items ? (float)SvNV(ST(locations[4])) : 1.0f;

    self = (lucy_Token*)XSBind_new_blank_obj(aTHX_ either_sv);
    lucy_Token_init(self, text, len, start_off, end_off, boost,
                    pos_inc);
    RETVAL = CFISH_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
get_text(self)
    lucy_Token *self;
CODE:
    RETVAL = newSVpvn(LUCY_Token_Get_Text(self), LUCY_Token_Get_Len(self));
    SvUTF8_on(RETVAL);
OUTPUT: RETVAL

void
set_text(self, sv)
    lucy_Token *self;
    SV *sv;
PPCODE:
{
    STRLEN len;
    char *ptr = SvPVutf8(sv, len);
    LUCY_Token_Set_Text(self, ptr, len);
}
END_XS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Analysis::Token",
    );
    $binding->set_pod_spec($pod_spec);
    $binding->append_xs($xs);
    $binding->exclude_method($_) for @hand_rolled;
    $binding->exclude_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
