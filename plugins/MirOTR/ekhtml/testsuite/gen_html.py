#!/usr/bin/env python

#
# gen_html: An HTML generation utility
#
# This utility is used to generate random HTML for the parser to
# deal with.  Flags for each of the components allow variations
# on HTML, so as to spew either valid or invalid HTML.
#

from string import *
from ek_sgmllib import tagfind
from random import randint
import re, sys

attrname   = re.compile('[a-zA-Z_][-.a-zA-Z_0-9]*')
attrvalue  = re.compile('[-a-zA-Z0-9@./:+*%?!&$\(\)_#=~]')    
quotedattrvalueset = digits + '#$%&()*+,-./:;=?@[\]^_`{|}~' 
attrvalueset = '-@./:+*%?!&$()_#=~*' + letters + digits

DataString = ['In A.D. 2101',
	      'War was beginning.',
	      'What happen?',
              'Somebody set up us the bomb',
              'We get signal',
              'What!',
              'Main screen turn on',
              "It's You!!",
              'How are you gentlemen!!',
              'All your base are belong to us',
              'You are on the way to destruction',
              'What you say!!',
              'You have no chance to survive make your time',
              'HA HA HA HA ....',
              "Take off every 'zig'",
              'You know what you doing',
              "Move 'zig'",
              'For great justice']

DataWords = split(join(DataString, ' '))

def get_word(matcher, bogus=0):
    while 1:
        res = DataWords[randint(0, len(DataWords)-1)]
        if not bogus:
            z = matcher.match(res)
            if not z or z.end(0) != len(res):
                continue
        return res
    
def get_tagname(bogus=0):
    return upper(get_word(tagfind, bogus))

def get_whitespace():
    return join(map(lambda x: whitespace[randint(0,len(whitespace) - 1)],
                                         range(randint(0, 3))), '')

def get_attrname(bogus=0):
    return get_word(attrname, bogus)


def get_quoted_attrvalue(bogus=0):
    quoteidx   = randint(0, 1)
    quote      = ["'", '"'][quoteidx]
    otherquote = ["'", '"'][not quoteidx]

    res = quote
    for i in range(randint(0, 20)):
        if not randint(0, 5):
            res = res +quotedattrvalueset[randint(0,len(quotedattrvalueset)-1)]
        elif not randint(0, 5):
            res = res + otherquote
        elif not randint(0, 5):
            res = res + get_whitespace()
        else:
            res = res + get_attrname(0)

    if not bogus:
        res = res + quote
    else:
        res = res + [otherquote, '<', '>'][randint(0, 2)]
    return res

def get_unquoted_attrvalue(bogus=0):
    res = ''
    for i in range(randint(not bogus, 20)):
        if not randint(0, 5):
            res = res + get_tagname(bogus)
        elif bogus and not randint(0, 5):
            res = res + get_whitespace()
        else:
            res = res + attrvalueset[randint(0, len(attrvalueset)-1)]

    return res

def get_starttag(bogus=0):
    res = '<'
    if bogus and not randint(0, 10):
        res = res + whitespace[randint(0, len(whitespace)-1)]

    res = res + get_tagname(bogus)
    if not randint(0, 5):
        res = res + get_whitespace()

    for nattrs in range(randint(0, 4)):
        res = res + whitespace[randint(0, len(whitespace)-1)]
        res = res + get_whitespace()
        res = res + get_attrname(bogus)
        t = randint(0, 1)
        if t:
            res = res + get_whitespace() + '=' + get_whitespace() + \
                  get_quoted_attrvalue(randint(0, bogus))
        else:
            res = res + get_whitespace() + '=' + get_whitespace() + \
                  get_unquoted_attrvalue(randint(0, bogus))

    if bogus and not randint(0, 5):
        res = res + get_whitespace() + '>'
    else:
        res = res + '>'

    return res

def get_comment():
    res = '<!--'

    for i in range(randint(0, 100)):
        res = res + get_whitespace()
        res = res + get_attrname(0)
        if not randint(0, 5):
            res = res + '--'
        elif not randint(0, 10):
            res = res + '->'
    res = res + get_whitespace() + '--' + get_whitespace() + '>'
    return res

def get_endtag():
    return '</' + get_attrname() + get_whitespace() + '>'
    

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print 'Syntax: %s <# items> <bogus>' % sys.argv[0]
        sys.exit(1)
        
    isbogus = int(sys.argv[2])
    for i in xrange(int(sys.argv[1])):
        x = randint(0, 2)
        if x == 0:
            print get_starttag(isbogus)
        elif x == 1:
            print get_comment()
        elif x == 2:
            print get_endtag()
