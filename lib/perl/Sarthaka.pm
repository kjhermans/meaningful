package Sarthaka;

use strict;

sub encode
{
  my $ref = shift;
  my @bytes = __encode_top($ref);
  my $str = '';
  foreach my $byte (@bytes) {
    $str .= chr($byte);
  }
  return $str;
}

sub encode_to_file
{
  my $ref = shift;
  my $path = shift;
  open FILE, "> $path" || die "Could not open $path for writing";
  my @bytes = __encode_top($ref);
  foreach my $byte (@bytes) {
    syswrite(FILE, chr($byte));
  }
  close FILE;
}

sub decode
{
  my $bin = shift;
  my @bytes;
  for (my $i=0; $i < length($bin); $i++) {
    push @bytes, ord(substr($bin, $i, 1));
  }
  my $obj = { bytes => \@bytes, byteoffset => 0, bitoffset => 0 };
  my $ref = __decode_top($obj);
  while (scalar(@{$ref}) && ref $ref->[-1] eq 'Sarthaka::ImplicitNull') {
    pop @{$ref};
  }
  if (scalar(@{$ref}) == 1) {
    return $ref->[0];
  } else {
    return $ref;
  }
}

sub decode_from_file
{
  my $path = shift;
  my $fh;
  if ($path eq '-') {
##..
  } else {
##..
  }
}

##---- 'private' functions ----#

sub __encode_top
{
  my $ref = shift;
  my $obj = { bytes => [], bitoffset => 7 };
  __encode_ref($ref, $obj);
  return @{$obj->{bytes}};
}

sub __decode_top
{
  my ($obj) = @_;
  my @list;
  while (!$obj->{eof}) {
    push @list, __decode_ref($obj);
  }
  return \@list;
}

sub __encode_ref
{
  my ($ref, $obj) = @_;
  if (!defined($ref)) {
    __encode_null($obj);
  } elsif (ref $ref eq 'HASH') {
    __encode_hash($ref, $obj);
  } elsif (ref $ref eq 'ARRAY') {
    __encode_array($ref, $obj);
  } elsif ($ref =~ /^(true|false)$/) {
    __encode_boolean($ref, $obj);
  } elsif ($ref =~ /^[0-9]*\.[0-9]+$/) {
    __encode_float($ref, $obj);
  } elsif ($ref =~ /^[0-9]+$/) {
    __encode_integer(int($ref), $obj);
  } else {
    __encode_string($ref, $obj);
  }
}

sub __decode_ref
{
  my ($obj) = @_;
  my $type = __decode_type($obj);
  if ($type == 0) {
    return undef;
#    return Sarthaka::ImplicitNull->new();
  } elsif ($type == 1) {
    return undef;
  } elsif ($type == 2) {
    return __decode_boolean($obj);
  } elsif ($type == 3) {
    return __decode_float($obj);
  } elsif ($type == 4) {
    return __decode_integer($obj);
  } elsif ($type == 5) {
    return __decode_string($obj);
  } elsif ($type == 6) {
    return __decode_array($obj);
  } elsif ($type == 7) {
    return __decode_hash($obj);
  }
}

sub __encode_null
{
  my ($obj) = @_;
  __encode_type(1, $obj);
}

sub __encode_boolean
{
  my ($ref, $obj) = @_;
  __encode_type(2, $obj);
  if ($ref eq 'true') {
    __encode_bit(1, $obj);
  } else {
    __encode_bit(0, $obj);
  }
}

sub __decode_boolean
{
  my ($obj) = @_;
  my $bit = __decode_bit($obj);
  return $bit;
}

sub __encode_float
{
  my ($ref, $obj) = @_;
  __encode_type(3, $obj);
  __encode_bytes(pack('d', $ref), $obj);
}

sub __decode_float
{
  my ($obj) = @_;
  return 0.0;
}

## Assuming max 64 bit signed integer (excess will not be encoded)
## Fairly complicated encoding that holds the middle between
## fixed length (which an int ultimately is) and dynamic length
## encoding using continuity bits. Add to that the one bit is not
## for the number, but for the sign.

sub __encode_integer
{
  my ($ref, $obj) = @_;
  __encode_type(4, $obj);
  my $sign = 0;
  if ($ref < 0) {
    $ref = -$ref;
    $sign = 1;
  }
  __encode_bit($sign, $obj);
  my @bytes;
  while ($ref && scalar(@bytes) < 8) {
    my $byte = $ref & 0xff;
    push @bytes, $byte;
    $ref >>= 8;
  }
  for (my $i=0; $i < scalar(@bytes); $i++) {
    __encode_bit(1, $obj);
    my $byte = $bytes[ $i ];
    if ($i == 7) {
      __encode_bits($byte, 7, $obj);
    } else {
      __encode_byte($byte, $obj);
    }
  }
  if (scalar(@bytes) < 8) {
    __encode_bit(0, $obj);
  }
}

sub __decode_integer
{
  my ($obj) = @_;
  my $result = 0;
  my $sign = __decode_bit($obj);
  for (my $i=0; $i < 8; $i++) {
    my $cont = __decode_bit($obj);
    last if (!$cont);
    my $byte;
    if ($i < 7) {
      $byte = __decode_byte($obj);
    } else {
      $byte = __decode_bits(7, $obj);
    }
    $result |= ($byte << ($i*8));
  }
  if ($sign) {
    $result = -$result;
  }
  return $result;
}

sub __encode_string
{
  my ($ref, $obj) = @_;
  __encode_type(5, $obj);
  __encode_string_body($ref, $obj);
}

sub __encode_string_body
{
  my ($ref, $obj) = @_;
  $ref = "$ref";
  while (length($ref)) {
    my $head = $ref;
    if (length($ref) > 1024) {
      $head = substr($ref, 0, 1024);
      $ref = substr($ref, 1024);
    } else {
      $ref = '';
    }
    __encode_bits(length($head), 10, $obj);
    __encode_bytes($head, $obj);
  }
  __encode_bits(0, 10, $obj);
}

sub __decode_string
{
  my ($obj) = @_;
  my $result = '';
  while (!$obj->{eof}) {
    my $length = __decode_bits(10, $obj);
    last if (!$length);
    for (my $i=0; $i < $length; $i++) {
      my $byte = __decode_byte($obj);
      $result .= chr($byte);
    }
  }
  return $result;
}

sub __encode_array
{
  my ($ref, $obj) = @_;
  __encode_type(6, $obj);
  foreach my $elt (@{$ref}) {
    __encode_bit(1, $obj);
    __encode_ref($elt, $obj);
  }
  __encode_bit(0, $obj);
}

sub __decode_array
{
  my ($obj) = @_;
  my $result = [];
  while (!$obj->{eof} && __decode_bit($obj)) {
    my $elt = __decode_ref($obj);
    push @{$result}, $elt;
  }
  return $result;
}

sub __encode_hash
{
  my ($ref, $obj) = @_;
  __encode_type(7, $obj);
  my @keys = keys(%{$ref});
  foreach my $key (@keys) {
    my $value = $ref->{$key};
    __encode_bit(1, $obj);
    __encode_string_body("$key", $obj);
    __encode_ref($value, $obj);
  }
  __encode_bit(0, $obj);
}

sub __decode_hash
{
  my ($obj) = @_;
  my $result = {};
  while (!$obj->{eof} && __decode_bit($obj)) {
    my $key = __decode_string($obj);
    my $value = __decode_ref($obj);
    $result->{$key} = $value;
  }
  return $result;
}

sub __encode_bit
{
  my ($bit, $obj) = @_;
  ++($obj->{bitoffset});
  if ($obj->{bitoffset} > 7) {
    push @{$obj->{bytes}}, 0;
    $obj->{bitoffset} = 0;
  }
  if ($bit) {
    $obj->{bytes}[-1] |= (1 << (7 - $obj->{bitoffset}));
  }
}

sub __decode_bit
{
  my ($obj) = @_;
  if ($obj->{byteoffset} >= scalar(@{$obj->{bytes}})) {
    $obj->{eof} = 1;
    return 0;
  }
  my $inputbyte = $obj->{bytes}[ $obj->{byteoffset} ];
  my $result = (($inputbyte >> (7 - $obj->{bitoffset})) & 0x01);
  if (++($obj->{bitoffset}) > 7) {
    $obj->{bitoffset} = 0;
    ++($obj->{byteoffset});
  }
  return $result;
}

sub __encode_bits
{
  my ($value, $nbits, $obj) = @_;
  for (my $i=0; $i < $nbits; $i++) {
    my $bit = $value & 0x01;
    $value >>= 1;
    __encode_bit($bit, $obj);
  }
}

sub __decode_bits
{
  my ($nbits, $obj) = @_;
  my $result = 0;
  for (my $i=0; $i < $nbits; $i++) {
    my $bit = __decode_bit($obj);
    if ($bit) {
      $result |= (1 << $i);
    }
  }
  return $result;
}

sub __encode_type
{
  my ($type, $obj) = @_;
  __encode_bits($type, 3, $obj);
}

sub __decode_type
{
  my ($obj) = @_;
  return __decode_bits(3, $obj);
}

sub __encode_byte
{
  my ($byte, $obj) = @_;
  __encode_bits($byte, 8, $obj);
}

sub __decode_byte
{
  my ($obj) = @_;
  return __decode_bits(8, $obj);
}

sub __encode_bytes
{
  my ($string, $obj) = @_;
  while (length($string)) {
    my $chr = substr($string, 0, 1);
    $string = substr($string, 1);
    __encode_byte(ord($chr), $obj);
  }
}

sub __decode_bytes
{
  my ($nbytes, $obj) = @_;
  my $string = '';
  for (my $i=0; $i < $nbytes; $i++) {
    my $byte = __decode_byte($obj);
    $string .= chr($byte);
  }
  return $string;
}

package Sarthaka::ImplicitNull;

sub new
{
  my $result = {};
  bless $result, 'Sarthaka::ImplicitNull';
  return $result;
}

1;
