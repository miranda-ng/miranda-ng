/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (�) 2012-16 Miranda NG project

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

#define TAG_MAX_LEN 128
#define ATTR_MAX_LEN 8192

#define T2UTF(A) A

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNodeIq class members

XmlNodeIq::XmlNodeIq(const TCHAR *type, int id, LPCTSTR to) :
	XmlNode(_T("iq"))
{
	if (type != NULL) *this << XATTR(_T("type"), type);
	if (to   != NULL) *this << XATTR(_T("to"),   to);
	if (id   != -1  ) *this << XATTRID(id);
}

XmlNodeIq::XmlNodeIq(const TCHAR *type, LPCTSTR idStr, LPCTSTR to) :
	XmlNode(_T("iq"))
{
	if (type  != NULL) *this << XATTR(_T("type"), type );
	if (to    != NULL) *this << XATTR(_T("to"),   to   );
	if (idStr != NULL) *this << XATTR(_T("id"),   idStr);
}

XmlNodeIq::XmlNodeIq(const TCHAR *type, HXML node, LPCTSTR to) :
	XmlNode(_T("iq"))
{
	if (type  != NULL) *this << XATTR(_T("type"), type );
	if (to    != NULL) *this << XATTR(_T("to"),   to   );
	if (node  != NULL) {
		const TCHAR *iqId = XmlGetAttrValue(*this, _T("id"));
		if (iqId != NULL) *this << XATTR(_T("id"), iqId);
	}
}

XmlNodeIq::XmlNodeIq(CJabberIqInfo *pInfo) :
	XmlNode(_T("iq"))
{
	if (pInfo) {
		if (pInfo->GetCharIqType() != NULL) *this << XATTR(_T("type"), _A2T(pInfo->GetCharIqType()));
		if (pInfo->GetReceiver()   != NULL) *this << XATTR(_T("to"), pInfo->GetReceiver());
		if (pInfo->GetIqId()       != -1)   *this << XATTRID(pInfo->GetIqId());
	}
}

XmlNodeIq::XmlNodeIq(const TCHAR *type, CJabberIqInfo *pInfo) :
	XmlNode(_T("iq"))
{
	if (type != NULL) *this << XATTR(_T("type"), type);
	if (pInfo) {
		if (pInfo->GetFrom()  != NULL) *this << XATTR(_T("to"), pInfo->GetFrom());
		if (pInfo->GetIdStr() != NULL) *this << XATTR(_T("id"), pInfo->GetIdStr());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNode class members

XmlNode::XmlNode(LPCTSTR pszName)
{
	m_hXml = xmlCreateNode(T2UTF(pszName), NULL, 0);
}

XmlNode::XmlNode(LPCTSTR pszName, LPCTSTR ptszText)
{
	m_hXml = xmlCreateNode(T2UTF(pszName), ptszText, 0);
}

XmlNode::XmlNode(const XmlNode& n)
{
	m_hXml = xmlCopyNode(n);
}

XmlNode& XmlNode::operator =(const XmlNode& n)
{
	if (m_hXml)
		xmlDestroyNode(m_hXml);
	m_hXml = xmlCopyNode(n);
	return *this;
}

XmlNode::~XmlNode()
{
	if (m_hXml) {
		xmlDestroyNode(m_hXml);
		m_hXml = NULL;
}	}

/////////////////////////////////////////////////////////////////////////////////////////

HXML __fastcall operator<<(HXML node, const XCHILDNS& child)
{
	HXML res = XmlAddChild(node, child.name);
	XmlAddAttr(res, _T("xmlns"), child.ns);
	return res;
}

HXML __fastcall operator<<(HXML node, const XQUERY& child)
{
	HXML n = XmlAddChild(node, _T("query"));
	if (n)
		XmlAddAttr(n, _T("xmlns"), child.ns);
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////

void __fastcall XmlAddAttr(HXML hXml, LPCTSTR name, LPCTSTR value)
{
	if (value)
		xmlAddAttr(hXml, name, T2UTF(value));
}

void __fastcall XmlAddAttr(HXML hXml, LPCTSTR pszName, int value)
{
	xmlAddAttrInt(hXml, T2UTF(pszName), value);
}

void __fastcall XmlAddAttr(HXML hXml, LPCTSTR pszName, unsigned __int64 value)
{
	TCHAR buf[60];
	_ui64tot(value, buf, 10);

    xmlAddAttr(hXml, T2UTF(pszName), T2UTF(buf));
}

void __fastcall XmlAddAttrID(HXML hXml, int id)
{
	TCHAR text[100];
	mir_sntprintf(text, _T(JABBER_IQID) _T("%d"), id);
	XmlAddAttr(hXml, _T("id"), text);
}

/////////////////////////////////////////////////////////////////////////////////////////

LPCTSTR __fastcall XmlGetAttr(HXML hXml, int n)
{
	return xmlGetAttr(hXml, n);
}

int __fastcall XmlGetAttrCount(HXML hXml)
{
	return xmlGetAttrCount(hXml);
}

LPCTSTR __fastcall XmlGetAttrName(HXML hXml, int n)
{
	return xmlGetAttrName(hXml, n);
}

/////////////////////////////////////////////////////////////////////////////////////////

void __fastcall XmlAddChild(HXML hXml, HXML n)
{
	xmlAddChild2(n, hXml);
}

HXML __fastcall XmlAddChild(HXML hXml, LPCTSTR name)
{
	return xmlAddChild(hXml, T2UTF(name), NULL);
}

HXML __fastcall XmlAddChild(HXML hXml, LPCTSTR name, LPCTSTR value)
{
	return xmlAddChild(hXml, T2UTF(name), T2UTF(value));
}

HXML __fastcall XmlAddChild(HXML hXml, LPCTSTR name, int value)
{
	TCHAR buf[40];
	_itot(value, buf, 10);
	return xmlAddChild(hXml, T2UTF(name), buf);
}

/////////////////////////////////////////////////////////////////////////////////////////

LPCTSTR __fastcall XmlGetAttrValue(HXML hXml, LPCTSTR key)
{
	return xmlGetAttrValue(hXml, key);
}

HXML __fastcall XmlGetChild(HXML hXml, int n)
{
	return xmlGetChild(hXml, n);
}

HXML __fastcall XmlGetChild(HXML hXml, LPCTSTR key)
{
	return xmlGetNthChild(hXml, key, 0);
}

HXML __fastcall XmlGetChild(HXML hXml, LPCSTR key)
{
	LPTSTR wszKey = mir_a2t(key);
	HXML result = xmlGetNthChild(hXml, wszKey, 0);
	mir_free(wszKey);
	return result;
}

HXML __fastcall XmlGetChildByTag(HXML hXml, LPCTSTR key, LPCTSTR attrName, LPCTSTR attrValue)
{
	return xmlGetChildByAttrValue(hXml, key, attrName, attrValue);
}

HXML __fastcall XmlGetChildByTag(HXML hXml, LPCSTR key, LPCSTR attrName, LPCTSTR attrValue)
{
	LPTSTR wszKey = mir_a2t(key), wszName = mir_a2t(attrName);
	HXML result = xmlGetChildByAttrValue(hXml, wszKey, wszName, attrValue);
	mir_free(wszKey), mir_free(wszName);
	return result;
}

int __fastcall XmlGetChildCount(HXML hXml)
{
	return xmlGetChildCount(hXml);
}

HXML __fastcall XmlGetNthChild(HXML hXml, LPCTSTR tag, int nth)
{
	int i, num;

	if (!hXml || tag == NULL || mir_tstrlen(tag) <= 0 || nth < 1)
		return NULL;

	num = 1;
	for (i=0; ; i++) {
		HXML n = xmlGetChild(hXml, i);
		if (!n)
			break;
		if (!mir_tstrcmp(tag, XmlGetName(n))) {
			if (num == nth)
				return n;

			num++;
	}	}

	return NULL;
}

LPCTSTR __fastcall XmlGetName(HXML xml)
{
	return xmlGetName(xml);
}

LPCTSTR __fastcall XmlGetText(HXML xml)
{
	return (xml) ? xmlGetText(xml) : NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////

void XPath::ProcessPath(LookupInfo &info, bool bCreate)
{
	if (!info.nodeName) return;

	TCHAR *nodeName = (TCHAR *)alloca(sizeof(TCHAR) * (info.nodeName.length+1));
	mir_tstrncpy(nodeName, info.nodeName.p, info.nodeName.length+1);

	if (info.attrName && info.attrValue) {
		TCHAR *attrName = (TCHAR *)alloca(sizeof(TCHAR)* (info.attrName.length + 1));
		mir_tstrncpy(attrName, info.attrName.p, info.attrName.length + 1);
		TCHAR *attrValue = (TCHAR *)alloca(sizeof(TCHAR)* (info.attrValue.length + 1));
		mir_tstrncpy(attrValue, info.attrValue.p, info.attrValue.length + 1);
		HXML hXml = XmlGetChildByTag(m_hXml, nodeName, attrName, attrValue);

		m_hXml = (hXml || !bCreate) ? hXml : (m_hXml << XCHILD(nodeName) << XATTR(attrName, attrValue));
	}
	else if (info.nodeIndex) {
		int idx = _ttoi(info.nodeIndex.p);
		m_hXml = mir_tstrcmp(nodeName, _T("*")) ? XmlGetNthChild(m_hXml, nodeName, idx) : XmlGetChild(m_hXml, idx - 1);
	}
	else {
		HXML hXml = XmlGetChild(m_hXml, nodeName);
		m_hXml = (hXml || !bCreate) ? hXml : (m_hXml << XCHILD(nodeName));
	}

	info.Reset();
}

XPath::PathType XPath::LookupImpl(bool bCreate)
{
	LookupState state = S_START;
	LookupInfo info = {0};

	for (LPCTSTR p = m_szPath; state < S_FINAL; ++p) {
		switch (state) {
		case S_START:
			ProcessPath(info, bCreate);
			if (!m_hXml) {
				state = S_FINAL_ERROR;
				break;
			}

			switch (*p) {
			case 0:
				state = S_FINAL_ERROR;
				break;
			case _T('@'):
				info.attrName.Begin(p + 1);
				state = S_ATTR_STEP;
				break;
			case _T('/'):
				break;
			default:
				info.nodeName.Begin(p);
				state = S_NODE_NAME;
				break;
			};
			break;

		case S_ATTR_STEP:
			switch (*p) {
			case 0:
				info.attrName.End(p);
				state = S_FINAL_ATTR;
				break;
			default:
				break;
			};
			break;

		case S_NODE_NAME:
			switch (*p) {
			case 0:
				info.nodeName.End(p);
				state = S_FINAL_NODESET;
				break;
			case _T('['):
				info.nodeName.End(p);
				state = S_NODE_OPENBRACKET;
				break;
			case _T('/'):
				info.nodeName.End(p);
				state = S_START;
				break;
			default:
				break;
			};
			break;

		case S_NODE_OPENBRACKET:
			switch (*p) {
			case 0:
				state = S_FINAL_ERROR;
				break;
			case _T('@'):
				info.attrName.Begin(p + 1);
				state = S_NODE_ATTRNAME;
				break;
			case _T('0'): case _T('1'): case _T('2'): case _T('3'): case _T('4'):
			case _T('5'): case _T('6'): case _T('7'): case _T('8'): case _T('9'):
				info.nodeIndex.Begin(p);
				state = S_NODE_INDEX;
				break;
			default:
				state = S_FINAL_ERROR;
				break;
			};
			break;

		case S_NODE_INDEX:
			switch (*p) {
			case 0:
				state = S_FINAL_ERROR;
				break;
			case _T(']'):
				info.nodeIndex.End(p);
				state = S_NODE_CLOSEBRACKET;
				break;
			case _T('0'): case _T('1'): case _T('2'): case _T('3'): case _T('4'):
			case _T('5'): case _T('6'): case _T('7'): case _T('8'): case _T('9'):
				break;
			default:
				state = S_FINAL_ERROR;
				break;
			};
			break;

		case S_NODE_ATTRNAME:
			switch (*p) {
			case 0:
				state = S_FINAL_ERROR;
				break;
			case _T('='):
				info.attrName.End(p);
				state = S_NODE_ATTREQUALS;
				break;
			default:
				break;
			};
			break;

		case S_NODE_ATTREQUALS:
			switch (*p) {
			case 0:
				state = S_FINAL_ERROR;
				break;
			case _T('\''):
				info.attrValue.Begin(p + 1);
				state = S_NODE_ATTRVALUE;
				break;
			default:
				state = S_FINAL_ERROR;
				break;
			};
			break;

		case S_NODE_ATTRVALUE:
			switch (*p) {
			case 0:
				state = S_FINAL_ERROR;
				break;
			case _T('\''):
				info.attrValue.End(p);
				state = S_NODE_ATTRCLOSEVALUE;
				break;
			default:
				break;
			};
			break;

		case S_NODE_ATTRCLOSEVALUE:
			switch (*p) {
			case 0:
				state = S_FINAL_ERROR;
				break;
			case _T(']'):
				state = S_NODE_CLOSEBRACKET;
				break;
			default:
				state = S_FINAL_ERROR;
				break;
			};
			break;

		case S_NODE_CLOSEBRACKET:
			switch (*p) {
			case 0:
				state = S_FINAL_NODE;
				break;
			case _T('/'):
				state = S_START;
				break;
			default:
				state = S_FINAL_ERROR;
				break;
			};
			break;
		}

		if (!*p && (state < S_FINAL))
			state = S_FINAL_ERROR;
	}

	switch (state) {
	case S_FINAL_ATTR:
		m_szParam = info.attrName.p;
		return T_ATTRIBUTE;
	case S_FINAL_NODE:
		ProcessPath(info, bCreate);
		return T_NODE;
	case S_FINAL_NODESET:
		m_szParam = info.nodeName.p;
		return T_NODESET;
	}

	return T_ERROR;
}
