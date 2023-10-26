#!/bin/sh

TESTS=`ls test_*.json | sort`
for TEST in $TESTS; do 
  echo $TEST
  cat $TEST
  perl ./encoder.pl $TEST > $TEST.enc
  hexdump -C $TEST.enc
  perl ./decoder.pl $TEST.enc > $TEST.out
  cat $TEST.out
done
