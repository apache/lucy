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

use strict;
use warnings;

package NameExtractor;

use base qw(Pod::Simple);

# Extraction of NAME sections. Possibly fragile.

sub _handle_element_start {
    my ($parser, $element_name, $attr_hash_r) = @_;

    if ($element_name eq 'head1') {
        $parser->{in_head1}      = 1;
        $parser->{head1_content} = '';
    }
}

sub _handle_text {
    my ($parser, $text) = @_;

    if ($parser->{in_head1}) {
        $parser->{head1_content} .= $text;
    }
    elsif ($parser->{after_head1}) {
        $parser->{extracted_name} = $text;
        $parser->{after_head1} = undef;
    }
}

sub _handle_element_end {
    my ($parser, $element_name, $attr_hash_r) = @_;

    if ($element_name eq 'head1') {
        if ($parser->{head1_content} eq 'NAME') {
            $parser->{after_head1} = 1;
        }
        $parser->{in_head1} = undef;
    }
}

package main;

use File::Find;
use File::Path qw(make_path);
use File::Slurp;
use Getopt::Std;
use Pod::Simple::HTML;

my $out_root = 'mdtext';

sub pod2mdtext {
    my ($base_dir, $filename) = @_;

    my @path_comps = split('/', $filename);
    pop(@path_comps);

    my $out_dir = join('/', $out_root, @path_comps);
    make_path($out_dir);

    my $out_filename = "$out_root/$filename";
    $out_filename =~ s"(\.[^/.]*)?$".mdtext";

    open(my $out_file, '>', $out_filename)
        or die("$out_filename: $!");

    my $p = Pod::Simple::HTML->new;

    $p->batch_mode(1);
    $p->batch_mode_current_level(scalar(@path_comps) + 1);
    $p->html_header_before_title('Title: ');
    $p->html_header_after_title(" - Apache Lucy Documentation\n\n<div>\n");
    $p->html_footer("\n</div>\n");
    $p->html_h_level(2);
    # Needed to make strip_verbatim_indent work, no idea why.
    $p->unaccept_codes('VerbatimFormatted');
    $p->strip_verbatim_indent('    ');

    $p->output_fh($out_file);
    $p->parse_file("$base_dir/$filename");

    close($out_file);

    my $name_ext = NameExtractor->new;
    $name_ext->parse_file("$base_dir/$filename");

    my $html_filename = $filename;
    $html_filename =~ s"(\.[^/.]*)?$".html";

    return {
        name          => $name_ext->{extracted_name},
        html_filename => $html_filename,
    };
}

my %opts;
getopt('v', \%opts);
my $version = $opts{v}
    or die("Usage: $0 -v version\n");

my @pod_infos;

for my $dir (qw(lib)) {
    my $wanted = sub {
        my $filename = $_;

        return if -d $filename;

        if ($filename =~ /\.pm$/) {
            my $content = read_file($filename);
            return unless $content =~ /^=head1/m;
        }
        elsif ($filename !~ /\.pod$/) {
            return;
        }

        $filename =~ s"^$dir/"";

        my $pod_info = pod2mdtext($dir, $filename);

        if (!defined($pod_info->{name})) {
            print STDERR (
                "Warning: Section NAME not found in ",
                $pod_info->{html_filename}, "\n",
            );
        }
        else {
            push(@pod_infos, $pod_info);
        }
    };

    find({ wanted => $wanted, no_chdir => 1 }, $dir);
}

my $index_filename = "$out_root/index.mdtext";
open(my $index_file, '>', $index_filename)
    or die("$index_filename: $!");

print $index_file (<<EOF);
Title: Perl API documentation for Apache Lucy $version

## Perl API documentation for Apache Lucy $version

EOF

for my $pod_info (sort { $a->{name} cmp $b->{name} } @pod_infos) {
    my $name          = $pod_info->{name};
    my $html_filename = $pod_info->{html_filename};

    if ($name !~ /(\S+)\s+-\s+(.*)/) {
        print STDERR ("Warning: Invalid NAME in $html_filename: $name\n");
        next;
    }

    my ($class, $desc) = ($1, $2);
    utf8::encode($desc);

    print $index_file (" - [$class]($html_filename) - $desc\n");
}

close($index_filename);

__END__

=head1 NAME

pod2mdtext.pl - Convert POD to mdtext for the Apache CMS

=head1 SYNOPSIS

    pod2mdtext.pl -v <version>

=head1 DESCRIPTION

This script creates mdtext files from POD. It must be run in the C<perl>
directory and scans all .pod files found in C<lib>. The resulting mdtext
files are stored in a directory named C<mdtext>.

=cut

