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

use 5.010;
use strict;
use warnings;

my $usage = "$0 X.Y.Z-rcN\n";
my $full_rc_version = shift(@ARGV) or die $usage;

$full_rc_version =~ m/^(\d+)\.(\d+)\.(\d+)-rc(\d+)$/ or die $usage;
my ( $major, $minor, $micro, $rc ) = ( $1, $2, $3, $4 );
my $x_y_z_version = sprintf( "%d.%d.%d", $major, $minor, $micro );

say qq|#######################################################################|;
say qq|# Commands needed to execute ReleaseGuide for Apache Lucy $x_y_z_version RC $rc|;
say qq|#######################################################################\n|;

if ( $rc < 2 ) {
    say qq|# Run update_version.|;
    say qq|./devel/bin/update_version $x_y_z_version\n|;
    say qq|# Commit version bump and CHANGES.|;
    say qq|svn commit -m "Updating CHANGES and version number for release $x_y_z_version"\n|;
}

if ( $micro == 0 ) {
    say qq|# Branch for non-bugfix release.|;
    say qq|svn copy https://svn.apache.org/repos/asf/incubator/lucy/trunk |
        . qq|https://svn.apache.org/repos/asf/incubator/lucy/branches/$major.$minor |
        . qq|-m "Branching for $x_y_z_version release"\n|;
}

say qq|# Tag release candidate.|;
say
    qq|svn copy https://svn.apache.org/repos/asf/incubator/lucy/branches/$major.$minor |
    . qq|https://svn.apache.org/repos/asf/incubator/lucy/tags/apache-lucy-incubating-$full_rc_version |
    . qq|-m "Tagging release candidate $rc for $x_y_z_version"\n|;


say qq|# Export pristine source tree.|;
say qq|svn export https://svn.apache.org/repos/asf/incubator/lucy/tags/apache-lucy-incubating-$full_rc_version |
 . qq|apache-lucy-incubating-$x_y_z_version\n|;

say qq|# Tar and gzip the export.|;
say qq|tar -czf apache-lucy-incubating-$x_y_z_version.tar.gz apache-lucy-incubating-$x_y_z_version\n|;

say qq|# Generate MD5 checksum.|;
say qq|md5sum apache-lucy-incubating-$x_y_z_version.tar.gz > apache-lucy-incubating-$x_y_z_version.tar.gz.md5\n|;

say qq|# Sign the release.|;
say qq|gpg --armor --output apache-lucy-incubating-$x_y_z_version.tar.gz.asc |
 . qq|--detach-sig apache-lucy-incubating-$x_y_z_version.tar.gz\n|;

say qq|# Copy files to people.apache.org.|;
say qq|ssh people.apache.org|;
say qq|mkdir public_html/apache-lucy-incubating-$full_rc_version|;
say qq|exit|;
say qq|scp -p apache-lucy-incubating-$x_y_z_version.tar.gz* |
 . qq|people.apache.org:~/public_html/apache-lucy-incubating-$full_rc_version\n|;

say qq|# Modify permissions.|;
say qq|ssh people.apache.org|;
say qq|cd public_html/apache-lucy-incubating-$full_rc_version/|;
say qq|find . -type f -exec chmod 664 {} \;|;
say qq|find . -type d -exec chmod 775 {} \;|;
say qq|chgrp -R incubator *\n|;

say qq|# Tag release after all votes have passed.|;
say qq|svn copy https://svn.apache.org/repos/asf/incubator/lucy/tags/apache-lucy-incubating-$full_rc_version |
 . qq|https://svn.apache.org/repos/asf/incubator/lucy/tags/apache-lucy-incubating-$x_y_z_version |
 . qq|-m "Tagging release $x_y_z_version"\n|;


say qq|# Copy to dist directory, remove RC dir.|;
say qq|ssh people.apache.org|;
say qq|cd public_html/|;
say qq|cp -p apache-lucy-incubating-$full_rc_version/* /www/www.apache.org/dist/incubator/lucy/|;
say qq|rm -rf apache-lucy-incubating-$full_rc_version/\n|;

say qq|#######################################################################|;

