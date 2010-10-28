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

use Test::More tests => 53;
use Clownfish::Class;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;

my $thing = Clownfish::Variable->new(
    parcel     => 'Neato',
    class_name => 'Foo',
    type       => $parser->type('Thing*'),
    micro_sym  => 'thing',
);
my $widget = Clownfish::Variable->new(
    class_name => 'Widget',
    type       => $parser->type('Widget*'),
    micro_sym  => 'widget',
);
my $tread_water = Clownfish::Function->new(
    parcel      => 'Neato',
    class_name  => 'Foo',
    return_type => $parser->type('void'),
    micro_sym   => 'tread_water',
    param_list  => $parser->param_list('()'),
);
my %foo_create_args = (
    parcel      => 'Neato',
    class_name  => 'Foo',
    member_vars => [$thing],
    inert_vars  => [$widget],
    functions   => [$tread_water],
);

my $foo = Clownfish::Class->create(%foo_create_args);
eval { Clownfish::Class->create(%foo_create_args) };
like( $@, qr/conflict/i,
    "Can't call create for the same class more than once" );
my $should_be_foo = Clownfish::Class->fetch_singleton(
    parcel     => 'Neato',
    class_name => 'Foo',
);
is( $foo, $should_be_foo, "fetch_singleton" );

my $foo_jr = Clownfish::Class->create(
    parcel            => 'Neato',
    class_name        => 'Foo::FooJr',
    parent_class_name => 'Foo',
    attributes        => { dumpable => 1 },
);

ok( $foo_jr->has_attribute('dumpable'), 'has_attribute' );
is( $foo_jr->get_struct_sym,  'FooJr',       "struct_sym" );
is( $foo_jr->full_struct_sym, 'neato_FooJr', "full_struct_sym" );

my $final_foo = Clownfish::Class->create(
    parcel            => 'Neato',
    class_name        => 'Foo::FooJr::FinalFoo',
    parent_class_name => 'Foo::FooJr',
    source_class      => 'Foo::FooJr',
    final             => 1,
    attributes        => { dumpable => 1 },
);
ok( $final_foo->final, "final" );
is( $final_foo->file_path( '/path/to', '.c', ),
    '/path/to/Foo/FooJr.c', "file_path" );
is( $final_foo->include_h, 'Foo/FooJr.h', "inlude_h uses source_class" );
is( $final_foo->get_parent_class_name, 'Foo::FooJr',
    "get_parent_class_name" );

my $do_stuff
    = $parser->subroutine_declaration_statement( 'void Do_Stuff(Foo *self);',
    0, class => 'Foo' )->{declared}
    or die "parsing failure";
$foo->add_method($do_stuff);

my $inert_do_stuff
    = $parser->subroutine_declaration_statement(
    'void Do_Stuff(InertFoo *self);',
    0, class => 'InertFoo' )->{declared}
    or die "parsing failure";
my %inert_args = (
    parcel     => 'Neato',
    class_name => 'InertFoo',
    inert      => 1,
);
eval {
    Clownfish::Class->create( %inert_args, methods => [$inert_do_stuff] );
};
like(
    $@,
    qr/inert class/i,
    "Error out on conflict between inert attribute and object method"
);

$foo->add_child($foo_jr);
$foo_jr->add_child($final_foo);
$foo->grow_tree;
eval { $foo->grow_tree };
like( $@, qr/grow_tree/, "call grow_tree only once." );
eval { $foo_jr->add_method($do_stuff) };
like( $@, qr/grow_tree/, "Forbid add_method after grow_tree." );

is( $foo_jr->get_parent,            $foo,      "grow_tree, one level" );
is( $final_foo->get_parent,         $foo_jr,   "grow_tree, two levels" );
is( $foo->novel_method("Do_Stuff"), $do_stuff, 'novel_method' );
is( $foo_jr->method("Do_Stuff"),    $do_stuff, "inherited method" );
ok( !$foo_jr->novel_method("Do_Stuff"),    'inherited method not novel' );
ok( $final_foo->method("Do_Stuff")->final, "Finalize inherited method" );
ok( !$foo_jr->method("Do_Stuff")->final, "Don't finalize method in parent" );
is_deeply( [ $foo->inert_vars ],        [$widget],      "inert vars" );
is_deeply( [ $foo->functions ],         [$tread_water], "inert funcs" );
is_deeply( [ $foo->methods ],           [$do_stuff],    "methods" );
is_deeply( [ $foo->novel_methods ],     [$do_stuff],    "novel_methods" );
is_deeply( [ $foo->novel_member_vars ], [$thing],       "novel_member_vars" );
is_deeply( [ $foo_jr->member_vars ], [$thing], "inherit member vars" );
is_deeply( [ $foo_jr->functions ],   [],       "don't inherit inert funcs" );
is_deeply( [ $foo_jr->novel_member_vars ], [], "novel_member_vars" );
is_deeply( [ $foo_jr->inert_vars ],        [], "don't inherit inert vars" );
is_deeply( [ $final_foo->novel_methods ],  [], "novel_methods" );

like( $foo_jr->get_autocode, qr/load/i, "autogenerate Dump/Load" );
is_deeply(
    [ $foo->tree_to_ladder ],
    [ $foo, $foo_jr, $final_foo ],
    'tree_to_ladder'
);

ok( $parser->class_modifier($_), "class_modifier: $_" )
    for (qw( abstract inert ));

ok( $parser->class_inheritance($_), "class_inheritance: $_" )
    for ( 'inherits Foo', 'inherits Foo::FooJr::FooIII' );

my $class_content
    = 'public class Foo::FooJr cnick FooJr inherits Foo { private int num; }';
my $class = $parser->class_declaration($class_content);
isa_ok( $class, "Clownfish::Class", "class_declaration FooJr" );
ok( ( scalar grep { $_->micro_sym eq 'num' } $class->member_vars ),
    "parsed private member var" );

$class_content = q|
    /** 
     * Bow wow.
     *
     * Wow wow wow.
     */
    public class Animal::Dog inherits Animal : lovable : drooly {
        public inert Dog* init(Dog *self, CharBuf *name, CharBuf *fave_food);
        inert uint32_t count();
        inert uint64_t num_dogs;

        private CharBuf *name;
        private bool_t   likes_to_go_fetch;
        private void     Chase_Tail(Dog *self);

        ChewToy *squishy;

        void               Destroy(Dog *self);
        public CharBuf*    Bark(Dog *self);
        public void        Eat(Dog *self);
        public void        Bite(Dog *self, Enemy *enemy);
        public Thing      *Fetch(Dog *self, Thing *thing);
        public final void  Bury(Dog *self, Bone *bone);
        public Owner      *mom;
        public abstract incremented nullable Thing*
        Scratch(Dog *self);

        int32_t[1]  flexible_array_at_end_of_struct;
    }
|;

$class = $parser->class_declaration($class_content);
isa_ok( $class, "Clownfish::Class", "class_declaration Dog" );
ok( ( scalar grep { $_->micro_sym eq 'num_dogs' } $class->inert_vars ),
    "parsed inert var" );
ok( ( scalar grep { $_->micro_sym eq 'mom' } $class->member_vars ),
    "parsed public member var" );
ok( ( scalar grep { $_->micro_sym eq 'squishy' } $class->member_vars ),
    "parsed parcel member var" );
ok( ( scalar grep { $_->micro_sym eq 'init' } $class->functions ),
    "parsed function" );
ok( ( scalar grep { $_->micro_sym eq 'chase_tail' } $class->methods ),
    "parsed private method" );
ok( ( scalar grep { $_->micro_sym eq 'destroy' } $class->methods ),
    "parsed parcel method" );
ok( ( scalar grep { $_->micro_sym eq 'bury' } $class->methods ),
    "parsed public method" );
ok( ( scalar grep { $_->micro_sym eq 'scratch' } $class->methods ),
    "parsed public abstract nullable method" );

for my $method ( $class->methods ) {
    if ( $method->micro_sym eq 'scratch' ) {
        ok( $method->{return_type}->nullable,
            "public abstract incremented nullable flagged as nullable" );
    }
}
is( ( scalar grep { $_->public } $class->methods ),
    6, "pass acl to Method constructor" );
ok( $class->has_attribute('lovable'), "parsed class attribute" );
ok( $class->has_attribute('drooly'),  "parsed second class attribute" );

$class_content = qq|
    parcel inert class Rigor::Mortis cnick Mort { 
        parcel inert void lie_still(); 
    }|;
$class = $parser->class_declaration($class_content);
isa_ok( $class, "Clownfish::Class", "inert class_declaration" );
ok( $class->inert, "inert modifier parsed and passed to constructor" );

$class_content = qq|
    final class Ultimo { 
        /** Throws an error. 
         */
        void Say_Never(Ultimo *self); 
    }|;
$class = $parser->class_declaration($class_content);
ok( $class->final, "final class_declaration" );
