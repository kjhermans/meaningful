#!/usr/bin/perl

use JSON::PP;
use strict;

my $jsonfile = shift;
my $json = `cat $jsonfile`;
my $object = decode_json($json);

my $bitpos = 7;
my $result = [];

encode($object);
foreach my $byte (@{$result}) {
  print chr($byte);
}

exit 0;

##---- functions ----##

sub encode
{
  my $obj = shift;
  if (ref $obj eq 'HASH') {
    addtype(7);
    my @keys = sort(keys(%{$obj}));
    for (my $i=0; $i < scalar(@keys); $i++) {
      my $key = $keys[ $i ];
      my $value = $obj->{$key};
      addbit(1);
      encode_scalar("$key");
      encode($value);
    }
    addbit(0);
  } elsif (ref $obj eq 'ARRAY') {
    addtype(6);
    for (my $i=0; $i < scalar(@{$obj}); $i++) {
      my $elt = $obj->[ $i ];
      addbit(1);
      encode($elt);
    }
    addbit(0);
  } elsif ($obj =~ /^(true|false)$/) {
    addtype(2);
    if ($obj eq 'true') {
      addbit(1);
    } else {
      addbit(0);
    }
  } elsif (!defined($obj)) {
    addtype(1);
  } elsif ($obj =~ /^[0-9]*\.[0-9]+$/) {
    addtype(4);
    my $bytes = pack('d', $obj);
    for (my $i=0; $i<length($bytes); $i++) {
      my $byte = ord(substr($bytes, $i, 1));
      addbyte($byte);
    }
  } elsif ($obj =~ /^[0-9]+$/) {
    addtype(3);
    my $bytes = pack('q', int($obj));
    for (my $i=0; $i<8; $i++) {
      my $byte = ord(substr($bytes, $i, 1));
      addbyte($byte);
    }
  } else {
    addtype(5);
    encode_scalar("$obj");
  }
}

sub encode_scalar
{
  my $str = shift;
  for (my $i=0; $i < length($str); $i++) {
    my $byte = ord(substr($str, $i, 1));
    addbit(1);
    addbyte($byte);
  }
  addbit(0);
}

sub encode_implicitnull
{
  addtype(0);
}

sub addtype
{
  my $type = shift;
  addbit(($type >> 2) & 0x01);
  addbit(($type >> 1) & 0x01);
  addbit(($type >> 0) & 0x01);
}

sub addbit
{
  my $bit = shift;
  ++$bitpos;
  if ($bitpos > 7) {
    push @{$result}, 0;
    $bitpos = 0;
  }
  my $byte = $result->[ -1 ];
  if ($bit) {
    $byte |= (1 << (7-$bitpos));
  }
  $result->[ -1 ] = $byte;
}

sub addbyte
{
  my $byte = shift;
  for (my $i=0; $i < 8; $i++) {
    my $bit = (($byte >> (7-$i)) & 0x01);
    addbit($bit);
  }
}
