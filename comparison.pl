#!/usr/bin/perl

for (my $i=0; $i < 512; $i++) {
  my @f1 = ();
  my @f2 = ();
  print STDERR "$i .. ";
  for (my $j=0; $j < 16; $j++) {
    system("perl ./randomizer.pl $i > /tmp/rnd_$i.json");
#    system("perl ./encoder.pl /tmp/rnd_$i.json > /tmp/rnd_$i.bin");
    system("perl -I lib/perl ./encode2.pl /tmp/rnd_$i.json > /tmp/rnd_$i.bin");
    system("perl ./json2der.pl /tmp/rnd_$i.json > /tmp/rnd_$i.der");
    my @s1 = stat("/tmp/rnd_$i.bin");
    my @s2 = stat("/tmp/rnd_$i.der");
    push @f1, $s1[7];
    push @f2, $s2[7];
  }
  my $avg1 = 0; foreach my $f (@f1) { $avg1 += $f; } $avg1 /= scalar(@f1);
  my $avg2 = 0; foreach my $f (@f2) { $avg2 += $f; } $avg2 /= scalar(@f2);
  print "$i, $avg1, $avg2\n";
}
print STDERR "\n";

1;
