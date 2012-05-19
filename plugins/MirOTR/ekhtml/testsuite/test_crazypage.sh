#! /bin/bash

for ntags in 1 10 100 1000
do
    for bogus in 0 1
    do
	python gen_html.py $ntags $bogus > crazy.page
	for bytesize in 1 13 162 983 8192
	do
	    echo numElements=$ntags allowBogusTags=$bogus feedSize=$bytesize
	    ./tester $bytesize < crazy.page > ek.test
	    ./pyparser.py < crazy.page > py.test
	    diff -i -u ek.test py.test || exit 1
	done
    done
done
