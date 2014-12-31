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
#include <newpluginapi.h>

DECLARE_HANDLE(HXML);
typedef int XML_ELEMENT_POS; // XML_ELEMENT_POS is not interchangeable with simple indexes

typedef enum
{
	XML_ELEM_TYPE_CHILD = 0,
	XML_ELEM_TYPE_ATTRIBUTE = 1,
	XML_ELEM_TYPE_TEXT = 2,
	XML_ELEM_TYPE_CLEAR = 3,
}
	XML_ELEMENT_TYPE;

/// Enumeration for XML parse errors.
typedef enum XMLError
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
} XMLError;

typedef struct
{
	size_t cbSize;

	HXML    (*createNode)(LPCTSTR name, LPCTSTR text, char isDeclaration);
	void    (*destroyNode)(HXML node);

	HXML    (*parseString)(LPCTSTR string, int* datalen, LPCTSTR tag);
	LPTSTR  (*toString)(HXML node, int* datalen);

	HXML    (*addChild)(HXML parent, LPCTSTR name, LPCTSTR text);
	void    (*addChild2)(HXML child, HXML parent);
	HXML    (*copyNode)(HXML parent);
	HXML    (*getChild)(HXML parent, int number);
	int     (*getChildCount)(HXML);
	HXML    (*getChildByAttrValue)(HXML parent, LPCTSTR name, LPCTSTR attrName, LPCTSTR attrValue);
	HXML    (*getFirstChild)(HXML parent);
	HXML    (*getNthChild)(HXML parent, LPCTSTR name, int i);
	HXML    (*getNextChild)(HXML parent, LPCTSTR name, int *i);
	HXML    (*getChildByPath)(HXML parent, LPCTSTR path, char createNodeIfMissing);
	HXML    (*getNextNode)(HXML node);
	LPCTSTR (*getName)(HXML);
	HXML    (*getParent)(HXML);
	LPCTSTR (*getText)(HXML); //=getTextByIndex(HXML, 0)
	void    (*setText)(HXML, LPCTSTR); //=setTextByIndex(HXML, LPCTSTR, 0)

	LPCTSTR (*getAttr)(HXML, int i);
	LPCTSTR (*getAttrName)(HXML, int i);
	LPCTSTR (*getAttrValue)(HXML, LPCTSTR attrName);
	int     (*getAttrCount)(HXML);
	void    (*addAttr)(HXML, LPCTSTR attrName, LPCTSTR attrValue);
	void    (*addAttrInt)(HXML, LPCTSTR attrName, int attrValue);

	void    (*freeMem)(void*);

	// methods added in XML API v2
	char    (*isDeclaration)(HXML);
	LPTSTR  (*toStringWithFormatting)(HXML node, int* datalen);
	HXML    (*deepCopy)(HXML);
	void	(*setAttrByIndex)(HXML, int i, LPCTSTR value);
	void	(*setAttrByName)(HXML, LPCTSTR name, LPCTSTR value);
	HXML    (*addChildEx)(HXML parent, LPCTSTR name, char isDeclaration, XML_ELEMENT_POS n);
	void    (*addChildEx2)(HXML child, HXML parent, XML_ELEMENT_POS n);
	int     (*getTextCount)(HXML);
	LPCTSTR (*getTextByIndex)(HXML, int i);
	void    (*addText)(HXML, LPCTSTR, XML_ELEMENT_POS n);
	void    (*setTextByIndex)(HXML, int i, LPCTSTR);
	int     (*getClearCount)(HXML);
	LPCTSTR (*getClear)(HXML, int i, LPCTSTR *openTag, LPCTSTR *closeTag);
	void    (*addClear)(HXML, LPCTSTR lpszValue, LPCTSTR openTag, LPCTSTR closeTag, XML_ELEMENT_POS n);
	void    (*setClear)(HXML, int i, LPCTSTR lpszValue);
	int     (*getElementCount)(HXML);
	int     (*getElement)(HXML, XML_ELEMENT_POS n, XML_ELEMENT_TYPE *type, HXML *child, LPCTSTR *value, LPCTSTR *name, LPCTSTR *openTag, LPCTSTR *closeTag);
	// With getElement() it's possible to enumerate all the different contents (attribute, child, text, clear) of the current node. The order is reflecting the order of the original file/string. NOTE: 0 <= i < getElementCount().
	// type, child, value, name, openTag, closeTag will be filled on return, depending on type:
	// for XML_ELEM_TYPE_CHILD, child is valid;
	// for XML_ELEM_TYPE_ATTRIBUTE, name and value are valid;
	// for XML_ELEM_TYPE_TEXT, value is valid;
	// for XML_ELEM_TYPE_CLEAR, value, openTag and closeTag are valid.

	void    (*deleteNodeContent)(HXML); // forces the deletion of the content of this node and the subtree
	void	(*deleteAttrByIndex)(HXML, int i);
	void	(*deleteAttrByName)(HXML, LPCTSTR name);
	void	(*deleteText)(HXML, int i);
	void	(*deleteClear)(HXML, int i);

	XML_ELEMENT_POS (*positionOfChildByIndex)(HXML, int i);
	XML_ELEMENT_POS (*positionOfChildByNode)(HXML, HXML);
	XML_ELEMENT_POS (*positionOfChildByName)(HXML, LPCTSTR name, int i);
	XML_ELEMENT_POS (*positionOfText)(HXML, int i);
	XML_ELEMENT_POS (*positionOfClear)(HXML, int i);

	HXML	(*parseFile)(LPCTSTR filename, int* datalen, LPCTSTR tag);
	XMLError (*toFile)(HXML node, LPCTSTR filename, int withformattiing);
}
	XML_API;

#define XML_API_SIZEOF_V1 (sizeof(size_t)+26*sizeof(void*))

/* every protocol should declare this variable to use the XML API */
extern XML_API xi;

/*
a service to obtain the XML API

wParam = 0;
lParam = (LPARAM)(XML_API*).

returns TRUE if all is Ok, and FALSE otherwise
*/

#define MS_SYSTEM_GET_XI "Miranda/System/GetXmlApi"

__forceinline int mir_getXI(XML_API* dest)
{
	dest->cbSize = sizeof(*dest);
	return CallService(MS_SYSTEM_GET_XI, 0, (LPARAM)dest);
}

#endif // M_XML_H__
