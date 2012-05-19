#! /bin/bash

for i in `find tstpages -type f | sort`
do
    echo Testing page: $i
    ./tester 1 < $i > ek.test
    ./pyparser.py $i > py.test
    diff -a -i -u ek.test py.test || exit 1
done

