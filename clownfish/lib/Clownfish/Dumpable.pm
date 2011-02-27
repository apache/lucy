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
    my $package = ref($either) || $either;
    return $either->_new();
}

sub add_dumpables {
    my ( $self, $class ) = @_;
    confess( $class->get_class_name . " isn't dumpable" )
        unless $class->has_attribute('dumpable');

    # Inherit Dump/Load from parent if no novel member vars.
    my $parent = $class->get_parent;
    if ( $parent and $parent->has_attribute('dumpable') ) {
        return unless scalar @{ $class->novel_member_vars };
    }

    if ( !$class->novel_method('Dump') ) {
        $self->_add_dump_method($class);
    }
    if ( !$class->novel_method('Load') ) {
        $self->_add_load_method($class);
    }
}

sub _add_dump_method {
    my ( $self, $class ) = @_;
    my $method = $self->_make_method_obj( $class, 'Dump' );
    $class->add_method($method);
    my $full_func_sym = $method->full_func_sym;
    my $full_struct   = $class->full_struct_sym;
    my @members;
    my $parent = $class->get_parent;

    if ( $parent and $parent->has_attribute('dumpable') ) {
        my $vtable_var = $class->full_vtable_var;
        my $typedef    = $method->full_typedef;
        my $cnick      = $class->get_cnick;
        my $autocode   = <<END_STUFF;
cfish_Obj*
$full_func_sym($full_struct *self)
{
    $typedef super_dump = ($typedef)SUPER_METHOD($vtable_var, $cnick, Dump);
    cfish_Hash *dump = (cfish_Hash*)super_dump(self);
END_STUFF
        $class->append_autocode($autocode);
        @members = @{ $class->novel_member_vars };
    }
    else {
        my $autocode = <<END_STUFF;
cfish_Obj*
$full_func_sym($full_struct *self)
{
    cfish_Hash *dump = cfish_Hash_new(0);
    Cfish_Hash_Store_Str(dump, "_class", 6,
        (cfish_Obj*)Cfish_CB_Clone(Cfish_Obj_Get_Class_Name((cfish_Obj*)self)));
END_STUFF
        $class->append_autocode($autocode);
        @members = @{ $class->member_vars };
        shift @members;    # skip self->vtable
        shift @members;    # skip refcount self->ref
    }

    for my $member_var (@members) {
        $class->append_autocode( _process_dump_member($member_var) );
    }
    $class->append_autocode("    return (cfish_Obj*)dump;\n}\n\n");
}

sub _add_load_method {
    my ( $self, $class ) = @_;
    my $method = $self->_make_method_obj( $class, 'Load' );
    $class->add_method($method);
    my $full_func_sym = $method->full_func_sym;
    my $full_struct   = $class->full_struct_sym;
    my @members;
    my $parent = $class->get_parent;

    if ( $parent and $parent->has_attribute('dumpable') ) {
        my $vtable_var = $class->full_vtable_var;
        my $typedef    = $method->full_typedef;
        my $cnick      = $class->get_cnick;
        my $autocode   = <<END_STUFF;
cfish_Obj*
$full_func_sym($full_struct *self, cfish_Obj *dump)
{
    cfish_Hash *source = (cfish_Hash*)CFISH_CERTIFY(dump, CFISH_HASH);
    $typedef super_load = ($typedef)SUPER_METHOD($vtable_var, $cnick, Load);
    $full_struct *loaded = ($full_struct*)super_load(self, dump);
END_STUFF
        $class->append_autocode($autocode);
        @members = @{ $class->novel_member_vars };
    }
    else {
        my $autocode = <<END_STUFF;
cfish_Obj*
$full_func_sym($full_struct *self, cfish_Obj *dump)
{
    cfish_Hash *source = (cfish_Hash*)CFISH_CERTIFY(dump, CFISH_HASH);
    cfish_CharBuf *class_name = (cfish_CharBuf*)CFISH_CERTIFY(
        Cfish_Hash_Fetch_Str(source, "_class", 6), CFISH_CHARBUF);
    cfish_VTable *vtable = cfish_VTable_singleton(class_name, NULL);
    $full_struct *loaded = ($full_struct*)Cfish_VTable_Make_Obj(vtable);
    CHY_UNUSED_VAR(self);
END_STUFF
        $class->append_autocode($autocode);
        @members = @{ $class->member_vars };
        shift @members;    # skip self->vtable
        shift @members;    # skip refcount self->ref
    }

    for my $member_var (@members) {
        $class->append_autocode( _process_load_member($member_var) );
    }
    $class->append_autocode("    return (cfish_Obj*)loaded;\n}\n\n");
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
