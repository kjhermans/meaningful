#!/usr/bin/perl

my $n = shift @ARGV;
my $str = 'a' x ($n * 4);

print '{ "string": "' . $str . '" }';

1;
