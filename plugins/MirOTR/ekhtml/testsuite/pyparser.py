#! /usr/bin/env python

import ek_sgmllib, sys, string

class myparser(ek_sgmllib.SGMLParser):
    def __init__(self):
        ek_sgmllib.SGMLParser.__init__(self)

    def handle_data(self, data):
        sys.stdout.write(data)

    def start_way(self, attrs):
        print 'GOT WAY START!'
        
    def unknown_starttag(self, tag, attrs):
        print 'START: "%s"' % tag
        for i in range(len(attrs) - 1, -1, -1):
            key, val = attrs[i]
            print 'ATTRIBUTE: "%s" = "%s"' % (key, val)
        
    def unknown_endtag(self, tag):
        print 'END: "%s"' % tag
    
    def handle_comment(self, comment):
 	print 'COMMENT: "%s"' % comment
    
    def handle_entityref(self, name):
        self.handle_data('&' + name + ';')

    def handle_charref(self, name):
        self.handle_data('&#' + name + ';')
        
def testit(data):
    x = myparser()
    x.feed(data)
    x.close()
    
if __name__ == '__main__':
    if len(sys.argv) == 1:
        f = sys.stdin
    else:
        f = open(sys.argv[1])
        
    testit(f.read())
