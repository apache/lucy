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
use Data::Pageset;
use HTML::Entities qw( encode_entities );
use Lucy::Search::IndexSearcher;
use Lucy::Highlight::Highlighter;
use Lucy::Search::QueryParser;
use Lucy::Search::TermQuery;
use Lucy::Search::ANDQuery;

my $cgi           = CGI->new;
my $q             = $cgi->param('q') || '';
my $offset        = $cgi->param('offset') || 0;
my $category      = $cgi->param('category') || '';
my $hits_per_page = 10;

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
    num_wanted => $hits_per_page,
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
    my $title   = encode_entities( $hit->{title} );
    my $excerpt = $highlighter->create_excerpt($hit);
    $report .= qq|
        <p>
          <a href="$hit->{url}"><strong>$title</strong></a>
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
    $query_string = encode_entities($query_string);
    my $paging_info;
    if ( !length $query_string ) {
        # No query?  No display.
        $paging_info = '';
    }
    elsif ( $total_hits == 0 ) {
        # Alert the user that their search failed.
        $paging_info
            = qq|<p>No matches for <strong>$query_string</strong></p>|;
    }
    else {
        my $current_page = ( $offset / $hits_per_page ) + 1;
        my $pager        = Data::Pageset->new(
            {   total_entries    => $total_hits,
                entries_per_page => $hits_per_page,
                current_page     => $current_page,
                pages_per_set    => 10,
                mode             => 'slide',
            }
        );
        my $last_result  = $pager->last;
        my $first_result = $pager->first;

        # Display the result nums, start paging info.
        $paging_info = qq|
            <p>
                Results <strong>$first_result-$last_result</strong> 
                of <strong>$total_hits</strong> 
                for <strong>$query_string</strong>.
            </p>
            <p>
                Results Page:
            |;

        # Create a url for use in paging links.
        my $href = $cgi->url( -relative => 1 ) . "?" . $cgi->query_string;
        $href .= ";offset=0" unless $href =~ /offset=/;

        # Generate the "Prev" link.
        if ( $current_page > 1 ) {
            my $new_offset = ( $current_page - 2 ) * $hits_per_page;
            $href =~ s/(?<=offset=)\d+/$new_offset/;
            $paging_info .= qq|<a href="$href">&lt;= Prev</a>\n|;
        }

        # Generate paging links.
        for my $page_num ( @{ $pager->pages_in_set } ) {
            if ( $page_num == $current_page ) {
                $paging_info .= qq|$page_num \n|;
            }
            else {
                my $new_offset = ( $page_num - 1 ) * $hits_per_page;
                $href =~ s/(?<=offset=)\d+/$new_offset/;
                $paging_info .= qq|<a href="$href">$page_num</a>\n|;
            }
        }

        # Generate the "Next" link.
        if ( $current_page != $pager->last_page ) {
            my $new_offset = $current_page * $hits_per_page;
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
    $query_string = encode_entities($query_string);
    print "Content-type: text/html\n\n";
    print qq|
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
    "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
  <meta http-equiv="Content-type" 
    content="text/html;charset=ISO-8859-1">
  <link rel="stylesheet" type="text/css" 
    href="/us_constitution/uscon.css">
  <title>Lucy: $query_string</title>
</head>

<body>

  <div id="navigation">
    <form id="usconSearch" action="">
      <strong>
        Search the 
        <a href="/us_constitution/index.html">US Constitution</a>:
      </strong>
      <input type="text" name="q" id="q" value="$query_string">
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
        Powered by <a href="http://incubator.apache.org/lucy/"
        >Apache Lucy<small><sup>TM</sup></small></a>
      </em>
    </p>
  </div><!--bodytext-->

</body>

</html>
|;
}

