#!/usr/bin/perl
use HTML::Parser();
use strict;
sub start_handler
{
    my ($tag, $self) = @_;
    print "START: \"".$tag."\"\n";
}

sub end_handler
{
    my ($tag, $self) = @_;
    print "END: \"".$tag."\"\n";
}

sub text_handler
{
    my ($text, $self) = @_;
    print $text;
}

sub comment_handler
{
    my ($text, $self) = @_;
    print "COMMENT: \"";
    foreach my $cur_comment (@$text) {
        print $cur_comment;
    }
    print "\"\n";
}

my $p = HTML::Parser->new(api_version => 3);
$p->handler( start => \&start_handler, "tagname,self");
$p->handler( end => \&end_handler, "tagname,self");
$p->handler( text => \&text_handler, "dtext, self");
$p->handler( comment => \&comment_handler, "tokens, self");
$p->parse_file(shift || die) || die $!;

