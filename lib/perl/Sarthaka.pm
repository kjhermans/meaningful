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
  my $float = 0.0;
  my $enc = pack('d', $float);
  my $bytes = __decode_bytes(length($enc), $obj);
  my $result = unpack('d', $bytes);
  return $result;
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
    if (length($ref) > 1023) {
      $head = substr($ref, 0, 1023);
      $ref = substr($ref, 1023);
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
    $obj->{bytes}[-1] |= (1 << $obj->{bitoffset});
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
  my $result = (($inputbyte >> $obj->{bitoffset}) & 0x01);
  if (++($obj->{bitoffset}) > 7) {
    $obj->{bitoffset} = 0;
    ++($obj->{byteoffset});
  }
  return $result;
}

## Low endian

#sub __encode_bits
#{
#  my ($value, $nbits, $obj) = @_;
#  for (my $i=0; $i < $nbits; $i++) {
#    my $bit = $value & 0x01;
#    $value >>= 1;
#    __encode_bit($bit, $obj);
#  }
#}
#
#sub __decode_bits
#{
#  my ($nbits, $obj) = @_;
#  my $result = 0;
#  for (my $i=0; $i < $nbits; $i++) {
#    my $bit = __decode_bit($obj);
#    if ($bit) {
#      $result |= (1 << $i);
#    }
#  }
#  return $result;
#}

## High endian

sub __encode_bits
{
  my ($value, $nbits, $obj) = @_;
  for (my $i=0; $i < $nbits; $i++) {
    if ($value & (1<<($nbits-(1+$i)))) {
      __encode_bit(1, $obj);
    } else {
      __encode_bit(0, $obj);
    }
  }
}

sub __decode_bits
{
  my ($nbits, $obj) = @_;
  my $result = 0;
  for (my $i=0; $i < $nbits; $i++) {
    my $bit = __decode_bit($obj);
    if ($bit) {
      $result |= (1 << ($nbits-(1+$i)));
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





##---- String compression, experimental

my $chunkbits = 7;
my $chunksize = 2 ** $chunkbits;
my $chunkbits_u = 3;
my $chunksize_u = 2 ** $chunkbits_u;

#my $dict = [
#  ' ', ',', '.', '"', "\n", "'", 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
#  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
#  'w', 'x', 'y', 'z',
#];
#my $dictbits = 5;
#my $reversedict;
#for (my $i=0; $i < scalar(@{$dict}); $i++) {
#  $reversedict->{$dict->[$i]} = $i;
#}
#
#sub __encode_string_decode_char
#{
#  my $obj = shift;
#  return $dict->[ __decode_bits($dictbits, $obj) ];
#}
#
#sub __encode_string_dict_reverse
#{
#  my $chr = shift;
#  return ( $reversedict->{$chr}, $dictbits );
#}

## Huffman encoding based on an arbitrary English text

my $dict = [
  [ 0x20, 3, 0x00 ], #   -> 00 0 (freq 71374) 
  [ 0x65, 3, 0x01 ], # e -> 00 1 (freq 44244) 
  [ 0x74, 4, 0x04 ], # t -> 01 00 (freq 32406) 
  [ 0x61, 4, 0x05 ], # a -> 01 01 (freq 27789) 
  [ 0x6f, 4, 0x06 ], # o -> 01 10 (freq 25357) 
  [ 0x6e, 4, 0x07 ], # n -> 01 11 (freq 23670) 
  [ 0x69, 5, 0x10 ], # i -> 10 000 (freq 21560) 
  [ 0x68, 5, 0x11 ], # h -> 10 001 (freq 21236) 
  [ 0x73, 5, 0x12 ], # s -> 10 010 (freq 20055) 
  [ 0x72, 5, 0x13 ], # r -> 10 011 (freq 19164) 
  [ 0x64, 5, 0x14 ], # d -> 10 100 (freq 15163) 
  [ 0x6c, 5, 0x15 ], # l -> 10 101 (freq 13902) 
  [ 0x75, 5, 0x16 ], # u -> 10 110 (freq 9970) 
  [ 0x63, 5, 0x17 ], # c -> 10 111 (freq 9332) 
  [ 0x0a, 6, 0x30 ], #   -> 11 0000 (freq 8458) 
  [ 0x6d, 6, 0x31 ], # m -> 11 0001 (freq 7372) 
  [ 0x77, 6, 0x32 ], # w -> 11 0010 (freq 7368) 
  [ 0x66, 6, 0x33 ], # f -> 11 0011 (freq 7205) 
  [ 0x79, 6, 0x34 ], # y -> 11 0100 (freq 6824) 
  [ 0x70, 6, 0x35 ], # p -> 11 0101 (freq 6396) 
  [ 0x67, 6, 0x36 ], # g -> 11 0110 (freq 6062) 
  [ 0x62, 6, 0x37 ], # b -> 11 0111 (freq 4744) 
  [ 0x2c, 6, 0x38 ], # , -> 11 1000 (freq 4626) 
  [ 0x2e, 6, 0x39 ], # . -> 11 1001 (freq 4455) 
  [ 0x76, 6, 0x3a ], # v -> 11 1010 (freq 3259) 
  [ 0x6b, 6, 0x3b ], # k -> 11 1011 (freq 3059) 
  [ 0x22, 6, 0x3c ], # " -> 11 1100 (freq 3049) 
  [ 0x27, 6, 0x3d ], # ' -> 11 1101 (freq 870) 
  [ 0x2d, 6, 0x3e ], # - -> 11 1110 (freq 816) 
  [ 0x3b, 6, 0x3f ], # ; -> 11 1111 (freq 621) 
];

sub __encode_string_decode_char
{
  my $obj = shift;
  my $enclen = __decode_bits(2, $obj);
  my $remainder = __decode_bits($enclen + 1, $obj);
  my $codon = ($enclen << ($enclen + 1)) | $remainder;
  foreach my $elt (@{$dict}) {
    if ($codon == $elt->[ 2 ]) {
      return chr($elt->[ 0 ]);
    }
  }
}

sub __encode_string_dict_reverse
{
  my $chr = shift;
  foreach my $elt (@{$dict}) {
    if (ord($chr) == $elt->[ 0 ]) {
      return ( $elt->[ 2 ], $elt->[ 1 ] );
    }
  }
  return ( undef, undef );
}




sub __encode_string_compressed
{
  my ($string, $obj) = @_;

  my $state = {
    string => $string,
    chunks => [],
    obj    => $obj,
  };

  while (length($state->{string})) {
    my $chr = substr($state->{string}, 0, 1);
    $state->{string} = substr($state->{string}, 1);
    my ($i, $b) = __encode_string_dict_reverse($chr);
    if (defined($i)) {
      __encode_string_compressed_consume_compressible($state, $chr);
    } else {
      __encode_string_compressed_consume_incompressible($state, $chr);
    }
    __encode_string_compressed_examine_chunks($state);
  }
  __encode_string_compressed_encode_chunks($state);
  __encode_bit(0, $obj);
}

sub __encode_string_compressed_consume_compressible
{
  my ($state, $chr) = @_;

  my @chunk = ( $chr );

  while (length($state->{string})) {
    $chr = substr($state->{string}, 0, 1);
    my ($i, $b) = __encode_string_dict_reverse($chr);
    if (defined($i)) {
      push @chunk, $chr;
      $state->{string} = substr($state->{string}, 1);
    } else {
      last;
    }
  }
  push @{$state->{chunks}}, { type => 'cmpr', buf => [ @chunk ] };
}

sub __encode_string_compressed_consume_incompressible
{
  my ($state, $chr) = @_;

  my @chunk = ( $chr );

  while (length($state->{string})) {
    $chr = substr($state->{string}, 0, 1);
    my ($i, $b) = __encode_string_dict_reverse($chr);
    if (!defined($i)) {
      push @chunk, $chr;
      $state->{string} = substr($state->{string}, 1);
    } else {
      last;
    }
  }
  push @{$state->{chunks}}, { type => 'asis', buf => [ @chunk ] };
}

sub __encode_string_compressed_examine_chunks
{
  my $state = shift;

  if (scalar(@{$state->{chunks}}) < 16) {
    return;
  }
  for (my $i=1; $i < scalar(@{$state->{chunks}}); $i++) {
    my $chunk = $state->{chunks}[ $i ];
    if ($chunk->{type} eq 'cmpr' && scalar(@{$chunk->{buf}}) < 8) {
      my $prevchunk = $state->{chunks}[ $i-1 ];
      push @{$prevchunk->{buf}}, @{$chunk->{buf}};
      splice(@{$state->{chunks}}, $i, 1);
      --$i;
    }
  }
  for (my $i=1; $i < scalar(@{$state->{chunks}}); $i++) {
    my $chunk = $state->{chunks}[ $i ];
    if ($chunk->{type} eq 'asis') {
      my $prevchunk = $state->{chunks}[ $i-1 ];
      if ($prevchunk->{type} eq 'asis') {
        push @{$prevchunk->{buf}}, @{$chunk->{buf}};
        splice(@{$state->{chunks}}, $i, 1);
        --$i;
      }
    }
  }
  __encode_string_compressed_encode_chunks($state, 8);
}

sub __encode_string_compressed_encode_chunks
{
  my $state = shift;
  my $range = shift;
  my $i;

  if (!defined($range)) {
    $range = scalar(@{$state->{chunks}});
  }
  for ($i=0; $i < $range && $i < scalar(@{$state->{chunks}}); $i++) {
    my $chunk = $state->{chunks}[ $i ];
    if ($chunk->{type} eq 'cmpr') {
      __encode_string_compressed_chunk_compressed($chunk, $state->{obj});
    } else {
      __encode_string_compressed_chunk_as_is($chunk, $state->{obj});
    }
  }
  while ($i) {
    shift @{$state->{chunks}};
    --$i;
  }
}

use List::Util qw(min);

sub __encode_string_compressed_chunk_compressed
{
  my ($chunk, $obj) = @_;

  for (my $n=0; $n < scalar(@{$chunk->{buf}}); $n += $chunksize) {
    my $s = min($chunksize, scalar(@{$chunk->{buf}}) - $n);
    __encode_bit(1, $obj);
    __encode_bit(1, $obj);
    __encode_bits($s-1, $chunkbits, $obj);
    for (my $i=$n; $i < $n + $s; $i++) {
      my $chr = $chunk->{buf}[ $i ];
      my ($i, $b) = __encode_string_dict_reverse($chr);
      __encode_bits($i, $b, $obj);
    }
  }
}

sub __encode_string_compressed_chunk_as_is
{
  my ($chunk, $obj) = @_;

  for (my $n=0; $n < scalar(@{$chunk->{buf}}); $n += $chunksize_u) {
    my $s = min($chunksize_u, scalar(@{$chunk->{buf}}) - $n);
    __encode_bit(1, $obj);
    __encode_bit(0, $obj);
    __encode_bits($s-1, $chunkbits_u, $obj);
    for (my $i=$n; $i < $n + $s; $i++) {
      __encode_byte(ord($chunk->{buf}[ $i ]), $obj);
    }
  }
}

sub __decode_string_compressed
{
  my ($obj) = @_;

  my $res = '';

  while (1) {
    my $cont = __decode_bit($obj);
    last if (!$cont);
    my $compressed = __decode_bit($obj);
    if ($compressed) {
      my $chunklen = __decode_bits($chunkbits, $obj) + 1;
      for (my $i=0; $i < $chunklen; $i++) {
        $res .= __encode_string_decode_char($obj);
      }
    } else {
      my $chunklen = __decode_bits($chunkbits_u, $obj) + 1;
      for (my $i=0; $i < $chunklen; $i++) {
        $res .= chr(__decode_byte($obj));
      }
    }
  }
  return $res;
}

##---- /String compression





package Sarthaka::ImplicitNull;

sub new
{
  my $result = {};
  bless $result, 'Sarthaka::ImplicitNull';
  return $result;
}

1;
