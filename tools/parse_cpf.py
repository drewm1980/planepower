#!/usr/bin/env python
# Parse an orocos .cpf (xml) file

import xml.etree.ElementTree as xml

def parse_cpf(filename):
	import xml.etree.ElementTree as xml
	tree = xml.parse(filename)
	root = tree.getroot()
	props = root.getchildren()
	proplist = []
	for prop in props:
		d =dict(prop.items())
		name = d['name']
		ty = d['type']
		child = prop.getchildren()[0]
		value = child.text
		proplist.append((name,ty,value))
	return proplist

