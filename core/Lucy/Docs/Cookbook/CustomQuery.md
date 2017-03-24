# Sample subclass of Query

Explore Apache Lucy's support for custom query types by creating a
"PrefixQuery" class to handle trailing wildcards.

~~~ perl
my $prefix_query = PrefixQuery->new(
    field        => 'content',
    query_string => 'foo*',
);
my $hits = $searcher->hits( query => $prefix_query );
...
~~~

## Query, Compiler, and Matcher 

To add support for a new query type, we need three classes: a Query, a
Compiler, and a Matcher.  

* PrefixQuery - a subclass of [](cfish:lucy.Query), and the only class
  that client code will deal with directly.

* PrefixCompiler - a subclass of [](cfish:lucy.Compiler), whose primary 
  role is to compile a PrefixQuery to a PrefixMatcher.

* PrefixMatcher - a subclass of [](cfish:lucy.Matcher), which does the
  heavy lifting: it applies the query to individual documents and assigns a
  score to each match.

The PrefixQuery class on its own isn't enough because a Query object's role is
limited to expressing an abstract specification for the search.  A Query is
basically nothing but metadata; execution is left to the Query's companion
Compiler and Matcher.

Here's a simplified sketch illustrating how a Searcher's hits() method ties
together the three classes.

~~~ perl
sub hits {
    my ( $self, $query ) = @_;
    my $compiler = $query->make_compiler(
        searcher => $self,
        boost    => $query->get_boost,
    );
    my $matcher = $compiler->make_matcher(
        reader     => $self->get_reader,
        need_score => 1,
    );
    my @hits = $matcher->capture_hits;
    return \@hits;
}
~~~

### PrefixQuery

Our PrefixQuery class will have two attributes: a query string and a field
name.

~~~ perl
package PrefixQuery;
use base qw( Lucy::Search::Query );
use Carp;
use Scalar::Util qw( blessed );

# Inside-out member vars and hand-rolled accessors.
my %query_string;
my %field;
sub get_query_string { my $self = shift; return $query_string{$$self} }
sub get_field        { my $self = shift; return $field{$$self} }
~~~

PrefixQuery's constructor collects and validates the attributes.

~~~ perl
sub new {
    my ( $class, %args ) = @_;
    my $query_string = delete $args{query_string};
    my $field        = delete $args{field};
    my $self         = $class->SUPER::new(%args);
    confess("'query_string' param is required")
        unless defined $query_string;
    confess("Invalid query_string: '$query_string'")
        unless $query_string =~ /\*\s*$/;
    confess("'field' param is required")
        unless defined $field;
    $query_string{$$self} = $query_string;
    $field{$$self}        = $field;
    return $self;
}
~~~

Since this is an inside-out class, we'll need a destructor:

~~~ perl
sub DESTROY {
    my $self = shift;
    delete $query_string{$$self};
    delete $field{$$self};
    $self->SUPER::DESTROY;
}
~~~

The equals() method determines whether two Queries are logically equivalent:

~~~ perl
sub equals {
    my ( $self, $other ) = @_;
    return 0 unless blessed($other);
    return 0 unless $other->isa("PrefixQuery");
    return 0 unless $field{$$self} eq $field{$$other};
    return 0 unless $query_string{$$self} eq $query_string{$$other};
    return 1;
}
~~~

The last thing we'll need is a make_compiler() factory method which kicks out
a subclass of [](cfish:lucy.Compiler).

~~~ perl
sub make_compiler {
    my ( $self, %args ) = @_;
    return PrefixCompiler->new( %args, parent => $self );
}
~~~

### PrefixCompiler

PrefixQuery's make_compiler() method will be called internally at search-time
by objects which subclass [](cfish:lucy.Searcher) -- such as
[IndexSearchers](cfish:lucy.IndexSearcher).

A Searcher is associated with a particular collection of documents.   These
documents may all reside in one index, as with IndexSearcher, or they may be
spread out across multiple indexes on one or more machines, as with
LucyX::Remote::ClusterSearcher.

Searcher objects have access to certain statistical information about the
collections they represent; for instance, a Searcher can tell you how many
documents are in the collection...

~~~ perl
my $maximum_number_of_docs_in_collection = $searcher->doc_max;
~~~

... or how many documents a specific term appears in:

~~~ perl
my $term_appears_in_this_many_docs = $searcher->doc_freq(
    field => 'content',
    term  => 'foo',
);
~~~

Such information can be used by sophisticated Compiler implementations to
assign more or less heft to individual queries or sub-queries.  However, we're
not going to bother with weighting for this demo; we'll just assign a fixed
score of 1.0 to each matching document.

We don't need to write a constructor, as it will suffice to inherit new() from
Lucy::Search::Compiler.  The only method we need to implement for
PrefixCompiler is make_matcher().

~~~ perl
package PrefixCompiler;
use base qw( Lucy::Search::Compiler );

sub make_matcher {
    my ( $self, %args ) = @_;
    my $seg_reader = $args{reader};

    # Retrieve low-level components LexiconReader and PostingListReader.
    my $lex_reader
        = $seg_reader->obtain("Lucy::Index::LexiconReader");
    my $plist_reader
        = $seg_reader->obtain("Lucy::Index::PostingListReader");
    
    # Acquire a Lexicon and seek it to our query string.
    my $substring = $self->get_parent->get_query_string;
    $substring =~ s/\*.\s*$//;
    my $field = $self->get_parent->get_field;
    my $lexicon = $lex_reader->lexicon( field => $field );
    return unless $lexicon;
    $lexicon->seek($substring);
    
    # Accumulate PostingLists for each matching term.
    my @posting_lists;
    while ( defined( my $term = $lexicon->get_term ) ) {
        last unless $term =~ /^\Q$substring/;
        my $posting_list = $plist_reader->posting_list(
            field => $field,
            term  => $term,
        );
        if ($posting_list) {
            push @posting_lists, $posting_list;
        }
        last unless $lexicon->next;
    }
    return unless @posting_lists;
    
    return PrefixMatcher->new( posting_lists => \@posting_lists );
}
~~~

PrefixCompiler gets access to a [](cfish:lucy.SegReader)
object when make_matcher() gets called.  From the SegReader and its
sub-components [](cfish:lucy.LexiconReader) and
[](cfish:lucy.PostingListReader), we acquire a
[](cfish:lucy.Lexicon), scan through the Lexicon's unique
terms, and acquire a [](cfish:lucy.PostingList) for each
term that matches our prefix.

Each of these PostingList objects represents a set of documents which match
the query.

### PrefixMatcher

The Matcher subclass is the most involved.  

~~~ perl
package PrefixMatcher;
use base qw( Lucy::Search::Matcher );

# Inside-out member vars.
my %doc_ids;
my %tick;

sub new {
    my ( $class, %args ) = @_;
    my $posting_lists = delete $args{posting_lists};
    my $self          = $class->SUPER::new(%args);
    
    # Cheesy but simple way of interleaving PostingList doc sets.
    my %all_doc_ids;
    for my $posting_list (@$posting_lists) {
        while ( my $doc_id = $posting_list->next ) {
            $all_doc_ids{$doc_id} = undef;
        }
    }
    my @doc_ids = sort { $a <=> $b } keys %all_doc_ids;
    $doc_ids{$$self} = \@doc_ids;
    
    # Track our position within the array of doc ids.
    $tick{$$self} = -1;
    
    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $doc_ids{$$self};
    delete $tick{$$self};
    $self->SUPER::DESTROY;
}
~~~

The doc ids must be in order, or some will be ignored; hence the `sort`
above.

In addition to the constructor and destructor, there are three methods that
must be overridden.

next() advances the Matcher to the next valid matching doc.  

~~~ perl
sub next {
    my $self    = shift;
    my $doc_ids = $doc_ids{$$self};
    my $tick    = ++$tick{$$self};
    return 0 if $tick >= scalar @$doc_ids;
    return $doc_ids->[$tick];
}
~~~

get_doc_id() returns the current document id, or 0 if the Matcher is
exhausted.  ([Document numbers](cfish:DocIDs) start at 1, so 0 is
a sentinel.)

~~~ perl
sub get_doc_id {
    my $self    = shift;
    my $tick    = $tick{$$self};
    my $doc_ids = $doc_ids{$$self};
    return $tick < scalar @$doc_ids ? $doc_ids->[$tick] : 0;
}
~~~

score() conveys the relevance score of the current match.  We'll just return a
fixed score of 1.0:

~~~ perl
sub score { 1.0 }
~~~

## Usage 

To get a basic feel for PrefixQuery, insert the FlatQueryParser module
described in [](cfish:CustomQueryParser) (which supports
PrefixQuery) into the search.cgi sample app.

~~~ perl
my $parser = FlatQueryParser->new( schema => $searcher->get_schema );
my $query  = $parser->parse($q);
~~~

If you're planning on using PrefixQuery in earnest, though, you may want to
change up analyzers to avoid stemming, because stemming -- another approach to
prefix conflation -- is not perfectly compatible with prefix searches.

~~~ perl
# Polyanalyzer with no SnowballStemmer.
my $analyzer = Lucy::Analysis::PolyAnalyzer->new(
    analyzers => [
        Lucy::Analysis::StandardTokenizer->new,
        Lucy::Analysis::Normalizer->new,
    ],
);
~~~

