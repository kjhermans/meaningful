#!/usr/bin/perl

use strict;
use JSON::PP;

my $file = shift;
my $input = absorb_binary($file);

my $bytepos = 0;
my $bitpos = 0;
my $finished = 0;
my $doublesize; { my $s = pack('d', 3.141592); $doublesize = length($s); }

my @bottomlist;

while (!$finished) {
  my $tuple = popany();
  push @bottomlist, $tuple;
}

while (scalar(@bottomlist) && $bottomlist[-1]->{type} eq 'implicitnull') {
  pop @bottomlist;
}
my $result = \@bottomlist;
if (scalar(@bottomlist) == 1) {
  $result = $bottomlist[ 0 ];
} else {
  $result = { type => 'array', value => $result };
}

#use Data::Dumper; print STDERR Dumper $result;
my $result = rework_obj($result);

my $json = encode_json($result);
print $json . "\n";

exit 0;

##---- functions ----##

sub popany
{
  my $type = poptype();
  if ($type == 0) {
    return { type => 'implicitnull' };
  } elsif ($type == 1) {
    return { type => 'explicitnull' };
  } elsif ($type == 2) {
    my $bool = getbits(1);
    return { type => 'boolean', value => $bool };
  } elsif ($type == 3) {
    my $scalar = popfixed(8);
    return { type => 'int', value => unpack('q', $scalar) };
  } elsif ($type == 4) {
    my $scalar = popfixed($doublesize);
    return { type => 'float', value => unpack('d', $scalar) };
  } elsif ($type == 5) {
    my $scalar = popscalar();
    return { type => 'string', value => $scalar };
  } elsif ($type == 6) {
    my $array = poparray();
    return { type => 'array', value => $array };
  } elsif ($type == 7) {
    my $hash = pophash();
    return { type => 'hash', value => $hash };
  }
}

sub poptype
{
  return getbits(3);
}

sub popscalar
{
  my $result = '';
  while (1) {
    my $cont = getbits(1);
    last if (!$cont);
    my $byte = getbits(8);
    $result .= chr($byte);
  }
  return $result;
}

sub popfixed
{
  my $nbytes = shift;
  my $result = '';
  for (my $i=0; $i < $nbytes; $i++) {
    my $byte = getbits(8);
    $result .= chr($byte);
  }
  return $result;
}

sub poparray
{
  my @result;
  while (1) {
    my $cont = getbits(1);
    last if (!$cont);
    my $tuple = popany();
    push @result, $tuple;
  }
  return \@result;
}

sub pophash
{
  my %result;
  while (1) {
    my $cont = getbits(1);
    last if (!$cont);
    my $key = popscalar();
    my $val = popany();
    $result{$key} = $val;
  }
  return \%result;
}

sub getbits
{
  my $nbits = shift;
  my $result = 0;
  while ($nbits--) {
    my $bit = getbit();
    $result <<= 1;
    $result |= $bit;
  }
  return $result;
}

sub getbit
{
  if ($bytepos >= length($input)) {
    $finished = 1;
    return 0;
  }
  my $inputbyte = ord(substr($input, $bytepos, 1));
  my $result = (($inputbyte >> (7-$bitpos)) & 0x01);
  if (++$bitpos > 7) {
    $bitpos = 0;
    ++$bytepos;
  }
  return $result;
}

sub absorb_binary
{
  my $path = shift; die "$path not found" if (! -f $path);
  my $result = '';
  die "Error $@ opening $path" if (!open(FILE, '<', $path));
  binmode FILE;
  my $buf;
  while (1) {
    my $n = sysread(FILE, $buf, 1024);
    if (!$n) {
      close FILE;
      return $result;
    }
    $result .= $buf;
  }
}

sub rework_obj
{
  my $obj = shift;
  if ($obj->{type} eq 'hash') {
    return rework_hash($obj->{value});
  } elsif ($obj->{type} eq 'array') {
    return rework_array($obj->{value});
  } elsif ($obj->{type} eq 'string') {
    return $obj->{value};
  } elsif ($obj->{type} eq 'boolean') {
    return $obj->{value};
  } elsif ($obj->{type} =~ /^(ex|im)plicitnull$/) {
    return undef;
  } elsif ($obj->{type} eq 'int') {
    return $obj->{value};
  } elsif ($obj->{type} eq 'float') {
    return $obj->{value};
  } 
}

sub rework_hash
{
  my $hash = shift;
  my $result = {};
  foreach my $key (keys(%{$hash})) {
    $result->{$key} = rework_obj($hash->{$key});
  }
  return $result;
}

sub rework_array
{
  my $array = shift;
  my $result = [];
  foreach my $elt (@{$array}) {
    push @{$result}, rework_obj($elt);
  }
  return $result;
}

1;
