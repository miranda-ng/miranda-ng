/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#include <m_core.h>

/////////////////////////////////////////////////////////////////////////////////////////
// new API to replace the old one

#ifdef MIR_CORE_EXPORTS
#define TINYXML2_EXPORT 1
#else
#define TINYXML2_IMPORT 1
#endif

#include "../src/mir_core/src/tinyxml2.h"

typedef tinyxml2::XMLNode TiXmlNode;
typedef tinyxml2::XMLText TiXmlText;
typedef tinyxml2::XMLElement TiXmlElement;
typedef tinyxml2::XMLDocument TiXmlDocument;

typedef tinyxml2::XMLHandle TiXmlHandle;
typedef tinyxml2::XMLConstHandle TiXmlConst;

/////////////////////////////////////////////////////////////////////////////////////////
// safe wrappers

EXTERN_C MIR_CORE_DLL(int) XmlGetChildCount(const TiXmlElement*);

EXTERN_C MIR_CORE_DLL(TiXmlElement*) XmlAddChild(TiXmlElement*, const char *pszName);
EXTERN_C MIR_CORE_DLL(TiXmlElement*) XmlAddChildA(TiXmlElement*, const char *pszName, const char *ptszValue);
EXTERN_C MIR_CORE_DLL(TiXmlElement*) XmlAddChildI(TiXmlElement*, const char *pszName, int iValue);

EXTERN_C MIR_CORE_DLL(int) XmlGetChildInt(const TiXmlElement *hXml, const char *key);
EXTERN_C MIR_CORE_DLL(const char*) XmlGetChildText(const TiXmlElement *hXml, const char *key);
EXTERN_C MIR_CORE_DLL(const TiXmlElement*) XmlGetChildByTag(const TiXmlElement *hXml, const char *key, const char *attrName, const char *attrValue);
EXTERN_C MIR_CORE_DLL(const TiXmlElement*) XmlFirstChild(const TiXmlElement *hXml, const char *key = nullptr);

EXTERN_C MIR_CORE_DLL(void) XmlAddAttr(TiXmlElement*, const char *pszName, const char *ptszValue);
EXTERN_C MIR_CORE_DLL(const char*) XmlGetAttr(const TiXmlElement*, const char *pszName);

/////////////////////////////////////////////////////////////////////////////////////////
// simple element iterator
//
// allows traversing subnodes in a cycle like
// for (auto *pNode : TiXmlEnum(pRoot)) {

class TiXmlIterator
{
	const TiXmlElement *m_pCurr;

public:
	TiXmlIterator(const TiXmlElement *pNode) :
		m_pCurr(pNode)
	{
	}

	TiXmlIterator& operator=(const TiXmlElement *pNode)
	{
		m_pCurr = pNode;
		return *this;
	}

	// Prefix ++ overload
	TiXmlIterator& operator++()
	{
		if (m_pCurr)
			m_pCurr = m_pCurr->NextSiblingElement();
		return *this;
	}

	const TiXmlElement* operator*()
	{
		return m_pCurr;
	}

	bool operator!=(const TiXmlIterator &iterator)
	{
		return m_pCurr != iterator.m_pCurr;
	}
};

class TiXmlEnum
{
	const TiXmlElement *m_pFirst;

public:
	TiXmlEnum(const TiXmlNode *pNode)
	{
		m_pFirst = (pNode) ? pNode->FirstChildElement() : nullptr;
	}

	TiXmlIterator begin()
	{
		return TiXmlIterator(m_pFirst);
	}

	TiXmlIterator end()
	{
		return TiXmlIterator(nullptr);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// filtered element iterator
//
// allows traversing subnodes of the specified name in a cycle like
// for (auto *pNode : TiXmlFilter(pRoot, "element")) {

class TiXmlFilterIterator
{
	const TiXmlElement *m_pCurr;
	const char *m_pszFilter;

public:
	TiXmlFilterIterator(const TiXmlElement *pNode, const char *pszNodeName) :
		m_pszFilter(pszNodeName),
		m_pCurr(pNode)
	{
	}

	TiXmlFilterIterator& operator=(const TiXmlElement *pNode)
	{
		m_pCurr = pNode;
		return *this;
	}

	// Prefix ++ overload
	TiXmlFilterIterator& operator++()
	{
		if (m_pCurr)
			m_pCurr = m_pCurr->NextSiblingElement(m_pszFilter);
		return *this;
	}

	const TiXmlElement* operator*()
	{
		return m_pCurr;
	}

	bool operator!=(const TiXmlFilterIterator &iterator)
	{
		return m_pCurr != iterator.m_pCurr;
	}
};

class TiXmlFilter
{
	const TiXmlElement *m_pFirst;
	const char *m_pszFilter;

public:
	TiXmlFilter(const TiXmlNode *pNode, const char *pszNodeName) :
		m_pszFilter(pszNodeName)
	{
		m_pFirst = (pNode) ? pNode->FirstChildElement(pszNodeName) : nullptr;
	}

	TiXmlFilterIterator begin()
	{
		return TiXmlFilterIterator(m_pFirst, m_pszFilter);
	}

	TiXmlFilterIterator end()
	{
		return TiXmlFilterIterator(nullptr, nullptr);
	}
};

#endif // M_XML_H__
