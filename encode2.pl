#!/usr/bin/perl

use Sarthaka;
use JSON::PP;

my $file = shift @ARGV;
my $json = `cat $file`;
my $obj = decode_json($json);

my $bin = Sarthaka::encode($obj);

syswrite(STDOUT, $bin);

1;
