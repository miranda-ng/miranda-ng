/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_XML_H__
#define M_XML_H__

#include <tchar.h>
#include <m_core.h>

DECLARE_HANDLE(HXML);
typedef int XML_ELEMENT_POS; // XML_ELEMENT_POS is not interchangeable with simple indexes

enum XML_ELEMENT_TYPE
{
	XML_ELEM_TYPE_CHILD = 0,
	XML_ELEM_TYPE_ATTRIBUTE = 1,
	XML_ELEM_TYPE_TEXT = 2,
	XML_ELEM_TYPE_CLEAR = 3,
};

/// Enumeration for XML parse errors.
enum XMLError
{
	eXMLErrorNone = 0,
	eXMLErrorMissingEndTag,
	eXMLErrorNoXMLTagFound,
	eXMLErrorEmpty,
	eXMLErrorMissingTagName,
	eXMLErrorMissingEndTagName,
	eXMLErrorUnmatchedEndTag,
	eXMLErrorUnmatchedEndClearTag,
	eXMLErrorUnexpectedToken,
	eXMLErrorNoElements,
	eXMLErrorFileNotFound,
	eXMLErrorFirstTagNotFound,
	eXMLErrorUnknownCharacterEntity,
	eXMLErrorCharacterCodeAbove255,
	eXMLErrorCharConversionError,
	eXMLErrorCannotOpenWriteFile,
	eXMLErrorCannotWriteFile,

	eXMLErrorBase64DataSizeIsNotMultipleOf4,
	eXMLErrorBase64DecodeIllegalCharacter,
	eXMLErrorBase64DecodeTruncatedData,
	eXMLErrorBase64DecodeBufferTooSmall
};

EXTERN_C MIR_APP_DLL(HXML)    xmlCreateNode(LPCTSTR name, LPCTSTR text, char isDeclaration);
EXTERN_C MIR_APP_DLL(void)    xmlDestroyNode(HXML node);

EXTERN_C MIR_APP_DLL(HXML)    xmlParseString(LPCTSTR string, int *datalen, LPCTSTR tag);
EXTERN_C MIR_APP_DLL(LPTSTR)  xmlToString(HXML node, int *datalen);

EXTERN_C MIR_APP_DLL(HXML)    xmlAddChild(HXML parent, LPCTSTR name, LPCTSTR text);
EXTERN_C MIR_APP_DLL(void)    xmlAddChild2(HXML child, HXML parent);
EXTERN_C MIR_APP_DLL(HXML)    xmlCopyNode(HXML parent);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetChild(HXML parent, int number);
EXTERN_C MIR_APP_DLL(int)     xmlGetChildCount(HXML);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetChildByAttrValue(HXML parent, LPCTSTR name, LPCTSTR attrName, LPCTSTR attrValue);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetFirstChild(HXML parent);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetNthChild(HXML parent, LPCTSTR name, int i);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetNextChild(HXML parent, LPCTSTR name, int *i);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetChildByPath(HXML parent, LPCTSTR path, char createNodeIfMissing);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetNextNode(HXML node);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetName(HXML);
EXTERN_C MIR_APP_DLL(HXML)    xmlGetParent(HXML);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetText(HXML);
EXTERN_C MIR_APP_DLL(void)    xmlSetText(HXML, LPCTSTR);

EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetAttr(HXML, int i);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetAttrName(HXML, int i);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetAttrValue(HXML, LPCTSTR attrName);
EXTERN_C MIR_APP_DLL(int)     xmlGetAttrCount(HXML);
EXTERN_C MIR_APP_DLL(void)    xmlAddAttr(HXML, LPCTSTR attrName, LPCTSTR attrValue);
EXTERN_C MIR_APP_DLL(void)    xmlAddAttrInt(HXML, LPCTSTR attrName, int attrValue);

EXTERN_C MIR_APP_DLL(void)    xmlFree(void*);

// methods added in xml API v2
EXTERN_C MIR_APP_DLL(char)    xmlIsDeclaration(HXML);
EXTERN_C MIR_APP_DLL(LPTSTR)  xmlToStringWithFormatting(HXML node, int* datalen);
EXTERN_C MIR_APP_DLL(HXML)    xmlDeepCopy(HXML);
EXTERN_C MIR_APP_DLL(void)    xmlSetAttrByIndex(HXML, int i, LPCTSTR value);
EXTERN_C MIR_APP_DLL(void)    xmlSetAttrByName(HXML, LPCTSTR name, LPCTSTR value);
EXTERN_C MIR_APP_DLL(HXML)    xmlAddChildEx(HXML parent, LPCTSTR name, char isDeclaration, XML_ELEMENT_POS n);
EXTERN_C MIR_APP_DLL(void)    xmlAddChildEx2(HXML child, HXML parent, XML_ELEMENT_POS n);
EXTERN_C MIR_APP_DLL(int)     xmlGetTextCount(HXML);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetTextByIndex(HXML, int i);
EXTERN_C MIR_APP_DLL(void)    xmlAddText(HXML, LPCTSTR, XML_ELEMENT_POS n);
EXTERN_C MIR_APP_DLL(void)    xmlSetTextByIndex(HXML, int i, LPCTSTR);
EXTERN_C MIR_APP_DLL(int)     xmlGetClearCount(HXML);
EXTERN_C MIR_APP_DLL(LPCTSTR) xmlGetClear(HXML, int i, LPCTSTR *openTag, LPCTSTR *closeTag);
EXTERN_C MIR_APP_DLL(void)    xmlAddClear(HXML, LPCTSTR lpszValue, LPCTSTR openTag, LPCTSTR closeTag, XML_ELEMENT_POS n);
EXTERN_C MIR_APP_DLL(void)    xmlSetClear(HXML, int i, LPCTSTR lpszValue);
EXTERN_C MIR_APP_DLL(int)     xmlGetElementCount(HXML);
EXTERN_C MIR_APP_DLL(int)     xmlGetElement(HXML, XML_ELEMENT_POS n, XML_ELEMENT_TYPE *type, HXML *child, LPCTSTR *value, LPCTSTR *name, LPCTSTR *openTag, LPCTSTR *closeTag);

EXTERN_C MIR_APP_DLL(void)    xmlDeleteNodeContent(HXML); // forces the deletion of the content of this node and the subtree
EXTERN_C MIR_APP_DLL(void)    xmlDeleteAttrByIndex(HXML, int i);
EXTERN_C MIR_APP_DLL(void)    xmlDeleteAttrByName(HXML, LPCTSTR name);
EXTERN_C MIR_APP_DLL(void)    xmlDeleteText(HXML, int i);
EXTERN_C MIR_APP_DLL(void)    xmlDeleteClear(HXML, int i);

EXTERN_C MIR_APP_DLL(HXML)     xmlParseFile(LPCTSTR filename, int *datalen, LPCTSTR tag);
EXTERN_C MIR_APP_DLL(XMLError) xmlToFile(HXML node, LPCTSTR filename, int withformattiing);

EXTERN_C MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfChildByIndex(HXML, int i);
EXTERN_C MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfChildByNode(HXML, HXML);
EXTERN_C MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfChildByName(HXML, LPCTSTR name, int i);
EXTERN_C MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfText(HXML, int i);
EXTERN_C MIR_APP_DLL(XML_ELEMENT_POS) xmlPositionOfClear(HXML, int i);

#endif // M_XML_H__
