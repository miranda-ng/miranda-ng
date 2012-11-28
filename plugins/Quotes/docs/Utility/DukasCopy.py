from html.parser import HTMLParser
import sys

class MyHTMLParser(HTMLParser):
	def __init__(self, in_fn,out_fn):
		HTMLParser.__init__(self)
		f_in = open(in_fn,'r')
		self.quote = 0
		self.descr = ''
		self.f_out = open(out_fn,'w')
		self.feed(f_in.read())
		f_in.close()
		self.f_out.close()
		
	def handle_starttag(self, tag, attrs):
		#print ("start of a %s" % tag)
		#print (attrs)
		self.start = 1
		if tag == 'input':
			self.f_out.write('\n<quote>')
			for  k in attrs:
				if k[0] == 'stname':
					self.f_out.write('\n\t<symbol>%s</symbol>'%k[1])
				if k[0] == 'stid':
					self.f_out.write('\n\t<id>%s</id>'%k[1])
			self.quote = 1
			

	def handle_endtag(self, tag):
		self.start = 0
		if tag == 'tr' and self.quote == 1:
			if self.descr:
				self.f_out.write('\n\t<description>%s</description>'%self.descr)	
			self.f_out.write('\n</quote>')
			self.quote = 0
			self.descr = 1
		#print ("end of a %s" % tag)
	
	def handle_data(self, data):
		if self.start == 1:
			self.descr = data
		#print ("Data %s" % self.descr)



parser = MyHTMLParser(sys.argv[1],sys.argv[2])
parser.close()
	