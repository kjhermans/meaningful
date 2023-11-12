#!/usr/bin/perl

use JSON::PP;
use strict;

my $jsonfile = shift @ARGV;
my $json = `cat $jsonfile`;
my $obj = decode_json($json);
my $der = encode_der($obj);

syswrite(STDOUT, $der);

exit 0;

##---- functions ----##

sub encode_der
{
  my $obj = shift;
  if (!defined($obj)) {
    return chr(5) . chr(0);
  } elsif (ref $obj eq 'HASH') {
    my $str = '';
    foreach my $key (keys(%{$obj})) {
      my $val = $obj->{$key};
      $str .= encode_der( [ $key, $val ] );
    }
    return chr(0x30) . encode_length(length($str)) . $str;
  } elsif (ref $obj eq 'ARRAY') {
    my $str = '';
    foreach my $elt (@{$obj}) {
      $str .= encode_der($elt);
    }
    return chr(0x30) . encode_length(length($str)) . $str;
  } elsif ($obj =~ /^([0-9]+\.){2,}[0-9]+$/) {
    return encode_oid($obj);
  } elsif ($obj =~ /^[0-9]*\.[0-9]+$/) {
  } elsif ($obj =~ /^[0-9]+/) {
    return encode_int(int($obj));
  } else {
    return chr(4) . encode_length(length("$obj")) . "$obj";
  }
}

sub encode_int
{
  my $int = shift;
  my $str = '';
  my $sign = 0;
  if ($int < 0) {
    $int = -$int;
    $sign = 1;
  }
  my @bytes;
  while ($int) {
    my $byte = $int & 0xff;
    $int >>= 8;
    unshift @bytes, $byte;
  }
  if ($bytes[0] & 0x80) {
    unshift @bytes, 0;
  }
  if ($sign) {
    for (my $i=0; $i < scalar(@bytes); $i++) {
      $bytes[ $i ] = ~$bytes[ $i ];
    }
  }
  my $str = '';
  foreach my $byte (@bytes) {
    $str .= chr($byte);
  }
  return chr(0x02) . encode_length(length($str)) . $str;
}

sub encode_oid
{
  my $obj = shift;
  my @elts = split($obj, /\./);
  my $str = chr($elts[0] * 40 + $elts[1]);
  shift @elts;
  shift @elts;
  while (scalar(@elts)) {
    my $elt = shift @elts;
    my @bytes;
    while ($elt) {
      my $byte = $elt & 0x7f;
      $elt >>= 7;
      unshift @bytes, $byte;
    }
    while (scalar(@bytes)) {
      my $byte = pop @bytes;
      if (scalar(@bytes)) {
        $str .= chr(0x80 | $byte);
      } else {
        $str .= chr($byte);
      }
    }
  }
  return chr(0x06) . encode_length(length($str)) . $str;
}

sub encode_length
{
  my $l = shift;
  if ($l <= 127) {
    return chr($l);
  } else {
    my $rep = pack('N', $l);
    if ($l >= 2 ** 24) {
      return chr(0x83) . substr($rep, 1, 3);
    } elsif ($l >= 2 ** 16) {
      return chr(0x82) . substr($rep, 2, 2);
    } else {
      return chr(0x81) . substr($rep, 3, 1);
    }
  }
}
