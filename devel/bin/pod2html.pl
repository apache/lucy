#!/usr/bin/perl

use File::Find;
use File::Path qw(make_path);
use File::Slurp;
use Pod::Simple::HTML;

sub pod2html {
    my ($base_dir, $filename) = @_;

    my @path_comps = split('/', $filename);
    pop(@path_comps);

    my $out_dir = join('/', 'html', @path_comps);
    make_path($out_dir);

    my $out_filename = "html/$filename";
    $out_filename =~ s"(\.[^/.]*)?$".mdtext";

    open(my $out_file, '>', $out_filename)
        or die("$out_filename: $!");

    my $p = Pod::Simple::HTML->new;

    $p->batch_mode(1);
    $p->batch_mode_current_level(scalar(@path_comps) + 1);
    $p->html_header_before_title('Title: ');
    $p->html_header_after_title(" - Apache Lucy Documentation\n\n<div>\n");
    $p->html_footer("\n</div>\n");

    $p->output_fh($out_file);
    $p->parse_file("$base_dir/$filename");

    close($out_file);
}

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

        pod2html($dir, $filename);
    };

    find({ wanted => $wanted, no_chdir => 1 }, $dir);
}

