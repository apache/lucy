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

package BenchmarkingIndexer;

use Carp;
use Config;
use File::Spec::Functions qw( catfile catdir );
use POSIX qw( uname );

sub new {
    my $either = shift;
    my $class = ref($either) || $either;
    return bless {
        docs              => undef,
        increment         => undef,
        store             => undef,
        engine            => undef,
        version           => undef,
        index_dir         => undef,
        corpus_dir        => 'extracted_corpus',
        article_filepaths => undef,
        @_,
    }, $class;
}

sub init_indexer { confess "abstract method" }
sub build_index  { confess "abstract method" }

sub delayed_init {
    my $self              = shift;
    my $article_filepaths = $self->{article_filepaths}
        = $self->build_file_list;
    $self->{docs} = @$article_filepaths unless defined $self->{docs};
    $self->{increment} = $self->{docs} + 1 unless defined $self->{increment};
}

# Return a lexically sorted list of all article files from all subdirs.
sub build_file_list {
    my $self       = shift;
    my $corpus_dir = $self->{corpus_dir};
    my @article_filepaths;
    opendir CORPUS_DIR, $corpus_dir
        or confess "Can't opendir '$corpus_dir': $!";
    my @article_dir_names = grep {/articles/} readdir CORPUS_DIR;
    for my $article_dir_name (@article_dir_names) {
        my $article_dir = catdir( $corpus_dir, $article_dir_name );
        opendir ARTICLE_DIR, $article_dir
            or die "Can't opendir '$article_dir': $!";
        push @article_filepaths, map { catfile( $article_dir, $_ ) }
            grep {m/^article\d+\.txt$/} readdir ARTICLE_DIR;
    }
    @article_filepaths = sort @article_filepaths;
    $self->{article_filepaths} = \@article_filepaths;
}

# Print out stats for one run.
sub print_interim_report {
    my ( $self, %args ) = @_;
    printf( "%-3d  Secs: %.3f  Docs: %-4d\n", @args{qw( rep secs count )} );
}

sub start_report {
    # Start the output.
    print '-' x 60 . "\n";
}

# Print out aggregate stats.
sub print_final_report {
    my ( $self, $times ) = @_;

    # Produce mean and truncated mean.
    my @sorted_times = sort @$times;
    my $num_to_chop  = int( @sorted_times >> 2 );
    my $mean         = 0;
    my $trunc_mean   = 0;
    my $num_kept     = 0;
    for ( my $i = 0; $i < @sorted_times; $i++ ) {
        $mean += $sorted_times[$i];
        # Discard fastest 25% and slowest 25% of runs.
        next if $i < $num_to_chop;
        next if $i > ( $#sorted_times - $num_to_chop );
        $trunc_mean += $sorted_times[$i];
        $num_kept++;
    }

    $mean       /= @sorted_times;
    $trunc_mean /= $num_kept;
    my $num_discarded = @sorted_times - $num_kept;
    $mean       = sprintf( "%.3f", $mean );
    $trunc_mean = sprintf( "%.3f", $trunc_mean );

    # Get some info about the system.
    my $thread_support = $Config{usethreads} ? "yes" : "no";
    my @uname_info = (uname)[ 0, 2, 4 ];

    print <<END_REPORT;
------------------------------------------------------------
$self->{engine} $self->{version} 
Perl $Config{version}
Thread support: $thread_support
@uname_info
Mean: $mean secs 
Truncated mean ($num_kept kept, $num_discarded discarded): $trunc_mean secs
------------------------------------------------------------
END_REPORT
}

package BenchSchema::WhiteSpaceTokenizer;
use base qw( Lucy::Analysis::RegexTokenizer );

sub new { return shift->SUPER::new( pattern => '\S+' ) }

package BenchSchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::RegexTokenizer;

sub new {
    my $self = shift->SUPER::new;
    my $type = Lucy::Plan::FullTextType->new(
        analyzer => BenchSchema::WhiteSpaceTokenizer->new, );
    $self->spec_field( name => 'title', type => $type );
    return $self;
}

package BenchmarkingIndexer::Lucy;
use base qw( BenchmarkingIndexer );

use Time::HiRes qw( gettimeofday );

sub new {
    my $class = shift;
    my $self  = $class->SUPER::new(@_);

    require Lucy;
    require Lucy::Index::Indexer;

    # Provide runtime flexibility.
    my $schema = $self->{schema} = BenchSchema->new;
    my $body_type = Lucy::Plan::FullTextType->new(
        analyzer      => BenchSchema::WhiteSpaceTokenizer->new,
        highlightable => $self->{store} ? 1 : 0,
        stored        => $self->{store} ? 1 : 0,
    );
    $schema->spec_field( name => 'body', type => $body_type );

    $self->{index_dir} = 'lucy_index';
    $self->{engine}    = 'Lucy';
    $self->{version}   = $Lucy::VERSION;

    return $self;
}

sub init_indexer {
    my ( $self, $count ) = @_;
    my $truncate = $count == 0 ? 1 : 0;
    return Lucy::Index::Indexer->new(
        schema   => $self->{schema},
        index    => $self->{index_dir},
        truncate => $truncate,
        create   => 1,
    );
}

# Build an index, stopping at $max docs if $max > 0.
sub build_index {
    my $self = shift;
    $self->delayed_init;
    my ( $max, $increment, $article_filepaths )
        = @{$self}{qw( docs increment article_filepaths )};

    # Start timer.
    my $start = gettimeofday();

    my $indexer = $self->init_indexer(0);

    my $count = 0;
    while ( $count < $max ) {
        for my $article_filepath (@$article_filepaths) {
            # The title is the first line, the body is the rest.
            open( my $article_fh, '<', $article_filepath )
                or die "Can't open file '$article_filepath'";

            my %doc;
            $doc{title} = <$article_fh>;
            $doc{body} = do { local $/; <$article_fh> };

            $indexer->add_doc( \%doc );

            # Bail if we've reached spec'd number of docs.
            $count++;
            last if $count >= $max;
            if ( $count % $increment == 0 and $count ) {
                $indexer->commit;
                undef $indexer;
                $indexer = $self->init_indexer($count);
            }
        }
    }

    # Finish index.
    $indexer->optimize;
    $indexer->commit;

    # Return elapsed seconds.
    my $end  = gettimeofday();
    my $secs = $end - $start;
    return ( $count, $secs );
}

1;
