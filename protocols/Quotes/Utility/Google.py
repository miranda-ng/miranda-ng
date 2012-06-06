from html.parser import HTMLParser
import sys
from xml.etree.ElementTree import Element, ElementTree, SubElement

class MyHTMLParser(HTMLParser):
	def __init__(self,in_fn,out_fn):
		HTMLParser.__init__(self)
		f_in = open(in_fn,'r')
		self.quote = 0
		self.start = 0
		self.parse_option = 0
		self.elQuote = Element("fake")
		elProvider = Element("Provider")
		SubElement(elProvider,'name').text = 'Google'
		SubElement(elProvider,'ref').text = 'http://www.google.com'
		SubElement(elProvider,'url').text = 'http://www.google.com/finance/converter?a=1&'
		self.root = SubElement(elProvider,'section')
		SubElement(self.root,'name').text = 'Currencies'
		self.feed(f_in.read())
		f_in.close()
		ElementTree(elProvider).write(out_fn)
		
	def handle_starttag(self, tag, attrs):
		self.start = 1
		if tag == 'select':
			if self.parse_option == 0:
				for k in attrs:
					if k[0] == 'name' and k[1] == 'from':
						self.parse_option = 1
					break
			else:
				self.parse_option == 0
		elif self.parse_option == 1 and tag == 'option':
			for k in attrs:
				if k[0] == 'value':
					self.elQuote = SubElement(self.root,'quote')
					SubElement(self.elQuote,'id').text = k[1]
					SubElement(self.elQuote,'symbol').text = k[1]
				break

	def handle_endtag(self, tag):
		self.start = 0
		if tag == 'select':
			self.parse_option == 0
	
	def handle_data(self, data):
		if self.start == 1 and self.parse_option == 1:
			SubElement(self.elQuote,'description').text = data

parser = MyHTMLParser(sys.argv[1],sys.argv[2])
parser.close()
	
