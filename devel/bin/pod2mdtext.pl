#!/usr/bin/perl

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

=head1 NAME

pod2mdtext.pl - Convert POD to mdtext for the Apache CMS

=head1 SYNOPSIS

    pod2mdtext.pl --name=PROJECT_NAME [--full-name=FULL_NAME]
                  --version=X.Y.Z

=head1 DESCRIPTION

This script creates mdtext files from POD. It must be run in the C<perl>
directory and scans all .pod and .pm files found in C<lib>. The resulting
mdtext files are stored in a directory named C<mdtext>.

=head1 OPTIONS

=head2 --name

Short name of the project used for the index filename.

=head2 --full-name

Full name of the project used in titles. Defaults to C<--name>.

=head2 --version

Version number.

=cut

use strict;
use warnings;
use utf8;

use File::Find;
use File::Path qw( make_path );
use File::Slurp;
use Getopt::Long qw( GetOptions );
use Pod::Simple::HTML;

my $out_root = 'mdtext';

my $usage = join( ' ',
    $0,
    '--name=PROJECT_NAME',
    '[--full-name=FULL_NAME]',
    '--version=X.Y.Z',
) . "\n";

my ( $project_name, $full_name, $version );
GetOptions(
    'name=s'      => \$project_name,
    'full-name=s' => \$full_name,
    'version=s'   => \$version,
);
$project_name or die $usage;
$version      or die $usage;
$full_name ||= $project_name;

my @pod_infos;

find( { wanted => \&process_file, no_chdir => 1 }, 'lib' );

write_index();

sub process_file {
    my $filename = $_;
    my $dir      = $File::Find::topdir;

    return if -d $filename || $filename !~ /\.(pm|pod)\z/;
    my $content = read_file( $filename, binmode => ':utf8' );
    return if $filename =~ /\.pm$/ && $content !~ /^=head1/m;
    $filename =~ s|^$dir/||;

    if ( $content =~ /^=head1\s*NAME\s+(\S+)\s+-\s+(.*?)\s+^=/ms ) {
        push(@pod_infos, {
            class    => $1,
            desc     => $2,
            filename => $filename,
        });
    }
    else {
        print STDERR ("Warning: No valid NAME section found in $filename\n");
    }

    pod2mdtext( $dir, $filename );
};

sub pod2mdtext {
    my ( $base_dir, $filename ) = @_;

    my @path_comps = split('/', $filename);
    pop(@path_comps);

    my $out_dir = join('/', $out_root, @path_comps);
    make_path($out_dir);

    my $out_filename = "$out_root/$filename";
    $out_filename =~ s|(\.[^/.]*)?\z|.mdtext|;

    open( my $out_file, '>', $out_filename )
        or die("$out_filename: $!");

    my $p = Pod::Simple::HTML->new;
    my $after_title = " \x{2013} $full_name Documentation\n\n<div>\n";
    # Pod::Simple expects bytes.
    utf8::encode($after_title);

    $p->batch_mode(1);
    $p->batch_mode_current_level( scalar(@path_comps) + 1 );
    $p->html_header_before_title('Title: ');
    $p->html_header_after_title($after_title);
    $p->html_footer("\n</div>\n");
    $p->html_h_level(2);
    # Needed to make strip_verbatim_indent work, no idea why.
    $p->unaccept_codes('VerbatimFormatted');
    $p->strip_verbatim_indent('    ');

    $p->output_fh($out_file);
    $p->parse_file("$base_dir/$filename");

    close($out_file);
}

sub write_index {
    my $lc_project_name = lc($project_name);
    my $index_filename  = "$out_root/$lc_project_name-index.mdtext";
    open( my $index_file, '>:utf8', $index_filename )
        or die("$index_filename: $!");

    print $index_file (<<EOF);
Title: Perl API documentation for $full_name $version

## Perl API documentation for $full_name $version

EOF

    for my $pod_info ( sort { $a->{class} cmp $b->{class} } @pod_infos ) {
        my $class    = $pod_info->{class};
        my $desc     = $pod_info->{desc};
        my $filename = $pod_info->{filename};

        $filename =~ s|(\.[^/.]*)?\z|.html|;

        print $index_file ("- [$class]($filename) \x{2013} $desc\n");
    }

    close($index_filename);
}

