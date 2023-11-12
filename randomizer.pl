#!/usr/bin/perl

use JSON::PP;
use strict;

my $cplx = shift;

open RND, "< /dev/urandom";
my $buf;
sysread(RND, $buf, 4);
close RND;
my $seed = unpack('N', $buf);

srand($seed);
my $result = {};
my $steps = int(rand() * $cplx) + 32;
foreach my $step (0..$steps) {
  my $obj = $result;
  my $substeps = int(rand() * ($cplx / 16)) + 4;
  traverse_object($obj, $substeps);
}

print encode_json($result) . "\n";

exit 0;

##---- functions ----##

sub traverse_object
{
  my $obj = shift;
  my $n = shift;
  if ($n == 0) {
    return;
  }
  if (ref $obj eq 'HASH') {
    my @keys = keys(%{$obj});
    if ($n == 1 || scalar(@keys) < 2) {
      my $key = random_key($cplx / 16);
      my $value = random_value();
      $obj->{$key} = $value;
    } else {
      my $key = $keys[ int(rand(scalar(@keys))) ];
      $obj = $obj->{$key};
      traverse_object($obj, $n-1);
    }
  } elsif (ref $obj eq 'ARRAY') {
    if ($n == 1 || scalar(@{$obj}) < 2) {
      my $value = random_value();
      push @{$obj}, $value;
    } else {
      $obj = $obj->[ int(rand(scalar(@{$obj}))) ];
      traverse_object($obj, $n-1);
    }
  }
}

sub random_key
{
  my $n = shift;
  my $l = int(rand($n)) + 1;
  my $str = '';
  for (my $i=0; $i < $l; $i++) {
    $str .= ( 'a'..'z' )[ int(rand(26)) ];
  }
  return $str;
}

sub random_value
{
  my $type = int(rand(64));
  if ($type == 0) {
    return undef;
  } elsif ($type == 1) {
    return ( 'true', 'false' )[ int(rand(2)) ];
  } elsif ($type == 2) {
    return (int(rand(20000000)) / int(rand(20000000)));
  } elsif ($type == 3) {
    return int(rand(20000000));
  } elsif ($type > 4 && $type <= 8) {
    return random_key( int(rand(512)) + 8 );
  } elsif ($type > 8 && $type <= 32) {
    return [];
  } else {
    return {};
  }
}

1;
