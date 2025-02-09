#!/usr/bin/perl

use Sarthaka;

my $text = '
CHAPTER I

In Which Two Men Go Forth and One Arrives

About half-past eight on a fine, sunny June morning, a small yacht
crept out of Sennen Cove near the Land\'s End and headed for the open
sea. On the shelving beach of the Cove two women and a man, evidently
visitors (or "foreigners" to use the local term), stood watching her
departure with valedictory waving of cap or handkerchief, and the
boatman who had put the crew on board, aided by two of his comrades,
was hauling his boat up above the tide-mark.

A light northerly breeze filled the yacht\'s sails and drew her
gradually seaward. The figures of her crew dwindled to the size of
dolls; shrank with the increasing distance to the magnitude of
insects; and at last, losing all individuality, became mere specks
merged in the form of the fabric that bore them. At this point the
visitors turned their faces inland and walked away up the beach, and
the boatmen, having opined that "she be fetchin\' a tidy offing"
dismissed the yacht from their minds and reverted to the consideration
of a heap of netting and some invalid lobster pots.

On board the receding craft two men sat in the little cockpit. They
formed the entire crew, for the _Sandhopper_ was only a ship\'s
lifeboat, timbered and decked, of light draught and, in the matter of
spars and canvas, what the art critics would call "reticent."

Both men, despite the fineness of the weather, wore yellow oilskins
and sou\'westers, and that was about all they had in common. In other
respects they made a curious contrast, the one small, slender,
sharp-featured, dark almost to swarthiness, and restless and quick in
his movements: the other large, massive, red-faced, blue-eyed, with
the rounded outlines suggestive of ponderous strength: a great ox of a
man, heavy, stolid, but much less unwieldy than he looked.

The conversation incidental to getting the yacht under way had ceased
and silence had fallen on the occupants of the cockpit. The big man
grasped the tiller and looked sulky, which was probably his usual
aspect; and the small man watched him furtively. The land was nearly
two miles distant when the latter broke the silence with a remark very
similar to that of the boatman on the beach.

"You\'re not going to take the shore on board, Purcell. Where are we
supposed to be going to?"

"I am going outside the Longships," was the stolid answer.
';

#my $text = 'helloworld';
#my $text = 'Johnson-Smith';
#my $text = 'kees.jan.hermans@gmail.com';

my $obj = { bytes => [], bitoffset => 7 };

Sarthaka::__encode_string_compressed($text, $obj);

#use Data::Dumper; print STDERR Dumper $obj;
my $orig = length($text);
my $comp = scalar(@{$obj->{bytes}});
my $perc = int((($orig - $comp) / $orig) * 100);
print STDERR "ORIG: $orig, COMPR: $comp, PERC: $perc %\n";

$obj->{byteoffset} = 0;
$obj->{bitoffset} = 0;
my $str = Sarthaka::__decode_string_compressed($obj);

print $str;

1;
