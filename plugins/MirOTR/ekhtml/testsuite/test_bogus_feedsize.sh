#! /bin/bash
#
# Test the e-k parser to make sure that altering the feedsize doesn't change
# which tags get recognized.  This is really a boundary condition.
#

NUMTAGS=1000
NUMFEED=1024

echo Generating crazy.page with $NUMTAGS tags
python gen_html.py $NUMTAGS 1 > crazy.page

feedsize=1

echo "Parsing with $NUMFEED different feed sizes"
while [ $(($feedsize < $NUMFEED)) -eq 1 ]; do
    ./tester $feedsize < crazy.page > crazy.page.out.$feedsize
    feedsize=$(($feedsize + 1))
done

NUMSIZES=`ls -la crazy.page.out.* | awk '{print $5}' | sort | uniq | wc -l`
if [ $NUMSIZES -ne 1 ] ; then
    echo 'FAILURE -- Leaving crazy.page.out.* for examination'
    exit 1
fi

rm -f crazy.page.out.*
