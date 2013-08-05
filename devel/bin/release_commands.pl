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
use Getopt::Long qw( GetOptions );

my $usage = join( ' ',
    $0, qq|--version=X.Y.Z-rcN|, qq|--apache-id=APACHE_ID|,
    qq|--name="FULL NAME"| )
    . "\n";

my ( $full_rc_version, $apache_id, $name );
GetOptions(
    'version=s'   => \$full_rc_version,
    'apache-id=s' => \$apache_id,
    'name=s'      => \$name,
);
$full_rc_version or die $usage;
$apache_id       or die $usage;
$name            or die $usage;
$apache_id =~ /^\w+$/ or die $usage;
$full_rc_version =~ m/^(\d+)\.(\d+)\.(\d+)-rc(\d+)$/ or die $usage;
my ( $major, $minor, $micro, $rc ) = ( $1, $2, $3, $4 );
my $x_y_z_version = sprintf( "%d.%d.%d", $major, $minor, $micro );

say qq|###############################################################|;
say qq|# Commands to execute ReleaseGuide for |
    . qq|Apache Lucy $x_y_z_version RC $rc|;
say qq|###############################################################\n|;

say qq|# If your code signing key is not already available from pgp.mit.edu|;
say qq|# and <http://www.apache.org/dist/lucy/KEYS>, publish it.|;
say qq|[...]\n|;

say qq|# chdir to the top level of your local copy of the Lucy repository.|;
say qq|[...]\n|;

if ( $micro == 0 && $rc < 2 ) {
    say qq|# Check out the master branch.|;
    say qq|git checkout master\n|;
}
else {
    say qq|# Check out the $major.$minor branch.|;
    say qq|git checkout $major.$minor\n|;
}

if ( $rc < 2 ) {
    say qq|# Since this is the first RC, run update_version.|;
    say qq|./devel/bin/update_version $x_y_z_version\n|;
    say qq|# Update the the CHANGES file and associate release|;
    say qq|# $x_y_z_version with today's date.|;
    say qq|[...]\n|;
    say qq|# Commit version bump and CHANGES.|;
    say qq|git commit -m "Updating CHANGES and version number |
        . qq|for release $x_y_z_version."\n|;
}

if ( $micro == 0 && $rc < 2 ) {
    say qq|# Since this is the first release in a series (i.e. X.Y.0),|;
    say qq|# create a branch.|;
    say qq|git checkout -b $major.$minor|;
}

say qq|# Create a tag for the release candidate.|;
say qq|git tag apache-lucy-$full_rc_version |
    . qq|-m "Tagging release candidate $rc for $x_y_z_version."\n|;

say qq|# Export a pristine copy of the source from the release candidate|;
say qq|# tag.|;
say qq|git archive --prefix=apache-lucy-$x_y_z_version/ |
    . qq|--output=apache-lucy-$x_y_z_version.tar.gz |
    . qq|apache-lucy-$full_rc_version\n|;

say qq|# Create an RC directory in our dev area on dist.apache.org|;
say qq|# and check out a copy.|;
say qq|svn mkdir -m "Create RC dir for apache-lucy-$full_rc_version" |
    . qq|https://dist.apache.org/repos/dist/dev/lucy/apache-lucy-$full_rc_version|;
say qq|svn co |
    . qq|https://dist.apache.org/repos/dist/dev/lucy/apache-lucy-$full_rc_version\n|;

say qq|# Move the tarball and a copy of the CHANGES file into the RC dir,|;
say qq|# then chdir into it.|;
say qq|mv apache-lucy-$x_y_z_version.tar.gz apache-lucy-$full_rc_version|;
say qq|cp -p CHANGES apache-lucy-$full_rc_version/CHANGES-$x_y_z_version.txt|;
say qq|cd apache-lucy-$full_rc_version\n|;

say qq|# Generate checksums.|;
say qq|perl -MDigest -e '\$d = Digest->new("MD5"); open \$fh, |
    . qq|"<apache-lucy-$x_y_z_version.tar.gz" or die; |
    . qq|\$d->addfile(\$fh); print \$d->hexdigest; |
    . qq|print "  apache-lucy-$x_y_z_version.tar.gz\\n"' > |
    . qq| apache-lucy-$x_y_z_version.tar.gz.md5|;
say qq|perl -MDigest -e '\$d = Digest->new("SHA-512"); open \$fh, |
    . qq|"<apache-lucy-$x_y_z_version.tar.gz" or die; |
    . qq|\$d->addfile(\$fh); print \$d->hexdigest; |
    . qq|print "  apache-lucy-$x_y_z_version.tar.gz\\n"' > |
    . qq| apache-lucy-$x_y_z_version.tar.gz.sha\n|;

say qq|# Sign the release.|;
say qq|gpg --armor --output apache-lucy-$x_y_z_version.tar.gz.asc |
    . qq|--detach-sig apache-lucy-$x_y_z_version.tar.gz\n|;

say qq|# Add the artifacts and commit to the dev area on dist.apache.org.|;
say qq|svn add |
    . qq|apache-lucy-$x_y_z_version.tar.gz |
    . qq|apache-lucy-$x_y_z_version.tar.gz.md5 |
    . qq|apache-lucy-$x_y_z_version.tar.gz.sha |
    . qq|apache-lucy-$x_y_z_version.tar.gz.asc |
    . qq|CHANGES-$x_y_z_version.txt |;
say qq|svn ci -m "Add apache-lucy-$x_y_z_version artifacts"\n|;

say qq|# Perform whatever QC seems prudent on the tarball, installing it|;
say qq|# on test systems, etc.|;
say qq|[...]\n|;

say qq|# Push your branch and the tag for the RC.|;
say qq|git push origin $major.$minor|;
say qq|git push origin apache-lucy-$full_rc_version\n|;

say qq|###############################################################|;
say qq|# Voting|;
say qq|###############################################################\n|;

say qq|# Call a release vote on the dev list, referring to the artifacts|;
say qq|# made public in the previous step and using the boilerplate email|;
say qq|# below.|;
say qq|[...]\n|;

say qq|###############################################################|;
say qq|# After the vote has passed...|;
say qq|###############################################################\n|;

say qq|# Tag the release and delete the RC tags.|;
say qq|git tag apache-lucy-$x_y_z_version apache-lucy-$full_rc_version|;
say qq|git push origin apache-lucy-$x_y_z_version|;
for ( 1 .. $rc ) {
    my $rc_tag = qq|apache-lucy-$major.$minor.$micro-rc$_|;
    say qq|git tag -d $rc_tag|;
    say qq|git push origin :$rc_tag|;
}
say "";

say qq|# Copy release artifacts to the production dist directory and|;
say qq|# remove the RC dir.  The "svnmucc" app, which ships with Subversion|;
say qq|# 1.7, is required.  If you don't have it, you can ssh to|;
say qq|# people.apache.org and run the commands from there.|;
say qq|ssh $apache_id\@people.apache.org|;
say qq|svnmucc -m "Publish Apache Lucy $x_y_z_version" |
    . qq|-U https://dist.apache.org/repos/dist/ |
    . qq|mv dev/lucy/apache-lucy-$full_rc_version/apache-lucy-$x_y_z_version.tar.gz |
    . qq|release/lucy/apache-lucy-$x_y_z_version.tar.gz |
    . qq|mv dev/lucy/apache-lucy-$full_rc_version/apache-lucy-$x_y_z_version.tar.gz.md5 |
    . qq|release/lucy/apache-lucy-$x_y_z_version.tar.gz.md5 |
    . qq|mv dev/lucy/apache-lucy-$full_rc_version/apache-lucy-$x_y_z_version.tar.gz.sha |
    . qq|release/lucy/apache-lucy-$x_y_z_version.tar.gz.sha |
    . qq|mv dev/lucy/apache-lucy-$full_rc_version/apache-lucy-$x_y_z_version.tar.gz.asc |
    . qq|release/lucy/apache-lucy-$x_y_z_version.tar.gz.asc |
    . qq|mv dev/lucy/apache-lucy-$full_rc_version/CHANGES-$x_y_z_version.txt |
    . qq|release/lucy/CHANGES-$x_y_z_version.txt |
    . qq|rm dev/lucy/apache-lucy-$full_rc_version\n|;

say qq|# Carefully remove the artifacts for any previous releases superseded|;
say qq|# by this one.|;
if ( $micro > 0 ) {
    my $prev = sprintf( "%d.%d.%d", $major, $minor, $micro - 1 );
    say qq|svnmucc -m "Remove Apache Lucy $prev" |
        . qq|-U https://dist.apache.org/repos/dist/release/lucy/ |
        . qq|rm apache-lucy-$prev.tar.gz |
        . qq|rm apache-lucy-$prev.tar.gz.md5 |
        . qq|rm apache-lucy-$prev.tar.gz.sha |
        . qq|rm apache-lucy-$prev.tar.gz.asc |
        . qq|rm CHANGES-$prev.txt |;
}
say qq|[...]\n|;

say qq|# Update the issue tracker.|;
say qq|# While logged into JIRA, visit the following web page. (Note: this|;
say qq|# permalink may or may not work, and you may not have the necessary|;
say qq|# JIRA permissions to perform the required actions.  Please let the|;
say qq|# dev list know if you encounter problems.)  Click the "release"|;
say qq|# link for $x_y_z_version and input the date from the CHANGES file.|;
say qq|https://issues.apache.org/jira/plugins/servlet/|
    . qq|project-config/LUCY/versions\n|;

say qq|# Once the release files are in place, update the download page|;
say qq|# of the Lucy website. The easiest way to perform this action is to|;
say qq|# use the CMS bookmarklet at https://cms.apache.org/#bookmark|;
say qq|# to access the edit screens via the CMS web interface.  Change the|;
say qq|# artifact links to point at the new version; ensure that while the|;
say qq|# primary download links point at mirrors, the signature and sums|;
say qq|# files point at apache.org.|;
say qq|[...]\n|;

say qq|# [TODO: this action cannot yet be performed by the RM, so ignore.]|;
say qq|# Publish HTML exports of the documentation for the new release on|;
say qq|# the Lucy website.|;
say qq|[...]\n|;

say qq|# Send emails announcing the release to:|;
say qq|#|;
say qq|#     * The user list.|;
say qq|#     * The dev list.|;
say qq|#     * The announce\@a.o list.  Be sure to send from your|;
say qq|#       \@apache.org address|;
say qq|#|;
say qq|# Use the entry in the CHANGES file as the basis for your|;
say qq|# email, or optionally, use the boilerplate announcement text below.|;
say qq|[...]\n|;

say qq|###############################################################|;
say qq|# Boilerplate VOTE email for dev\@lucy.a.o|;
say qq|# Suggested subject:|;
say qq|#|;
say qq|#    [VOTE] Apache Lucy $x_y_z_version RC $rc|;
say qq|#|;
say qq|###############################################################\n|;

say <<END_LUCY_DEV_VOTE;
Hello,

Release candidate $rc for Apache Lucy version $x_y_z_version can be
found at:

    https://dist.apache.org/repos/dist/dev/lucy/apache-lucy-$full_rc_version/

See the CHANGES file at the top level of the archive for information
about the content of this release.

This candidate was assembled according to the process documented at:

    http://wiki.apache.org/lucy/ReleaseGuide

It was cut using "git archive" from the tag at:

    https://git-wip-us.apache.org/repos/asf?p=lucy.git;a=tag;h=refs/tags/apache-lucy-$full_rc_version

Please vote on releasing this candidate as Apache Lucy version
$x_y_z_version.  The vote will be held open for at least the next 72
hours.

All interested parties are welcome to inspect the release candidate
and express approval or disapproval.  Votes from members of the Lucy
PMC are binding; the vote passes if there are at least three binding
+1 votes and more +1 votes than -1 votes. 

For suggestions as to how to evaluate Apache Lucy release candidates,
and for information on ASF voting procedures, see:

    http://wiki.apache.org/lucy/ReleaseVerification
    http://wiki.apache.org/lucy/ReleasePrep
    http://www.apache.org/foundation/voting.html

[ ] +1 Release RC $rc as Apache Lucy $x_y_z_version.
[ ] +0
[ ] -1 Do not release RC $rc as Apache Lucy $x_y_z_version because...

Here's my +1.

Thanks!
END_LUCY_DEV_VOTE

say qq|###############################################################|;
say qq|# Boilerplate ANNOUNCE email|;
say qq|# Suggested subject:|;
say qq|#|;
say qq|#    [ANNOUNCE] Apache Lucy $x_y_z_version released|;
say qq|#|;
say qq|###############################################################\n|;

say <<END_ANNOUNCE_EMAIL;
Greetings,

The Apache Lucy team is pleased to announce the release of
version $x_y_z_version!

The Apache Lucy search engine library provides full-text search
for dynamic programming languages.  For a list of issues resolved
in this version, please see the release notes:

  http://www.apache.org/dist/lucy/CHANGES-$x_y_z_version.txt

The most recent release can be obtained from our download page:

  http://lucy.apache.org/download.html

For general information on Apache Lucy, please visit the project
website:

  http://lucy.apache.org/

Regards, 

$name, on behalf of the Apache Lucy development team
and community

END_ANNOUNCE_EMAIL

say qq|###############################################################|;

