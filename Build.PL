use 5.008003;
use strict;
use warnings;

use Module::Build;

my $subclass_code = <<'END_BUILD_CLASS';

package Lucy::Build;
use strict;
use warnings;
use base qw( Module::Build );

use File::Spec::Functions qw( catdir catfile splitpath );
use File::Path qw( mkpath );
use File::Find qw( find );
use ExtUtils::CBuilder;
use Config;

my $METAQUOTE_EXE_PATH     = 'metaquote';
my $CHARMONIZE_EXE_PATH    = 'charmonize';
my $CHARMONIZER_SOURCE_DIR = catdir(qw( charmonizer src ));

# Compile the metaquote source filter utility.
sub ACTION_metaquote {
    my $self        = shift;
    my $source_path = catfile(qw( charmonizer metaquote metaquote.c ));

    # don't compile if we're up to date
    return if $self->up_to_date( [$source_path], $METAQUOTE_EXE_PATH );

    # compile
    print "\nBuilding $METAQUOTE_EXE_PATH...\n\n";
    my $cbuilder = ExtUtils::CBuilder->new;
    my $o_file = $cbuilder->compile( source => $source_path );
    $cbuilder->link_executable(
        objects  => [$o_file],
        exe_file => $METAQUOTE_EXE_PATH,
    );

    # clean both the object file and the executable
    $self->add_to_cleanup( $o_file, $METAQUOTE_EXE_PATH );
}

# Build the charmonize executable.
sub ACTION_charmonizer {
    my $self         = shift;
    my $filtered_dir = catdir(qw( charmonizer filtered_src ));
    if ( !-d $filtered_dir ) {
        mkpath($filtered_dir) or die "can't mkpath '$filtered_dir': $!";
    }

    $self->dispatch('metaquote');

    my $charm_source_files
        = $self->_find_charm_source_files($CHARMONIZER_SOURCE_DIR);
    my $filtered_files = $self->_metaquote_charm_files($charm_source_files);
    my $charmonize_c   = catfile(qw( charmonizer charmonize.c ));
    my @all_source     = ( $charmonize_c, @$filtered_files );

    # don't compile if we're up to date
    return if $self->up_to_date( \@all_source, $CHARMONIZE_EXE_PATH );

    print "Building $CHARMONIZE_EXE_PATH...\n\n";

    my $cbuilder = ExtUtils::CBuilder->new;

    my @o_files;
    for (@all_source) {
        next unless /\.c$/;
        my $o_file = $cbuilder->object_file($_);
        push @o_files, $o_file;

        next if $self->up_to_date( $_, $o_file );

        $cbuilder->compile(
            source       => $_,
            include_dirs => [$filtered_dir],
        );
    }

    my $exe_path = $cbuilder->link_executable(
        objects  => \@o_files,
        exe_file => $CHARMONIZE_EXE_PATH,
    );

    $self->add_to_cleanup( $filtered_dir, @$filtered_files, @o_files,
        $CHARMONIZE_EXE_PATH, );
}

sub _find_charm_source_files {
    my ( $self, $dir ) = @_;
    my @files;
    find(
        {   wanted => sub {
                if ( $File::Find::name =~ /\.c?harm$/ ) {
                    push @files, $File::Find::name;
                }
            },
            no_chdir => 1,
        },
        $dir,
    );
    return \@files;
}

sub _metaquote_charm_files {
    my ( $self, $charm_files ) = @_;
    my @filtered_files;

    for my $source_path (@$charm_files) {
        my $dest_path = $source_path;
        $dest_path =~ s#(.*)src#$1filtered_src#;
        $dest_path =~ s#\.charm#.c#;
        $dest_path =~ s#\.harm#.h#;

        push @filtered_files, $dest_path;

        next if ( $self->up_to_date( $source_path, $dest_path ) );

        # create directories if need be
        my ( undef, $dir, undef ) = splitpath($dest_path);
        if ( !-d $dir ) {
            $self->add_to_cleanup($dir);
            mkpath $dir or die "Couldn't mkpath $dir";
        }

        # run the metaquote filter
        system("./$METAQUOTE_EXE_PATH $source_path $dest_path");
    }

    return \@filtered_files;
}

# Run the charmonizer executable, creating the lucyconf.h file.
sub ACTION_lucyconf {
    my $self = shift;

    $self->dispatch('charmonizer');

    my $lucyconf_path = "lucyconf.h";
    return if $self->up_to_date( $CHARMONIZE_EXE_PATH, $lucyconf_path );

    print "\nWriting $lucyconf_path...\n\n";
    system(
        "./$CHARMONIZE_EXE_PATH --outpath=$lucyconf_path --cc=$Config{cc}");

    $self->add_to_cleanup($lucyconf_path);
}

sub ACTION_code {
    my $self = shift;
    $self->dispatch('lucyconf');
    $self->SUPER::ACTION_code(@_);
}

END_BUILD_CLASS

my $class = Module::Build->subclass(
    class => 'Lucy::Build',
    code  => $subclass_code,
);

my $builder = $class->new(
    module_name        => 'Lucy',
    license            => 'apache',
    dist_author        => 'Marvin Humphrey <marvin at rectangular dot com>',
    dist_version_from  => 'src/perl/Lucy.pm',
    create_makefile_pl => 'passthrough',
    add_to_cleanup     => [
        'Lucy-*',       'typemap', 'MANIFEST.bak', 'lastmod',
        'perltidy.ERR', '*.o',
    ],
);

$builder->create_build_script();

__END__


    /**
     * Copyright 2006 The Apache Software Foundation
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

