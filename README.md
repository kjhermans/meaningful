# meaningful

The premise of this project is that it must be possible to create
a binary encoding format specification such that any random input
deterministically produces a valid output.
The use of this specification must allow for complex data structures
in the output (for example, as offered by ASN.1, XML or JSON),
and even useful ones (in other words, this is not just a joke).
An encoder is also provided, as are some tests.

But all in all, it should be enough to issue:

    cat /dev/random | head -c 32 > /tmp/foo
    perl ./decoder.pl /tmp/foo

And watch the output.
