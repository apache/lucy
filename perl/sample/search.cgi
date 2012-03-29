#!/usr/local/bin/perl -T

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

# (Change configuration variables as needed.)
my $path_to_index = '/path/to/index';

use CGI;
use List::Util qw( max min );
use POSIX qw( ceil );
use Encode qw( decode );
use Lucy::Search::IndexSearcher;
use Lucy::Highlight::Highlighter;
use Lucy::Search::QueryParser;
use Lucy::Search::TermQuery;
use Lucy::Search::ANDQuery;

my $cgi       = CGI->new;
my $q         = decode( "UTF-8", $cgi->param('q') || '' );
my $offset    = decode( "UTF-8", $cgi->param('offset') || 0 );
my $category  = decode( "UTF-8", $cgi->param('category') || '' );
my $page_size = 10;

# Create an IndexSearcher and a QueryParser.
my $searcher = Lucy::Search::IndexSearcher->new( 
    index => $path_to_index,
);
my $qparser = Lucy::Search::QueryParser->new( 
    schema => $searcher->get_schema,
);

# Build up a Query.
my $query = $qparser->parse($q);
if ($category) {
    my $category_query = Lucy::Search::TermQuery->new(
        field => 'category', 
        term  => $category,
    );
    $query = Lucy::Search::ANDQuery->new(
        children => [ $query, $category_query ]
    );
}

# Execute the Query and get a Hits object.
my $hits = $searcher->hits(
    query      => $query,
    offset     => $offset,
    num_wanted => $page_size,
);
my $hit_count = $hits->total_hits;

# Arrange for highlighted excerpts to be created.
my $highlighter = Lucy::Highlight::Highlighter->new(
    searcher => $searcher,
    query    => $q,
    field    => 'content'
);

# Create result list.
my $report = '';
while ( my $hit = $hits->next ) {
    my $score   = sprintf( "%0.3f", $hit->get_score );
    my $excerpt = $highlighter->create_excerpt($hit);
    $report .= qq|
        <p>
          <a href="$hit->{url}"><strong>$hit->{title}</strong></a>
          <em>$score</em>
          <br />
          $excerpt
          <br />
          <span class="excerptURL">$hit->{url}</span>
        </p>
    |;
}

#--------------------------------------------------------------------#
# No Lucy tutorial material below this point - just html generation. #
#--------------------------------------------------------------------#

# Generate html, print and exit.
my $paging_links = generate_paging_info( $q, $hit_count );
my $cat_select = generate_category_select($category);
blast_out_content( $q, $report, $paging_links, $cat_select );

# Create html fragment with links for paging through results n-at-a-time.
sub generate_paging_info {
    my ( $query_string, $total_hits ) = @_;
    my $escaped_q = CGI::escapeHTML($query_string);
    my $paging_info;
    if ( !length $query_string ) {
        # No query?  No display.
        $paging_info = '';
    }
    elsif ( $total_hits == 0 ) {
        # Alert the user that their search failed.
        $paging_info
            = qq|<p>No matches for <strong>$escaped_q</strong></p>|;
    }
    else {
        # Calculate the nums for the first and last hit to display.
        my $last_result = min( ( $offset + $page_size ), $total_hits );
        my $first_result = min( ( $offset + 1 ), $last_result );

        # Display the result nums, start paging info.
        $paging_info = qq|
            <p>
                Results <strong>$first_result-$last_result</strong> 
                of <strong>$total_hits</strong> 
                for <strong>$escaped_q</strong>.
            </p>
            <p>
                Results Page:
            |;

        # Calculate first and last hits pages to display / link to.
        my $current_page = int( $first_result / $page_size ) + 1;
        my $last_page    = ceil( $total_hits / $page_size );
        my $first_page   = max( 1, ( $current_page - 9 ) );
        $last_page = min( $last_page, ( $current_page + 10 ) );

        # Create a url for use in paging links.
        my $href = $cgi->url( -relative => 1 );
        $href .= "?q=" . CGI::escape($query_string);
        $href .= ";category=" . CGI::escape($category);
        $href .= ";offset=" . CGI::escape($offset);

        # Generate the "Prev" link.
        if ( $current_page > 1 ) {
            my $new_offset = ( $current_page - 2 ) * $page_size;
            $href =~ s/(?<=offset=)\d+/$new_offset/;
            $paging_info .= qq|<a href="$href">&lt;= Prev</a>\n|;
        }

        # Generate paging links.
        for my $page_num ( $first_page .. $last_page ) {
            if ( $page_num == $current_page ) {
                $paging_info .= qq|$page_num \n|;
            }
            else {
                my $new_offset = ( $page_num - 1 ) * $page_size;
                $href =~ s/(?<=offset=)\d+/$new_offset/;
                $paging_info .= qq|<a href="$href">$page_num</a>\n|;
            }
        }

        # Generate the "Next" link.
        if ( $current_page != $last_page ) {
            my $new_offset = $current_page * $page_size;
            $href =~ s/(?<=offset=)\d+/$new_offset/;
            $paging_info .= qq|<a href="$href">Next =&gt;</a>\n|;
        }

        # Close tag.
        $paging_info .= "</p>\n";
    }

    return $paging_info;
}

# Build up the HTML "select" object for the "category" field.
sub generate_category_select {
    my $cat = shift;
    my $select = qq|
      <select name="category">
        <option value="">All Sections</option>
        <option value="article">Articles</option>
        <option value="amendment">Amendments</option>
      </select>|;
    if ($cat) {
        $select =~ s/"$cat"/"$cat" selected/;
    }
    return $select;
}

# Print content to output.
sub blast_out_content {
    my ( $query_string, $hit_list, $paging_info, $category_select ) = @_;
    my $escaped_q = CGI::escapeHTML($query_string);
    binmode( STDOUT, ":encoding(UTF-8)" );
    print qq|Content-type: text/html; charset=UTF-8\n\n|;
    print qq|
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
    "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
  <meta http-equiv="Content-type" 
    content="text/html;charset=UTF-8">
  <link rel="stylesheet" type="text/css" 
    href="/us_constitution/uscon.css">
  <title>Lucy: $escaped_q</title>
</head>

<body>

  <div id="navigation">
    <form id="usconSearch" action="">
      <strong>
        Search the 
        <a href="/us_constitution/index.html">US Constitution</a>:
      </strong>
      <input type="text" name="q" id="q" value="$escaped_q">
      $category_select
      <input type="submit" value="=&gt;">
    </form>
  </div><!--navigation-->

  <div id="bodytext">

  $hit_list

  $paging_info

  </div><!--bodytext-->
    <p style="font-size: smaller; color: #666">
      <em>
        Powered by <a href="http://lucy.apache.org/"
        >Apache Lucy<small><sup>TM</sup></small></a>
      </em>
    </p>
  </div><!--bodytext-->

</body>

</html>
|;
}

