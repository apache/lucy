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

use strict;
use warnings;

package Clownfish::Dumpable;
use Carp;
use Clownfish::Class;
use Clownfish::Type;
use Clownfish::Method;
use Clownfish::Variable;

sub new {
    my $either = shift;
    return bless {}, ref($either) || $either;
}

sub add_dumpables {
    my ( $self, $class ) = @_;
    confess( $class->get_class_name . " isn't dumpable" )
        unless $class->has_attribute('dumpable');

    # Inherit Dump/Load from parent if no novel member vars.
    my $parent = $class->get_parent;
    if ( $parent and $parent->has_attribute('dumpable') ) {
        return unless scalar $class->novel_member_vars;
    }

    if ( !$class->novel_method('Dump') ) {
        $self->_add_dump_method($class);
    }
    if ( !$class->novel_method('Load') ) {
        $self->_add_load_method($class);
    }
}

# Create a Clownfish::Method object for either Dump() or Load().
sub _make_method_obj {
    my ( $self, $class, $dump_or_load ) = @_;
    my $return_type = Clownfish::Type::Object->new(
        incremented => 1,
        specifier   => 'Obj',
        indirection => 1,
        parcel      => $class->get_parcel,
    );
    my $self_type = Clownfish::Type::Object->new(
        specifier   => $class->get_struct_sym,
        indirection => 1,
        parcel      => $class->get_parcel,
    );
    my $self_var = Clownfish::Variable->new(
        type      => $self_type,
        parcel    => $class->get_parcel,
        micro_sym => 'self',
    );

    my $param_list;
    if ( $dump_or_load eq 'Dump' ) {
        $param_list = Clownfish::ParamList->new( variables => [$self_var], );
    }
    else {
        my $dump_type = Clownfish::Type::Object->new(
            specifier   => 'Obj',
            indirection => 1,
            parcel      => $class->get_parcel,
        );
        my $dump_var = Clownfish::Variable->new(
            type      => $dump_type,
            parcel    => $class->get_parcel,
            micro_sym => 'dump',
        );
        $param_list = Clownfish::ParamList->new(
            variables => [ $self_var, $dump_var ], );
    }

    return Clownfish::Method->new(
        parcel      => $class->get_parcel,
        return_type => $return_type,
        class_name  => $class->get_class_name,
        class_cnick => $class->get_cnick,
        param_list  => $param_list,
        macro_sym   => $dump_or_load,
        exposure    => 'public',
    );
}

sub _add_dump_method {
    my ( $self, $class ) = @_;
    my $method = $self->_make_method_obj( $class, 'Dump' );
    $class->add_method($method);
    my $full_func_sym = $method->full_func_sym;
    my $full_struct   = $class->full_struct_sym;
    my $autocode;
    my @members;
    my $parent = $class->get_parent;

    if ( $parent and $parent->has_attribute('dumpable') ) {
        my $super_dump = 'kino_' . $parent->get_cnick . '_dump';
        my $super_type = $parent->full_struct_sym;
        $autocode = <<END_STUFF;
kino_Obj*
$full_func_sym($full_struct *self)
{
    kino_Hash *dump = (kino_Hash*)$super_dump(($super_type*)self);
END_STUFF
        @members = $class->novel_member_vars;
    }
    else {
        $autocode = <<END_STUFF;
kino_Obj*
$full_func_sym($full_struct *self)
{
    kino_Hash *dump = kino_Hash_new(0);
    Kino_Hash_Store_Str(dump, "_class", 6,
        (kino_Obj*)Kino_CB_Clone(Kino_Obj_Get_Class_Name((kino_Obj*)self)));
END_STUFF
        @members = $class->member_vars;
        shift @members;    # skip self->vtable
        shift @members;    # skip refcount self->ref
    }

    for my $member_var (@members) {
        $autocode .= $self->_process_dump_member( $class, $member_var );
    }
    $autocode .= "    return (kino_Obj*)dump;\n}\n\n";
    $class->append_autocode($autocode);
}

sub _process_dump_member {
    my ( $self, $class, $member ) = @_;
    my $type = $member->get_type;
    my $name = $member->micro_sym;
    my $len  = length($name);
    if ( $type->is_integer ) {
        return qq|    Kino_Hash_Store_Str(dump, "$name", $len, |
            . qq|(kino_Obj*)kino_CB_newf("%i64", (int64_t)self->$name));\n|;
    }
    elsif ( $type->is_floating ) {
        return qq|    Kino_Hash_Store_Str(dump, "$name", $len, |
            . qq|(kino_Obj*)kino_CB_newf("%f64", (double)self->$name));\n|;
    }
    elsif ( $type->is_object ) {
        return <<END_STUFF;
    if (self->$name) {
         Kino_Hash_Store_Str(dump, "$name", $len, Kino_Obj_Dump((kino_Obj*)self->$name));
    }
END_STUFF
    }
    else {
        confess( "Don't know how to dump a " . $type->get_specifier );
    }
}

sub _add_load_method {
    my ( $self, $class ) = @_;
    my $method = $self->_make_method_obj( $class, 'Load' );
    $class->add_method($method);
    my $full_func_sym = $method->full_func_sym;
    my $full_struct   = $class->full_struct_sym;
    my $autocode;
    my @members;
    my $parent = $class->get_parent;

    if ( $parent and $parent->has_attribute('dumpable') ) {
        my $super_load = 'kino_' . $parent->get_cnick . '_load';
        my $super_type = $parent->full_struct_sym;
        $autocode = <<END_STUFF;
kino_Obj*
$full_func_sym($full_struct *self, kino_Obj *dump)
{
    kino_Hash *source = (kino_Hash*)KINO_CERTIFY(dump, KINO_HASH);
    $full_struct *loaded 
        = ($full_struct*)$super_load(($super_type*)self, dump);
    CHY_UNUSED_VAR(self);
END_STUFF
        @members = $class->novel_member_vars;
    }
    else {
        $autocode = <<END_STUFF;
kino_Obj*
$full_func_sym($full_struct *self, kino_Obj *dump)
{
    kino_Hash *source = (kino_Hash*)KINO_CERTIFY(dump, KINO_HASH);
    kino_CharBuf *class_name = (kino_CharBuf*)KINO_CERTIFY(
        Kino_Hash_Fetch_Str(source, "_class", 6), KINO_CHARBUF);
    kino_VTable *vtable = kino_VTable_singleton(class_name, NULL);
    $full_struct *loaded = ($full_struct*)Kino_VTable_Make_Obj(vtable);
    CHY_UNUSED_VAR(self);
END_STUFF
        @members = $class->member_vars;
        shift @members;    # skip self->vtable
        shift @members;    # skip refcount self->ref
    }

    for my $member_var (@members) {
        $autocode .= $self->_process_load_member( $class, $member_var );
    }
    $autocode .= "    return (kino_Obj*)loaded;\n}\n\n";
    $class->append_autocode($autocode);
}

sub _process_load_member {
    my ( $self, $class, $member ) = @_;
    my $type       = $member->get_type;
    my $type_str   = $type->to_c;
    my $name       = $member->micro_sym;
    my $len        = length($name);
    my $struct_sym = $type->get_specifier;
    my $vtable_var = uc($struct_sym);
    my $extraction
        = $type->is_integer  ? qq|($type_str)Kino_Obj_To_I64(var)|
        : $type->is_floating ? qq|($type_str)Kino_Obj_To_F64(var)|
        : $type->is_object
        ? qq|($struct_sym*)KINO_CERTIFY(Kino_Obj_Load(var, var), $vtable_var)|
        : confess( "Don't know how to load " . $type->get_specifier );
    return <<END_STUFF;
    {
        kino_Obj *var = Kino_Hash_Fetch_Str(source, "$name", $len);
        if (var) { loaded->$name = $extraction; }
    }
END_STUFF
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Dumpable - Auto-generate code for "dumpable" classes.

=head1 SYNOPSIS

    my $dumpable = Clownfish::Dumpable->new;
    for my $class ( grep { $_->has_attribute('dumpable') } @classes ) {
        $dumpable->add_dumpables($class);
    }

=head1 DESCRIPTION

If a class declares that it has the attribute "dumpable", but does not declare
either Dump or Load(), Clownfish::Dumpable will attempt to auto-generate
those methods if methods inherited from the parent class do not suffice.

    class Foo::Bar inherits Foo : dumpable {
        Thing *thing;

        public inert incremented Bar*
        new();

        void
        Destroy(Bar *self);
    }

=head1 METHODS

=head2 new

    my $dumpable = Clownfish::Dumpable->new;

Constructor.  Takes no arguments.

=head2 add_dumpables

    $dumpable->add_dumpables($dumpable_class);

Analyze a class with the attribute "dumpable" and add Dump() or Load() methods
as necessary.

=cut
