#!/usr/bin/perl

use Sarthaka;
use JSON::PP;

my $file = shift @ARGV;
my $bin = absorb_binary($file);
my $obj = Sarthaka::decode($bin);
my $json = encode_json($obj);
print $json . "\n";

exit 0;

sub absorb_binary
{
  my $path = shift; die "$path not found" if (! -f $path);
  my $result = '';
  die "Error $@ opening $bytecodefile" if (!open(FILE, '<', $path));
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

1;
