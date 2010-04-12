use strict;
use warnings;

package Clownfish::Parser;
use base qw( Parse::RecDescent );

use Clownfish::Parcel;
use Clownfish::Type;
use Clownfish::Type::Primitive;
use Clownfish::Type::Integer;
use Clownfish::Type::Float;
use Clownfish::Type::Void;
use Clownfish::Type::VAList;
use Clownfish::Type::Arbitrary;
use Clownfish::Type::Object;
use Clownfish::Type::Composite;
use Clownfish::Variable;
use Clownfish::DocuComment;
use Clownfish::Function;
use Clownfish::Method;
use Clownfish::Class;
use Clownfish::CBlock;
use Clownfish::File;
use Carp;

our $grammar = <<'END_GRAMMAR';

file:
    { Clownfish::Parser->set_parcel(undef); 0; }
    major_block[%arg](s) eofile
    { Clownfish::Parser->new_file( \%item, \%arg ) }

major_block:
      class_declaration[%arg]
    | embed_c
    | parcel_definition

parcel_definition:
    'parcel' class_name cnick(?) ';'
    { 
        my $parcel = Clownfish::Parser->new_parcel( \%item );
        Clownfish::Parser->set_parcel($parcel);
        $parcel;
    }

embed_c:
    '__C__'
    /.*?(?=__END_C__)/s  
    '__END_C__'
    { Clownfish::CBlock->new( contents => $item[2] ) }

class_declaration:
    docucomment(?)
    exposure_specifier(?) class_modifier(s?) 'class' class_name 
        cnick(?)
        class_extension(?)
        class_attribute(s?)
    '{'
        declaration_statement[
            class  => $item{class_name}, 
            cnick  => $item{'cnick(?)'}[0],
            parent => $item{'class_extension(?)'}[0],
        ](s?)
    '}'
    { Clownfish::Parser->new_class( \%item, \%arg ) }

class_modifier:
      'inert'
    | 'abstract'
    | 'final'
    { $item[1] }

class_extension:
    'extends' class_name
    { $item[2] }

class_attribute:
    ':' /[a-z]+(?!\w)/
    { $item[2] }

class_name:
    object_type_specifier ( "::" object_type_specifier )(s?)
    { join('::', $item[1], @{ $item[2] } ) }

cnick:
    'cnick'
    /([A-Z][A-Za-z0-9]+)(?!\w)/
    { $1 }

declaration_statement:
      var_declaration_statement[%arg]
    | subroutine_declaration_statement[%arg]
    | <error>

var_declaration_statement:
    exposure_specifier(?) variable_modifier(s?) type declarator ';'
    {
        $return = {
            exposure  => $item[1][0] || 'parcel',
            modifiers => $item[2],
            declared  => Clownfish::Parser->new_var( \%item, \%arg ),
        };
    }

subroutine_declaration_statement:
    docucomment(?)
    exposure_specifier(?) 
    subroutine_modifier(s?) 
    type 
    declarator 
    param_list 
    ';'
    {
        $return = {
            exposure  => $item[2],
            modifiers => $item[3],
            declared  => Clownfish::Parser->new_sub( \%item, \%arg ),
        };
    }

param_list:
    '(' 
    param_list_elem(s? /,/)
    (/,\s*.../)(?)
    ')'
    {
        Clownfish::Parser->new_param_list( $item[2], $item[3][0] ? 1 : 0 );
    }

param_list_elem:
    param_variable assignment(?)
    { [ $item[1], $item[2][0] ] }

param_variable:
    type declarator
    { Clownfish::Parser->new_var(\%item); }

assignment: 
    '=' scalar_constant
    { $item[2] }

type:
    nullable(?) simple_type type_postfix(s?)
    { Clownfish::Parser->simple_or_composite_type(\%item) }

nullable:
    'nullable'

simple_type:
      object_type
    | primitive_type
    | void_type
    | va_list_type
    | arbitrary_type
    { $item[1] }

object_type:
    type_qualifier(s?) object_type_specifier '*'
    { Clownfish::Parser->new_object_type(\%item); }

primitive_type:
      c_integer_type
    | chy_integer_type
    | float_type
    { $item[1] }

c_integer_type:
    type_qualifier(s?) c_integer_specifier
    { Clownfish::Parser->new_integer_type(\%item) }

chy_integer_type:
    type_qualifier(s?) chy_integer_specifier
    { Clownfish::Parser->new_integer_type(\%item) }

float_type:
    type_qualifier(s?) c_float_specifier
    { Clownfish::Parser->new_float_type(\%item) }

void_type:
    type_qualifier(s?) void_type_specifier
    { Clownfish::Parser->new_void_type(\%item) }

va_list_type:
    va_list_type_specifier
    { Clownfish::Type::VAList->new }

arbitrary_type:
    arbitrary_type_specifier
    { Clownfish::Parser->new_arbitrary_type(\%item); }

type_qualifier:
      'const' 
    | 'incremented'
    | 'decremented'
    | 'nullable'

subroutine_modifier:
      'inert'
    | 'inline'
    | 'abstract'
    | 'final'
    { $item[1] }

exposure_specifier:
      'public'
    | 'private'
    | 'parcel'
    | 'local'

variable_modifier:
      'inert'
    { $item[1] }

type_specifier:
    (    object_type_specifier 
       | primitive_type_specifier
       | void_type_specifier
       | va_list_type_specifier
       | arbitrary_type_specifier
    ) 
    { $item[1] }

primitive_type_specifier:
      chy_integer_specifier
    | c_integer_specifier 
    | c_float_specifier 
    { $item[1] }

chy_integer_specifier:
    /(?:chy_)?(bool)_t(?!\w)/

c_integer_specifier:
    /(?:(?:u?int(?:8|16|32|64)_t)|(?:char|int|short|long|size_t))(?!\w)/

c_float_specifier:
    /(?:float|double)(?!\w)/

void_type_specifier:
    /void(?!\w)/

va_list_type_specifier:
    /va_list(?!\w)/

arbitrary_type_specifier:
    /\w+_t(?!\w)/

object_type_specifier:
    /[A-Z]+[A-Z0-9]*[a-z]+[A-Za-z0-9]*(?!\w)/

declarator:
    identifier 
    { $item[1] }

type_postfix:
      '*'
      { '*' }
    | '[' ']'
      { '[]' }
    | '[' constant_expression ']'
      { "[$item[2]]" }

identifier:
    ...!reserved_word /[a-zA-Z_]\w*/x
    { $item[2] }

docucomment:
    /\/\*\*.*?\*\//s
    { Clownfish::DocuComment->parse($item[1]) }

constant_expression:
      /\d+/
    | /[A-Z_]+/

scalar_constant:
      hex_constant
    | float_constant
    | integer_constant
    | string_literal
    | 'NULL'
    | 'true'
    | 'false'

integer_constant:
    /(?:-\s*)?\d+/
    { $item[1] }

hex_constant:
    /0x[a-fA-F0-9]+/
    { $item[1] }

float_constant:
    /(?:-\s*)?\d+\.\d+/
    { $item[1] }

string_literal: 
    /"(?:[^"\\]|\\.)*"/
    { $item[1] }

reserved_word:
    /(const|double|enum|extern|float|register|signed|sizeof
       |inert|struct|typedef|union|unsigned|void)(?!\w)/x
    | chy_integer_specifier
    | c_integer_specifier

eofile:
    /^\Z/

END_GRAMMAR

sub new { return shift->SUPER::new($grammar) }

sub strip_plain_comments {
    my ( $self, $text ) = @_;
    while ( $text =~ m#(/\*[^*].*?\*/)#ms ) {
        my $blanked = $1;
        $blanked =~ s/\S/ /g;
        $text    =~ s#/\*[^*].*?\*/#$blanked#ms;
    }
    return $text;
}

our $parcel = undef;
sub set_parcel { $parcel = $_[1] }

sub new_integer_type {
    my ( undef, $item ) = @_;
    my $specifier = $item->{c_integer_specifier}
        || $item->{chy_integer_specifier};
    my %args = ( specifier => $specifier );
    $args{$_} = 1 for @{ $item->{'type_qualifier(s?)'} };
    return Clownfish::Type::Integer->new(%args);
}

sub new_float_type {
    my ( undef, $item ) = @_;
    my %args = ( specifier => $item->{c_float_specifier} );
    $args{$_} = 1 for @{ $item->{'type_qualifier(s?)'} };
    return Clownfish::Type::Float->new(%args);
}

sub new_void_type {
    my ( undef, $item ) = @_;
    my %args = ( specifier => $item->{void_type_specifier} );
    $args{$_} = 1 for @{ $item->{'type_qualifier(s?)'} };
    return Clownfish::Type::Void->new(%args);
}

sub new_arbitrary_type {
    my ( undef, $item ) = @_;
    return Clownfish::Type::Arbitrary->new(
        specifier => $item->{arbitrary_type_specifier},
        parcel    => $parcel,
    );
}

sub new_object_type {
    my ( undef, $item ) = @_;
    my %args = (
        specifier => $item->{object_type_specifier},
        parcel    => $parcel,
    );
    $args{$_} = 1 for @{ $item->{'type_qualifier(s?)'} };
    return Clownfish::Type::Object->new(%args);
}

sub simple_or_composite_type {
    my ( undef, $item ) = @_;
    my $simple_type = $item->{simple_type};
    my $postfixes   = $item->{'type_postfix(s?)'};
    my $nullable    = scalar @{ $item->{'nullable(?)'} } ? 1 : undef;
    my $type;

    if ( !@$postfixes ) {
        if ($nullable) {
            my $type_class = ref($simple_type);
            confess "$type_class can't be 'nullable'" unless
                $simple_type->isa("Clownfish::Type::Object");
            $simple_type->set_nullable($nullable);
        }
        return $simple_type;
    }
    else {
        my %args = (
            child       => $simple_type,
            indirection => 0,
            nullable    => $nullable,
        );
        for my $postfix (@$postfixes) {
            if ( $postfix =~ /\[/ ) {
                $args{array} ||= '';
                $args{array} .= $postfix;
            }
            elsif ( $postfix eq '*' ) {
                $args{indirection}++;
            }
        }
        return Clownfish::Type::Composite->new(%args);
    }
}

sub new_var {
    my ( undef, $item, $arg ) = @_;
    my $exposure = $item->{'exposure_specifier(?)'}[0];
    my %args = $exposure ? ( exposure => $exposure ) : ();
    if ($arg) {
        $args{class_name}  = $arg->{class} if $arg->{class};
        $args{class_cnick} = $arg->{cnick} if $arg->{cnick};
    }
    return Clownfish::Variable->new(
        parcel    => $parcel,
        type      => $item->{type},
        micro_sym => $item->{declarator},
        %args,
    );
}

sub new_param_list {
    my ( undef, $param_list_elems, $variadic ) = @_;
    my @vars = map { $_->[0] } @$param_list_elems;
    my @vals = map { $_->[1] } @$param_list_elems;
    return Clownfish::ParamList->new(
        variables      => \@vars,
        initial_values => \@vals,
        variadic       => $variadic,
    );
}

sub new_sub {
    my ( undef, $item, $arg ) = @_;
    my $class;
    my $modifiers  = $item->{'subroutine_modifier(s?)'};
    my $docucom    = $item->{'docucomment(?)'}[0];
    my $exposure   = $item->{'exposure_specifier(?)'}[0];
    my $inert      = scalar grep { $_ eq 'inert' } @$modifiers;
    my %extra_args = $exposure ? ( exposure => $exposure ) : ();

    if ($inert) {
        $class = 'Clownfish::Function';
        $extra_args{micro_sym} = $item->{declarator};
        $extra_args{inline} = scalar grep { $_ eq 'inline' } @$modifiers;
    }
    else {
        $class = 'Clownfish::Method';
        $extra_args{macro_sym} = $item->{declarator};
        $extra_args{abstract} = scalar grep { $_ eq 'abstract' } @$modifiers;
        $extra_args{final}    = scalar grep { $_ eq 'final' } @$modifiers;
    }

    return $class->new(
        parcel      => $parcel,
        docucomment => $docucom,
        class_name  => $arg->{class},
        class_cnick => $arg->{cnick},
        return_type => $item->{type},
        param_list  => $item->{param_list},
        %extra_args,
    );
}

sub new_class {
    my ( undef, $item, $arg ) = @_;
    my ( @member_vars, @inert_vars, @functions, @methods );
    my $source_class = $arg->{source_class} || $item->{class_name};
    my %class_modifiers
        = map { ( $_ => 1 ) } @{ $item->{'class_modifier(s?)'} };
    my %class_attributes
        = map { ( $_ => 1 ) } @{ $item->{'class_attribute(s?)'} };

    for my $declaration ( @{ $item->{'declaration_statement(s?)'} } ) {
        my $declared  = $declaration->{declared};
        my $exposure  = $declaration->{exposure};
        my $modifiers = $declaration->{modifiers};
        my $inert     = ( scalar grep {/inert/} @$modifiers ) ? 1 : 0;
        my $subs      = $inert ? \@functions : \@methods;
        my $vars      = $inert ? \@inert_vars : \@member_vars;

        if ( $declared->isa('Clownfish::Variable') ) {
            push @$vars, $declared;
        }
        else {
            push @$subs, $declared;
        }
    }

    return Clownfish::Class->create(
        parcel            => $parcel,
        class_name        => $item->{class_name},
        cnick             => $item->{'cnick(?)'}[0],
        parent_class_name => $item->{'class_extension(?)'}[0],
        member_vars       => \@member_vars,
        functions         => \@functions,
        methods           => \@methods,
        inert_vars        => \@inert_vars,
        docucomment       => $item->{'docucomment(?)'}[0],
        source_class      => $source_class,
        inert             => $class_modifiers{inert},
        final             => $class_modifiers{final},
        attributes        => \%class_attributes,
    );
}

sub new_file {
    my ( undef, $item, $arg ) = @_;

    return Clownfish::File->new(
        parcel       => $parcel,
        blocks       => $item->{'major_block(s)'},
        source_class => $arg->{source_class},
    );
}

sub new_parcel {
    my ( undef, $item ) = @_;
    Clownfish::Parcel->singleton(
        name  => $item->{class_name},
        cnick => $item->{'cnick(?)'}[0],
    );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Parser - Parse Clownfish header files.

=head1 SYNOPSIS

     my $class_def = $parser->class($class_text);

=head1 DESCRIPTION

Clownfish::Parser is a combined lexer/parser which parses Clownfish header
files.  It is not at all strict, as it relies heavily on the C parser to pick
up errors such as misspelled type names.

=head1 METHODS

=head2 new

Constructor, takes no arguments.

=head2 strip_plain_comments

    my $stripped = $parser->strip_plain_comments($code_with_comments);

Remove plain C comments from supplied code.  All non-whitespace characters are
turned to spaces; all whitespace characters are preserved, so that the number
of lines is consistent between before and after.

JavaDoc-syntax "DocuComments", which begin with "/**" are left alone.  

This is a sloppy implementation which will mangle quoted comments and such.

=head1 COPYRIGHT AND LICENSE

    /**
     * Copyright 2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

=cut

