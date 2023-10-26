# Completely meaningful complex data structure binary encoding

The premise of this project is that it must be possible to create
a binary encoding format specification such that any random input
deterministically produces a valid output.
The overarching idea is that the parser cannot fail for any input.

The use of this specification must allow for complex data structures
in the output (for example, as offered by ASN.1, XML or JSON),
and even useful ones (in other words, this is not just a joke).
An encoder is also provided, as are some tests.

But all in all, it should be enough to issue:

    $ cat /dev/random | head -c 32 > /tmp/foo
    $ perl ./decoder.pl /tmp/foo

And watch the output.
An example of input, encoded binary in hexdump, and output of the
decoder:

    $ cat test.json
    {
      "foo" : "bar",
      "more" : "test",
      "sub" : [
        "a", "b", "c"
      ]
    }
    $ perl ./encoder.pl test.json > test.bin
    $ hexdump -C test.bin
    00000000  fb 35 be de b6 2b 0d c9  b6 db ee 56 55 ba 59 6e  |.5...+.....VU.Yn|
    00000010  77 46 e7 75 b1 36 d8 5b  62 6d 8c 00              |wF.u.6.[bm..|
    0000001c
    $ perl ./decoder.pl test.bin
    {"more":"test","sub":["a","b","c"],"foo":"bar"}

