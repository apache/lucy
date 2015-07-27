# A more flexible app structure.

## Goal

In this tutorial chapter, we'll refactor the apps we built in
[](cfish:SimpleTutorial) so that they look exactly the same from
the end user's point of view, but offer the developer greater possibilites for
expansion.  

To achieve this, we'll ditch Lucy::Simple and replace it with the
classes that it uses internally:

* [](cfish:lucy.Schema) - Plan out your index.
* [](cfish:lucy.FullTextType) - Field type for full text search.
* [](cfish:lucy.EasyAnalyzer) - A one-size-fits-all parser/tokenizer.
* [](cfish:lucy.Indexer) - Manipulate index content.
* [](cfish:lucy.IndexSearcher) - Search an index.
* [](cfish:lucy.Hits) - Iterate over hits returned by a Searcher.

## Adaptations to indexer.pl

After we load our modules...

``` c
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Analysis/EasyAnalyzer.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/Indexer.h"
#include "Lucy/Plan/FullTextType.h"
#include "Lucy/Plan/StringType.h"
#include "Lucy/Plan/Schema.h"

const char path_to_index[] = "/path/to/index";
const char uscon_source[]  = "/usr/local/apache2/htdocs/us_constitution";
```

``` perl
use Lucy::Plan::Schema;
use Lucy::Plan::FullTextType;
use Lucy::Analysis::EasyAnalyzer;
use Lucy::Index::Indexer;
```

... the first item we're going need is a [](cfish:lucy.Schema).

The primary job of a Schema is to specify what fields are available and how
they're defined.  We'll start off with three fields: title, content and url.

``` c
static Schema*
S_create_schema() {
    // Create a new schema.
    Schema *schema = Schema_new();

    // Create an analyzer.
    String       *language = Str_newf("en");
    EasyAnalyzer *analyzer = EasyAnalyzer_new(language);

    // Specify fields.

    FullTextType *type = FullTextType_new((Analyzer*)analyzer);

    {
        String *field_str = Str_newf("title");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("content");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

    {
        String *field_str = Str_newf("url");
        Schema_Spec_Field(schema, field_str, (FieldType*)type);
        DECREF(field_str);
    }

    DECREF(type);
    DECREF(analyzer);
    DECREF(language);
    return schema;
}
```

``` perl
# Create Schema.
my $schema = Lucy::Plan::Schema->new;
my $easyanalyzer = Lucy::Analysis::EasyAnalyzer->new(
    language => 'en',
);
my $type = Lucy::Plan::FullTextType->new(
    analyzer => $easyanalyzer,
);
$schema->spec_field( name => 'title',   type => $type );
$schema->spec_field( name => 'content', type => $type );
$schema->spec_field( name => 'url',     type => $type );
```

All of the fields are spec'd out using the [](lucy.FullTextType) FieldType,
indicating that they will be searchable as "full text" -- which means that
they can be searched for individual words.  The "analyzer", which is unique to
FullTextType fields, is what breaks up the text into searchable tokens.

Next, we'll swap our Lucy::Simple object out for an [](lucy.Indexer).
The substitution will be straightforward because Simple has merely been
serving as a thin wrapper around an inner Indexer, and we'll just be peeling
away the wrapper.

First, replace the constructor:

``` c
int
main() {
    // Initialize the library.
    lucy_bootstrap_parcel();

    Schema *schema = S_create_schema();
    String *folder = Str_newf("%s", path_to_index);

    Indexer *indexer = Indexer_new(schema, (Obj*)folder, NULL,
                                   Indexer_CREATE | Indexer_TRUNCATE);

```

``` perl
# Create Indexer.
my $indexer = Lucy::Index::Indexer->new(
    index    => $path_to_index,
    schema   => $schema,
    create   => 1,
    truncate => 1,
);
```

Next, have the `indexer` object [](cfish:lucy.Indexer.Add_Doc) where we
were having the `lucy` object adding the document before:

``` c
    DIR *dir = opendir(uscon_source);
    if (dir == NULL) {
        perror(uscon_source);
        return 1;
    }

    for (struct dirent *entry = readdir(dir);
         entry;
         entry = readdir(dir)) {

        if (S_ends_with(entry->d_name, ".txt")) {
            Doc *doc = S_parse_file(entry->d_name);
            Indexer_Add_Doc(indexer, doc, 1.0);
            DECREF(doc);
        }
    }

    closedir(dir);
```

``` perl
foreach my $filename (@filenames) {
    my $doc = parse_file($filename);
    $indexer->add_doc($doc);
}
```

There's only one extra step required: at the end of the app, you must call
commit() explicitly to close the indexing session and commit your changes.
(Lucy::Simple hides this detail, calling commit() implicitly when it needs to).

``` c
    Indexer_Commit(indexer);

    DECREF(indexer);
    DECREF(folder);
    DECREF(schema);
    return 0;
}
```

``` perl
$indexer->commit;
```

## Adaptations to search.cgi

In our search app as in our indexing app, Lucy::Simple has served as a
thin wrapper -- this time around [](cfish:lucy.IndexSearcher) and
[](cfish:lucy.Hits).  Swapping out Simple for these two classes is
also straightforward:

``` c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/String.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Search/Hits.h"
#include "Lucy/Search/IndexSearcher.h"

const char path_to_index[] = "/path/to/index";

int
main(int argc, char *argv[]) {
    // Initialize the library.
    lucy_bootstrap_parcel();

    if (argc < 2) {
        printf("Usage: %s <querystring>\n", argv[0]);
        return 0;
    }

    const char *query_c = argv[1];

    printf("Searching for: %s\n\n", query_c);

    String        *folder   = Str_newf("%s", path_to_index);
    IndexSearcher *searcher = IxSearcher_new((Obj*)folder);

    String *query_str = Str_newf("%s", query_c);
    Hits *hits = IxSearcher_Hits(searcher, (Obj*)query_str, 0, 10, NULL);

    String *title_str = Str_newf("title");
    String *url_str   = Str_newf("url");
    HitDoc *hit;
    int i = 1;

    // Loop over search results.
    while (NULL != (hit = Hits_Next(hits))) {
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
    DECREF(hits);
    DECREF(query_str);
    DECREF(searcher);
    DECREF(folder);
    return 0;
}
```

``` perl
use Lucy::Search::IndexSearcher;

my $searcher = Lucy::Search::IndexSearcher->new( 
    index => $path_to_index,
);
my $hits = $searcher->hits(    # returns a Hits object, not a hit count
    query      => $q,
    offset     => $offset,
    num_wanted => $page_size,
);
my $hit_count = $hits->total_hits;  # get the hit count here

...

while ( my $hit = $hits->next ) {
    ...
}
```

## Hooray!

Congratulations!  Your apps do the same thing as before... but now they'll be
easier to customize.  

In our next chapter, [](cfish:FieldTypeTutorial), we'll explore
how to assign different behaviors to different fields.


