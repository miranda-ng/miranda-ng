/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org),
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
	if (str == nullptr) return nullptr;

	XMLResults res;
	XMLNode result = XMLNode::parseFile(str, tag, &res);

	if (datalen != nullptr)
		datalen[0] += res.nChars;

	return (res.error == eXMLErrorNone || (tag != nullptr && res.error == eXMLErrorMissingEndTag)) ? result.detach() : nullptr;
}

MIR_APP_DLL(HXML) xmlParseString(LPCTSTR str, int* datalen, LPCTSTR tag)
{
	if (str == nullptr) return nullptr;

	XMLResults res;
	XMLNode result = XMLNode::parseString(str, tag, &res);

	if (datalen != nullptr)
		datalen[0] += res.nChars;

	return (res.error == eXMLErrorNone || (tag != nullptr && res.error == eXMLErrorMissingEndTag)) ? result.detach() : nullptr;
}

MIR_APP_DLL(HXML) xmlAddChild(HXML _n, LPCTSTR name, LPCTSTR text)
{
	XMLNode result = XMLNode(_n).addChild(name);
	if (text != nullptr)
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

MIR_APP_DLL(int) xmlToFile(HXML _n, LPCTSTR filename, int withformatting)
{
	return XMLNode(_n).writeToFile(filename, nullptr, withformatting);
}

MIR_APP_DLL(void) xmlAddAttr(HXML _n, LPCTSTR attrName, LPCTSTR attrValue)
{
	if (attrName != nullptr && attrValue != nullptr)
		XMLNode(_n).addAttribute(attrName, attrValue);
}

MIR_APP_DLL(void) xmlAddAttrInt(HXML _n, LPCTSTR attrName, int attrValue)
{
	wchar_t buf[40];
	_itow(attrValue, buf, 10);
	XMLNode(_n).addAttribute(attrName, buf);
}

MIR_APP_DLL(void) xmlFree(void* p)
{
	free(p);
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
