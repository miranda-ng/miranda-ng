/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "xmlParser.h"

MIR_APP_DLL(HXML) xmlCreateNode(LPCTSTR name, LPCTSTR text, char isDeclaration)
{
	XMLNode result = XMLNode::createXMLTopNode(name, isDeclaration);
	if (text)
		result.updateText(text);
	return result.detach();
}

MIR_APP_DLL(void) xmlDestroyNode(HXML n)
{
	XMLNode tmp; tmp.attach(n);
}

MIR_APP_DLL(HXML) xmlParseFile(LPCTSTR str, int* datalen, LPCTSTR tag)
{
	if (str == NULL) return NULL;

	XMLResults res;
	XMLNode result = XMLNode::parseFile(str, tag, &res);

	if (datalen != NULL)
		datalen[0] += res.nChars;

	return (res.error == eXMLErrorNone || (tag != NULL && res.error == eXMLErrorMissingEndTag)) ? result.detach() : NULL;
}

MIR_APP_DLL(HXML) xmlParseString(LPCTSTR str, int* datalen, LPCTSTR tag)
{
	if (str == NULL) return NULL;

	XMLResults res;
	XMLNode result = XMLNode::parseString(str, tag, &res);

	if (datalen != NULL)
		datalen[0] += res.nChars;

	return (res.error == eXMLErrorNone || (tag != NULL && res.error == eXMLErrorMissingEndTag)) ? result.detach() : NULL;
}

MIR_APP_DLL(HXML) xmlAddChild(HXML _n, LPCTSTR name, LPCTSTR text)
{
	XMLNode result = XMLNode(_n).addChild(name);
	if (text != NULL)
		result.updateText(text);
	return result;
}

MIR_APP_DLL(void) xmlAddChild2(HXML _child, HXML _parent)
{
	XMLNode child(_child), parent(_parent);
	parent.addChild(child);
}

MIR_APP_DLL(HXML) xmlCopyNode(HXML _n)
{
	XMLNode result = XMLNode(_n);
	return result.detach();
}

MIR_APP_DLL(LPCTSTR) xmlGetAttr(HXML _n, int i)
{
	return XMLNode(_n).getAttributeValue(i);
}

MIR_APP_DLL(int) xmlGetAttrCount(HXML _n)
{
	return XMLNode(_n).nAttribute();
}

MIR_APP_DLL(LPCTSTR) xmlGetAttrName(HXML _n, int i)
{
	return XMLNode(_n).getAttributeName(i);
}

MIR_APP_DLL(HXML) xmlGetChild(HXML _n, int i)
{
	return XMLNode(_n).getChildNode(i);
}

MIR_APP_DLL(HXML) xmlGetChildByAttrValue(HXML _n, LPCTSTR name, LPCTSTR attrName, LPCTSTR attrValue)
{
	return XMLNode(_n).getChildNodeWithAttribute(name, attrName, attrValue);
}

MIR_APP_DLL(int) xmlGetChildCount(HXML _n)
{
	return XMLNode(_n).nChildNode();
}

MIR_APP_DLL(HXML) xmlGetFirstChild(HXML _n)
{
	return XMLNode(_n).getChildNode(0);
}

MIR_APP_DLL(HXML) xmlGetNthChild(HXML _n, LPCTSTR name, int i)
{
	return XMLNode(_n).getChildNode(name, i);
}

MIR_APP_DLL(HXML) xmlGetNextChild(HXML _n, LPCTSTR name, int* i)
{
	return XMLNode(_n).getChildNode(name, i);
}

MIR_APP_DLL(HXML) xmlGetNextNode(HXML _n)
{
	return XMLNode(_n).getNextNode();
}

MIR_APP_DLL(HXML) xmlGetChildByPath(HXML _n, LPCTSTR path, char createNodeIfMissing)
{
	return XMLNode(_n).getChildNodeByPath(path, createNodeIfMissing);
}

MIR_APP_DLL(LPCTSTR) xmlGetName(HXML _n)
{
	return XMLNode(_n).getName();
}

MIR_APP_DLL(HXML) xmlGetParent(HXML _n)
{
	return XMLNode(_n).getParentNode();
}

MIR_APP_DLL(LPCTSTR) xmlGetText(HXML _n)
{
	return XMLNode(_n).getInnerText();
}

MIR_APP_DLL(LPCTSTR) xmlGetAttrValue(HXML _n, LPCTSTR attrName)
{
	return XMLNode(_n).getAttribute(attrName);
}

MIR_APP_DLL(void) xmlSetText(HXML _n, LPCTSTR _text)
{
	XMLNode(_n).updateText(_text);
}

MIR_APP_DLL(LPTSTR) xmlToString(HXML _n, int* datalen)
{
	return XMLNode(_n).createXMLString(0, datalen);
}

MIR_APP_DLL(XMLError) xmlToFile(HXML _n, LPCTSTR filename, int withformatting)
{
	return XMLNode(_n).writeToFile(filename, NULL, withformatting);
}

MIR_APP_DLL(void) xmlAddAttr(HXML _n, LPCTSTR attrName, LPCTSTR attrValue)
{
	if (attrName != NULL && attrValue != NULL)
		XMLNode(_n).addAttribute(attrName, attrValue);
}

MIR_APP_DLL(void) xmlAddAttrInt(HXML _n, LPCTSTR attrName, int attrValue)
{
	TCHAR buf[40];
	_itot(attrValue, buf, 10);
	XMLNode(_n).addAttribute(attrName, buf);
}

MIR_APP_DLL(void) xmlFree(void* p)
{
	free(p);
}

//) xml API v2 methods
MIR_APP_DLL(int) xmlGetTextCount(HXML _n)
{
	return XMLNode(_n).nText();
}

MIR_APP_DLL(LPCTSTR) xmlGetTextByIndex(HXML _n, int i)
{
	return XMLNode(_n).getText(i);
}

MIR_APP_DLL(void) xmlSetTextByIndex(HXML _n, int i, LPCTSTR value)
{
	XMLNode(_n).updateText(value, i);
}

MIR_APP_DLL(void) xmlAddText(HXML _n, LPCTSTR value, XML_ELEMENT_POS pos)
{
	XMLNode(_n).addText(value, (XMLElementPosition)pos);
}

MIR_APP_DLL(LPTSTR) xmlToStringWithFormatting(HXML _n, int* datalen)
{
	return XMLNode(_n).createXMLString(1, datalen);
}

MIR_APP_DLL(int) xmlGetClearCount(HXML _n)
{
	return XMLNode(_n).nClear();
}

MIR_APP_DLL(LPCTSTR) xmlGetClear(HXML _n, int i, LPCTSTR *openTag, LPCTSTR *closeTag)
{
	XMLClear c = XMLNode(_n).getClear(i);
	if (openTag)
		*openTag = c.lpszOpenTag;
	if (closeTag)
		*closeTag = c.lpszCloseTag;
	return c.lpszValue;
}

MIR_APP_DLL(void) xmlAddClear(HXML _n, LPCTSTR lpszValue, LPCTSTR openTag, LPCTSTR closeTag, XML_ELEMENT_POS pos)
{
	XMLNode(_n).addClear(lpszValue, openTag, closeTag, (XMLElementPosition)pos);
}

MIR_APP_DLL(void) xmlSetClear(HXML _n, int i, LPCTSTR lpszValue)
{
	XMLNode(_n).updateClear(lpszValue, i);
}

MIR_APP_DLL(int) xmlGetElement(HXML _n, XML_ELEMENT_POS pos, XML_ELEMENT_TYPE *type, HXML *child, LPCTSTR *value, LPCTSTR *name, LPCTSTR *openTag, LPCTSTR *closeTag)
{
	// reset all values
	if (child)
		*child = NULL;
	if (value)
		*value = NULL;
	if (name)
		*name = NULL;
	if (openTag)
		*openTag = NULL;
	if (closeTag)
		*closeTag = NULL;

	if (!type || pos >= XMLNode(_n).nElement())
		return false;

	XMLNodeContents c(XMLNode(_n).enumContents((XMLElementPosition)pos));
	switch (c.etype) {
	case eNodeChild:
		*type = XML_ELEM_TYPE_CHILD;
		if (child)
			*child = c.child;
		break;
	
	case eNodeAttribute:
		*type = XML_ELEM_TYPE_ATTRIBUTE;
		if (name)
			*name = c.attrib.lpszName;
		if (value)
			*value = c.attrib.lpszValue;
		break;
	
	case eNodeText:
		*type = XML_ELEM_TYPE_TEXT;
		if (value)
			*value = c.text;
		break;

	case eNodeClear:
		*type = XML_ELEM_TYPE_CLEAR;
		if (value)
			*value = c.clear.lpszValue;
		if (openTag)
			*openTag = c.clear.lpszOpenTag;
		if (closeTag)
			*closeTag = c.clear.lpszCloseTag;
		break;

	case eNodeNULL:
		return false;
	}
	return true;
}

MIR_APP_DLL(int) xmlGetElementCount(HXML _n)
{
	return XMLNode(_n).nElement();
}

MIR_APP_DLL(char) xmlIsDeclaration(HXML _n)
{
	return XMLNode(_n).isDeclaration();
}

MIR_APP_DLL(HXML) xmlDeepCopy(HXML _n)
{
	return XMLNode(_n).deepCopy().detach();
}

MIR_APP_DLL(HXML) xmlAddChildEx(HXML _n, LPCTSTR name, char isDeclaration, XML_ELEMENT_POS pos)
{
	return XMLNode(_n).addChild(name, isDeclaration, (XMLElementPosition)pos);
}

MIR_APP_DLL(void) xmlAddChildEx2(HXML _n, HXML parent, XML_ELEMENT_POS pos)
{
	XMLNode(_n).addChild(parent, (XMLElementPosition)pos);
}

MIR_APP_DLL(void) xmlSetAttrByIndex(HXML _n, int i, LPCTSTR value)
{
	XMLNode(_n).updateAttribute(value, NULL, i);
}

MIR_APP_DLL(void) xmlSetAttrByName(HXML _n, LPCTSTR name, LPCTSTR value)
{
	XMLNode(_n).updateAttribute(value, NULL, name);
}

MIR_APP_DLL(void) xmlDeleteNodeContent(HXML _n)
{
	XMLNode(_n).deleteNodeContent();
}

MIR_APP_DLL(void) xmlDeleteAttrByIndex(HXML _n, int i)
{
	XMLNode(_n).deleteAttribute(i);
}

MIR_APP_DLL(void) xmlDeleteAttrByName(HXML _n, LPCTSTR name)
{
	XMLNode(_n).deleteAttribute(name);
}

MIR_APP_DLL(void) xmlDeleteText(HXML _n, int i)
{
	XMLNode(_n).deleteText(i);
}

MIR_APP_DLL(void) xmlDeleteClear(HXML _n, int i)
{
	XMLNode(_n).deleteClear(i);
}

MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfText(HXML _n, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfText(i);
}

MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfClear(HXML _n, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfClear(i);
}

MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfChildByIndex(HXML _n, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfChildNode(i);
}

MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfChildByNode(HXML _n, HXML child)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfChildNode(child);
}

MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfChildByName(HXML _n, LPCTSTR name, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfChildNode(name, i);
}
