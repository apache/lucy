# Bare-bones search app.

## Setup

Copy the text presentation of the US Constitution from the `sample` directory
of the Apache Lucy distribution to the base level of your web server's
`htdocs` directory.

    $ cp -R sample/us_constitution /usr/local/apache2/htdocs/

## Indexing: indexer.pl

Our first task will be to create an application called `indexer.pl` which
builds a searchable "inverted index" from a collection of documents.  

After we specify some configuration variables and load all necessary
modules...

``` c
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Simple.h"
#include "Lucy/Document/Doc.h"

const char path_to_index[] = "lucy_index";
const char uscon_source[]  = "../../common/sample/us_constitution";
```

``` perl
#!/usr/local/bin/perl
use strict;
use warnings;

# (Change configuration variables as needed.)
my $path_to_index = '/path/to/index';
my $uscon_source  = '/usr/local/apache2/htdocs/us_constitution';

use Lucy::Simple;
use File::Spec::Functions qw( catfile );
```

... we'll start by creating a [Lucy::Simple](lucy.Simple) object, telling it
where we'd like the index to be located and the language of the source
material.

``` c
int
main() {
    // Initialize the library.
    lucy_bootstrap_parcel();

    String *folder   = Str_newf("%s", path_to_index);
    String *language = Str_newf("en");
    Simple *lucy     = Simple_new((Obj*)folder, language);
```

``` perl
my $lucy = Lucy::Simple->new(
    path     => $path_to_index,
    language => 'en',
);
```

Next, we'll add a subroutine which parses our sample documents.

``` c
Doc*
S_parse_file(const char *filename) {
    size_t bytes = strlen(uscon_source) + 1 + strlen(filename) + 1;
    char *path = (char*)malloc(bytes);
    path[0] = '\0';
    strcat(path, uscon_source);
    strcat(path, "/");
    strcat(path, filename);

    FILE *stream = fopen(path, "r");
    if (stream == NULL) {
        perror(path);
        exit(1);
    }

    char *title    = NULL;
    char *bodytext = NULL;
    if (fscanf(stream, "%m[^\r\n] %m[\x01-\x7F]", &title, &bodytext) != 2) {
        fprintf(stderr, "Can't extract title/bodytext from '%s'", path);
        exit(1);
    }

    Doc *doc = Doc_new(NULL, 0);

    {
        // Store 'title' field
        String *field = Str_newf("title");
        String *value = Str_new_from_utf8(title, strlen(title));
        Doc_Store(doc, field, (Obj*)value);
        DECREF(field);
        DECREF(value);
    }

    {
        // Store 'content' field
        String *field = Str_newf("content");
        String *value = Str_new_from_utf8(bodytext, strlen(bodytext));
        Doc_Store(doc, field, (Obj*)value);
        DECREF(field);
        DECREF(value);
    }

    {
        // Store 'url' field
        String *field = Str_newf("url");
        String *value = Str_new_from_utf8(filename, strlen(filename));
        Doc_Store(doc, field, (Obj*)value);
        DECREF(field);
        DECREF(value);
    }

    fclose(stream);
    free(bodytext);
    free(title);
    free(path);
    return doc;
}
```

``` perl
# Parse a file from our US Constitution collection and return a hashref with
# the fields title, body, and url.
sub parse_file {
    my $filename = shift;
    my $filepath = catfile( $uscon_source, $filename );
    open( my $fh, '<', $filepath ) or die "Can't open '$filepath': $!";
    my $text = do { local $/; <$fh> };    # slurp file content
    $text =~ /\A(.+?)^\s+(.*)/ms
        or die "Can't extract title/bodytext from '$filepath'";
    my $title    = $1;
    my $bodytext = $2;
    return {
        title    => $title,
        content  => $bodytext,
        url      => "/us_constitution/$filename",
    };
}
```

Add some elementary directory reading code...

``` c
    DIR *dir = opendir(uscon_source);
    if (dir == NULL) {
        perror(uscon_source);
        return 1;
    }
```

``` perl
# Collect names of source files.
opendir( my $dh, $uscon_source )
    or die "Couldn't opendir '$uscon_source': $!";
my @filenames = grep { $_ =~ /\.txt/ } readdir $dh;
```

... and now we're ready for the meat of indexer.pl -- which occupies exactly
one line of code.

``` c
    for (struct dirent *entry = readdir(dir);
         entry;
         entry = readdir(dir)) {

        if (S_ends_with(entry->d_name, ".txt")) {
            Doc *doc = S_parse_file(entry->d_name);
            Simple_Add_Doc(lucy, doc); // ta-da!
            DECREF(doc);
        }
    }

    closedir(dir);

    DECREF(lucy);
    DECREF(language);
    DECREF(folder);
    return 0;
}
```

``` perl
foreach my $filename (@filenames) {
    my $doc = parse_file($filename);
    $lucy->add_doc($doc);  # ta-da!
}
```

## Search: search.cgi

As with our indexing app, the bulk of the code in our search script won't be
Lucy-specific.  

The beginning is dedicated to CGI processing and configuration.

``` c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Simple.h"

const char path_to_index[] = "lucy_index";

static void
S_usage_and_exit(const char *arg0) {
    printf("Usage: %s <querystring>\n", arg0);
    exit(1);
}

int
main(int argc, char *argv[]) {
    // Initialize the library.
    lucy_bootstrap_parcel();

    if (argc != 2) {
        S_usage_and_exit(argv[0]);
    }

    const char *query_c = argv[1];

    printf("Searching for: %s\n\n", query_c);
```

``` perl
#!/usr/local/bin/perl -T
use strict;
use warnings;

# (Change configuration variables as needed.)
my $path_to_index = '/path/to/index';

use CGI;
use List::Util qw( max min );
use POSIX qw( ceil );
use Encode qw( decode );
use Lucy::Simple;

my $cgi       = CGI->new;
my $q         = decode( "UTF-8", $cgi->param('q') || '' );
my $offset    = decode( "UTF-8", $cgi->param('offset') || 0 );
my $page_size = 10;
```

Once that's out of the way, we create our Lucy::Simple object and feed
it a query string.

``` c
    String *folder   = Str_newf("%s", path_to_index);
    String *language = Str_newf("en");
    Simple *lucy     = Simple_new((Obj*)folder, language);

    String *query_str = Str_newf("%s", query_c);
    Simple_Search(lucy, query_str, 0, 10);
```

``` perl
my $lucy = Lucy::Simple->new(
    path     => $path_to_index,
    language => 'en',
);
my $hit_count = $lucy->search(
    query      => $q,
    offset     => $offset,
    num_wanted => $page_size,
);
```

The value returned by [](lucy.Simple.Search) is the total number of documents
in the collection which matched the query.  We'll show this hit count to the
user, and also use it in conjunction with the parameters `offset` and
`num_wanted` to break up results into "pages" of manageable size.

Calling [](lucy.Simple.Search) on our Simple object turns it into an iterator.
Invoking [](lucy.Simple.Next) now returns hits one at a time as [](lucy.HitDoc)
objects, starting with the most relevant.

``` c
    String *title_str = Str_newf("title");
    String *url_str   = Str_newf("url");
    HitDoc *hit;
    int i = 1;

    // Loop over search results.
    while (NULL != (hit = Simple_Next(lucy))) {
        String *title = (String*)HitDoc_Extract(hit, title_str);
        char *title_c = Str_To_Utf8(title);

        String *url = (String*)HitDoc_Extract(hit, url_str);
        char *url_c = Str_To_Utf8(url);

        printf("Result %d: %s (%s)\n", i, title_c, url_c);

        free(url_c);
        free(title_c);
        DECREF(url);
        DECREF(title);
        DECREF(hit);
        i++;
    }

    DECREF(url_str);
    DECREF(title_str);
    DECREF(query_str);
    DECREF(lucy);
    DECREF(language);
    DECREF(folder);
    return 0;
}
```

``` perl
# Create result list.
my $report = '';
while ( my $hit = $lucy->next ) {
    my $score = sprintf( "%0.3f", $hit->get_score );
    $report .= qq|
        <p>
          <a href="$hit->{url}"><strong>$hit->{title}</strong></a>
          <em>$score</em>
          <br>
          <span class="excerptURL">$hit->{url}</span>
        </p>
        |;
}
```

The rest of the script is just text wrangling. 

``` perl
#---------------------------------------------------------------#
# No tutorial material below this point - just html generation. #
#---------------------------------------------------------------#

# Generate paging links and hit count, print and exit.
my $paging_links = generate_paging_info( $q, $hit_count );
blast_out_content( $q, $report, $paging_links );

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

# Print content to output.
sub blast_out_content {
    my ( $query_string, $hit_list, $paging_info ) = @_;
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
      <input type="submit" value="=&gt;">
    </form>
  </div><!--navigation-->

  <div id="bodytext">

  $hit_list

  $paging_info

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
```

## OK... now what?

Lucy::Simple is perfectly adequate for some tasks, but it's not very flexible.
Many people find that it doesn't do at least one or two things they can't live
without.

In our next tutorial chapter,
[](cfish:BeyondSimpleTutorial), we'll rewrite our
indexing and search scripts using the classes that Lucy::Simple hides
from view, opening up the possibilities for expansion; then, we'll spend the
rest of the tutorial chapters exploring these possibilities.

