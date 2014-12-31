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

#include "..\..\core\commonheaders.h"
#include "xmlParser.h"

static HXML xmlapiCreateNode(LPCTSTR name, LPCTSTR text, char isDeclaration)
{
	XMLNode result = XMLNode::createXMLTopNode(name, isDeclaration);
	if (text)
		result.updateText(text);
	return result.detach();
}

static void xmlapiDestroyNode(HXML n)
{
	XMLNode tmp; tmp.attach(n);
}

static HXML xmlapiParseFile(LPCTSTR str, int* datalen, LPCTSTR tag)
{
	if (str == NULL) return NULL;

	XMLResults res;
	XMLNode result = XMLNode::parseFile(str, tag, &res);

	if (datalen != NULL)
		datalen[0] += res.nChars;

	return (res.error == eXMLErrorNone || (tag != NULL && res.error == eXMLErrorMissingEndTag)) ? result.detach() : NULL;
}

static HXML xmlapiParseString(LPCTSTR str, int* datalen, LPCTSTR tag)
{
	if (str == NULL) return NULL;

	XMLResults res;
	XMLNode result = XMLNode::parseString(str, tag, &res);

	if (datalen != NULL)
		datalen[0] += res.nChars;

	return (res.error == eXMLErrorNone || (tag != NULL && res.error == eXMLErrorMissingEndTag)) ? result.detach() : NULL;
}

static HXML xmlapiAddChild(HXML _n, LPCTSTR name, LPCTSTR text)
{
	XMLNode result = XMLNode(_n).addChild(name);
	if (text != NULL)
		result.updateText(text);
	return result;
}

static void xmlapiAddChild2(HXML _child, HXML _parent)
{
	XMLNode child(_child), parent(_parent);
	parent.addChild(child);
}

static HXML xmlapiCopyNode(HXML _n)
{
	XMLNode result = XMLNode(_n);
	return result.detach();
}

static LPCTSTR xmlapiGetAttr(HXML _n, int i)
{
	return XMLNode(_n).getAttributeValue(i);
}

static int xmlapiGetAttrCount(HXML _n)
{
	return XMLNode(_n).nAttribute();
}

static LPCTSTR xmlapiGetAttrName(HXML _n, int i)
{
	return XMLNode(_n).getAttributeName(i);
}

static HXML xmlapiGetChild(HXML _n, int i)
{
	return XMLNode(_n).getChildNode(i);
}

static HXML xmlapiGetChildByAttrValue(HXML _n, LPCTSTR name, LPCTSTR attrName, LPCTSTR attrValue)
{
	return XMLNode(_n).getChildNodeWithAttribute(name, attrName, attrValue);
}

static int xmlapiGetChildCount(HXML _n)
{
	return XMLNode(_n).nChildNode();
}

static HXML xmlapiGetFirstChild(HXML _n)
{
	return XMLNode(_n).getChildNode(0);
}

static HXML xmlapiGetNthChild(HXML _n, LPCTSTR name, int i)
{
	return XMLNode(_n).getChildNode(name, i);
}

static HXML xmlapiGetNextChild(HXML _n, LPCTSTR name, int* i)
{
	return XMLNode(_n).getChildNode(name, i);
}

static HXML xmlapiGetNextNode(HXML _n)
{
	return XMLNode(_n).getNextNode();
}

static HXML xmlapiGetChildByPath(HXML _n, LPCTSTR path, char createNodeIfMissing)
{
	return XMLNode(_n).getChildNodeByPath(path, createNodeIfMissing);
}

static LPCTSTR xmlapiGetName(HXML _n)
{
	return XMLNode(_n).getName();
}

static HXML xmlapiGetParent(HXML _n)
{
	return XMLNode(_n).getParentNode();
}

static LPCTSTR xmlapiGetText(HXML _n)
{
	return XMLNode(_n).getInnerText();
}

static LPCTSTR xmlapiGetAttrValue(HXML _n, LPCTSTR attrName)
{
	return XMLNode(_n).getAttribute(attrName);
}

static void xmlapiSetText(HXML _n, LPCTSTR _text)
{
	XMLNode(_n).updateText(_text);
}

static LPTSTR xmlapiToString(HXML _n, int* datalen)
{
	return XMLNode(_n).createXMLString(0, datalen);
}

static XMLError xmlapiToFile(HXML _n, LPCTSTR filename, int withformatting)
{
	return XMLNode(_n).writeToFile(filename, NULL, withformatting);
}

static void xmlapiAddAttr(HXML _n, LPCTSTR attrName, LPCTSTR attrValue)
{
	if (attrName != NULL && attrValue != NULL)
		XMLNode(_n).addAttribute(attrName, attrValue);
}

static void xmlapiAddAttrInt(HXML _n, LPCTSTR attrName, int attrValue)
{
	TCHAR buf[40];
	_itot(attrValue, buf, 10);
	XMLNode(_n).addAttribute(attrName, buf);
}

static void xmlapiFree(void* p)
{
	free(p);
}

// XML API v2 methods
static int xmlapiGetTextCount(HXML _n)
{
	return XMLNode(_n).nText();
}

static LPCTSTR xmlapiGetTextByIndex(HXML _n, int i)
{
	return XMLNode(_n).getText(i);
}

static void xmlapiSetTextByIndex(HXML _n, int i, LPCTSTR value)
{
	XMLNode(_n).updateText(value, i);
}

static void xmlapiAddText(HXML _n, LPCTSTR value, XML_ELEMENT_POS pos)
{
	XMLNode(_n).addText(value, (XMLElementPosition)pos);
}

static LPTSTR xmlapiToStringWithFormatting(HXML _n, int* datalen)
{
	return XMLNode(_n).createXMLString(1, datalen);
}

static int xmlapiGetClearCount(HXML _n)
{
	return XMLNode(_n).nClear();
}

static LPCTSTR xmlapiGetClear(HXML _n, int i, LPCTSTR *openTag, LPCTSTR *closeTag)
{
	XMLClear c = XMLNode(_n).getClear(i);
	if (openTag)
		*openTag = c.lpszOpenTag;
	if (closeTag)
		*closeTag = c.lpszCloseTag;
	return c.lpszValue;
}

static void xmlapiAddClear(HXML _n, LPCTSTR lpszValue, LPCTSTR openTag, LPCTSTR closeTag, XML_ELEMENT_POS pos)
{
	XMLNode(_n).addClear(lpszValue, openTag, closeTag, (XMLElementPosition)pos);
}

static void xmlapiSetClear(HXML _n, int i, LPCTSTR lpszValue)
{
	XMLNode(_n).updateClear(lpszValue, i);
}

static int xmlapiGetElement(HXML _n, XML_ELEMENT_POS pos, XML_ELEMENT_TYPE *type, HXML *child, LPCTSTR *value, LPCTSTR *name, LPCTSTR *openTag, LPCTSTR *closeTag)
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
		{
			*type = XML_ELEM_TYPE_CHILD;
			if (child)
				*child = c.child;
		} break;
		case eNodeAttribute:
		{
			*type = XML_ELEM_TYPE_ATTRIBUTE;
			if (name)
				*name = c.attrib.lpszName;
			if (value)
				*value = c.attrib.lpszValue;
		} break;
		case eNodeText:
		{
			*type = XML_ELEM_TYPE_TEXT;
			if (value)
				*value = c.text;
		} break;
		case eNodeClear:
		{
			*type = XML_ELEM_TYPE_CLEAR;
			if (value)
				*value = c.clear.lpszValue;
			if (openTag)
				*openTag = c.clear.lpszOpenTag;
			if (closeTag)
				*closeTag = c.clear.lpszCloseTag;
		} break;
		case eNodeNULL:
		{
			return false;
		} break;
	}
	return true;
}

static int xmlapiGetElementCount(HXML _n)
{
	return XMLNode(_n).nElement();
}

static char xmlapiIsDeclaration(HXML _n)
{
	return XMLNode(_n).isDeclaration();
}

static HXML xmlapiDeepCopy(HXML _n)
{
	return XMLNode(_n).deepCopy().detach();
}

static HXML xmlapiAddChildEx(HXML _n, LPCTSTR name, char isDeclaration, XML_ELEMENT_POS pos)
{
	return XMLNode(_n).addChild(name, isDeclaration, (XMLElementPosition)pos);
}

static void xmlapiAddChildEx2(HXML _n, HXML parent, XML_ELEMENT_POS pos)
{
	XMLNode(_n).addChild(parent, (XMLElementPosition)pos);
}

static void xmlapiSetAttrByIndex(HXML _n, int i, LPCTSTR value)
{
	XMLNode(_n).updateAttribute(value, NULL, i);
}

static void xmlapiSetAttrByName(HXML _n, LPCTSTR name, LPCTSTR value)
{
	XMLNode(_n).updateAttribute(value, NULL, name);
}

static void xmlapiDeleteNodeContent(HXML _n)
{
	XMLNode(_n).deleteNodeContent();
}

static void xmlapiDeleteAttrByIndex(HXML _n, int i)
{
	XMLNode(_n).deleteAttribute(i);
}

static void xmlapiDeleteAttrByName(HXML _n, LPCTSTR name)
{
	XMLNode(_n).deleteAttribute(name);
}

static void xmlapiDeleteText(HXML _n, int i)
{
	XMLNode(_n).deleteText(i);
}

static void xmlapiDeleteClear(HXML _n, int i)
{
	XMLNode(_n).deleteClear(i);
}

static XML_ELEMENT_POS xmlapiPositionOfText(HXML _n, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfText(i);
}

static XML_ELEMENT_POS xmlapiPositionOfClear(HXML _n, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfClear(i);
}

static XML_ELEMENT_POS xmlapiPositionOfChildByIndex(HXML _n, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfChildNode(i);
}

static XML_ELEMENT_POS xmlapiPositionOfChildByNode(HXML _n, HXML child)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfChildNode(child);
}

static XML_ELEMENT_POS xmlapiPositionOfChildByName(HXML _n, LPCTSTR name, int i)
{
	return (XML_ELEMENT_POS)XMLNode(_n).positionOfChildNode(name, i);
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR GetXmlApi(WPARAM, LPARAM lParam)
{
	XML_API* xi = (XML_API*)lParam;
	if (xi == NULL)
		return FALSE;

	if (xi->cbSize != XML_API_SIZEOF_V1 && xi->cbSize != sizeof(XML_API))
		return FALSE;

	xi->createNode = xmlapiCreateNode;
	xi->destroyNode = xmlapiDestroyNode;

	xi->parseString = xmlapiParseString;
	xi->toString = xmlapiToString;
	xi->freeMem = xmlapiFree;
	xi->parseFile = xmlapiParseFile;
	xi->toFile = xmlapiToFile;

	xi->addChild = xmlapiAddChild;
	xi->addChild2 = xmlapiAddChild2;
	xi->copyNode = xmlapiCopyNode;
	xi->getChild = xmlapiGetChild;
	xi->getChildByAttrValue = xmlapiGetChildByAttrValue;
	xi->getChildCount = xmlapiGetChildCount;
	xi->getFirstChild = xmlapiGetFirstChild;
	xi->getNthChild = xmlapiGetNthChild;
	xi->getNextChild = xmlapiGetNextChild;
	xi->getNextNode = xmlapiGetNextNode;
	xi->getChildByPath = xmlapiGetChildByPath;
	xi->getName = xmlapiGetName;
	xi->getParent = xmlapiGetParent;
	xi->getText = xmlapiGetText;
	xi->setText = xmlapiSetText;

	xi->getAttr = xmlapiGetAttr;
	xi->getAttrCount = xmlapiGetAttrCount;
	xi->getAttrName = xmlapiGetAttrName;
	xi->getAttrValue = xmlapiGetAttrValue;
	xi->addAttr = xmlapiAddAttr;
	xi->addAttrInt = xmlapiAddAttrInt;

	if (xi->cbSize > XML_API_SIZEOF_V1) {
		xi->isDeclaration = xmlapiIsDeclaration;
		xi->toStringWithFormatting = xmlapiToStringWithFormatting;
		xi->deepCopy = xmlapiDeepCopy;
		xi->setAttrByIndex = xmlapiSetAttrByIndex;
		xi->setAttrByName = xmlapiSetAttrByName;
		xi->addChildEx = xmlapiAddChildEx;
		xi->addChildEx2 = xmlapiAddChildEx2;
		xi->getTextCount = xmlapiGetTextCount;
		xi->getTextByIndex = xmlapiGetTextByIndex;
		xi->addText = xmlapiAddText;
		xi->setTextByIndex = xmlapiSetTextByIndex;
		xi->getClearCount = xmlapiGetClearCount;
		xi->getClear = xmlapiGetClear;
		xi->addClear = xmlapiAddClear;
		xi->setClear = xmlapiSetClear;
		xi->getElementCount = xmlapiGetElementCount;
		xi->getElement = xmlapiGetElement;

		xi->deleteNodeContent = xmlapiDeleteNodeContent;
		xi->deleteAttrByIndex = xmlapiDeleteAttrByIndex;
		xi->deleteAttrByName = xmlapiDeleteAttrByName;
		xi->deleteText = xmlapiDeleteText;
		xi->deleteClear = xmlapiDeleteClear;

		xi->positionOfChildByIndex = xmlapiPositionOfChildByIndex;
		xi->positionOfChildByNode = xmlapiPositionOfChildByNode;
		xi->positionOfChildByName = xmlapiPositionOfChildByName;
		xi->positionOfText = xmlapiPositionOfText;
		xi->positionOfClear = xmlapiPositionOfClear;
	}
	return TRUE;
}

void InitXmlApi(void)
{
	CreateServiceFunction(MS_SYSTEM_GET_XI, GetXmlApi);
}
