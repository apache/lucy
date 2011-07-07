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
use Text::Wrap qw( wrap );

$Text::Wrap::unexpand = 0;    # tabs must die

my %section_headers = (
    Bug           => 'Bugfixes',
    Improvement   => 'Improvements',
    "New Feature" => 'New features',
    Task          => 'Tasks',
    Test          => 'Tests',
);

while (<>) {
    next unless /\S/;
    s/\s*$//;
    s/^\s*//;

    if (/^\s*Release Notes/) {
        # Start the entry with the release number.
        /(\d+\.\d+\.\d+)/ or die "Couldn't match version number in $_";
        my $version = $1;
        print "$1  XXXX-XX-XX\n";
    }
    elsif (/^\*\*\s*(.*)/) {
        # Start a new section.
        my $section_header = $section_headers{$1}
            or die "Unknown issue type: '$1'";
        print "\n";
        print "  $section_header:\n\n";
    }
    elsif (/\* (\[LUCY-\d+\] - )\s*(.*)/) {
        # Process an issue.
        my $issue_start   = "    * $1";
        my $title         = $2;
        my $indent_amount = length($issue_start);
        my $indent        = " " x $indent_amount;
        $Text::Wrap::columns = 78 - $indent_amount;
        $title = wrap( "", $indent, $title );
        print "$issue_start$title\n";
    }
    else {
        die "Unexpected line: $_";
    }
}

print "\n";

__END__

=head1 NAME

format_changes_entry.pl

=head1 SYNOPSIS

	cat jira_release_notes.txt | format_changes_entry.pl > changes_entry.txt

=head1 DESCRIPTION

JIRA can automatically generate a "Release Notes" in text format, but the
layout is less than ideal for the purposes of Lucy CHANGES entries and release
announcement emails.  This script makes the text more user-friendly by
wrapping long lines, collapsing whitespace and so on.

=cut

