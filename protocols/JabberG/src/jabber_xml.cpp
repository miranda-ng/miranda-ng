/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

XmlNodeIq::XmlNodeIq(const wchar_t *type, int id, LPCTSTR to) :
	XmlNode(L"iq")
{
	if (type != nullptr) *this << XATTR(L"type", type);
	if (to   != nullptr) *this << XATTR(L"to",   to);
	if (id   != -1  ) *this << XATTRID(id);
}

XmlNodeIq::XmlNodeIq(const wchar_t *type, LPCTSTR idStr, LPCTSTR to) :
	XmlNode(L"iq")
{
	if (type  != nullptr) *this << XATTR(L"type", type );
	if (to    != nullptr) *this << XATTR(L"to",   to   );
	if (idStr != nullptr) *this << XATTR(L"id",   idStr);
}

XmlNodeIq::XmlNodeIq(const wchar_t *type, HXML node, LPCTSTR to) :
	XmlNode(L"iq")
{
	if (type  != nullptr) *this << XATTR(L"type", type );
	if (to    != nullptr) *this << XATTR(L"to",   to   );
	if (node  != nullptr) {
		const wchar_t *iqId = XmlGetAttrValue(*this, L"id");
		if (iqId != nullptr) *this << XATTR(L"id", iqId);
	}
}

XmlNodeIq::XmlNodeIq(CJabberIqInfo *pInfo) :
	XmlNode(L"iq")
{
	if (pInfo) {
		if (pInfo->GetCharIqType() != nullptr) *this << XATTR(L"type", _A2T(pInfo->GetCharIqType()));
		if (pInfo->GetReceiver()   != nullptr) *this << XATTR(L"to", pInfo->GetReceiver());
		if (pInfo->GetIqId()       != -1)   *this << XATTRID(pInfo->GetIqId());
	}
}

XmlNodeIq::XmlNodeIq(const wchar_t *type, CJabberIqInfo *pInfo) :
	XmlNode(L"iq")
{
	if (type != nullptr) *this << XATTR(L"type", type);
	if (pInfo) {
		if (pInfo->GetFrom()  != nullptr) *this << XATTR(L"to", pInfo->GetFrom());
		if (pInfo->GetIdStr() != nullptr) *this << XATTR(L"id", pInfo->GetIdStr());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNode class members

XmlNode::XmlNode(LPCTSTR pszName)
{
	m_hXml = xmlCreateNode(T2UTF(pszName), nullptr, 0);
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
		m_hXml = nullptr;
}	}

/////////////////////////////////////////////////////////////////////////////////////////

HXML __fastcall operator<<(HXML node, const XCHILDNS& child)
{
	HXML res = XmlAddChild(node, child.name);
	XmlAddAttr(res, L"xmlns", child.ns);
	return res;
}

HXML __fastcall operator<<(HXML node, const XQUERY& child)
{
	HXML n = XmlAddChild(node, L"query");
	if (n)
		XmlAddAttr(n, L"xmlns", child.ns);
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
	wchar_t buf[60];
	_ui64tot(value, buf, 10);

    xmlAddAttr(hXml, T2UTF(pszName), T2UTF(buf));
}

void __fastcall XmlAddAttrID(HXML hXml, int id)
{
	wchar_t text[100];
	mir_snwprintf(text, _T(JABBER_IQID) L"%d", id);
	XmlAddAttr(hXml, L"id", text);
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
	return xmlAddChild(hXml, T2UTF(name), nullptr);
}

HXML __fastcall XmlAddChild(HXML hXml, LPCTSTR name, LPCTSTR value)
{
	return xmlAddChild(hXml, T2UTF(name), T2UTF(value));
}

HXML __fastcall XmlAddChild(HXML hXml, LPCTSTR name, int value)
{
	wchar_t buf[40];
	_itow(value, buf, 10);
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
	LPTSTR wszKey = mir_a2u(key);
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
	LPTSTR wszKey = mir_a2u(key), wszName = mir_a2u(attrName);
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

	if (!hXml || tag == nullptr || mir_wstrlen(tag) <= 0 || nth < 1)
		return nullptr;

	num = 1;
	for (i=0; ; i++) {
		HXML n = xmlGetChild(hXml, i);
		if (!n)
			break;
		if (!mir_wstrcmp(tag, XmlGetName(n))) {
			if (num == nth)
				return n;

			num++;
	}	}

	return nullptr;
}

LPCTSTR __fastcall XmlGetName(HXML xml)
{
	return xmlGetName(xml);
}

LPCTSTR __fastcall XmlGetText(HXML xml)
{
	return (xml) ? xmlGetText(xml) : nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

void XPath::ProcessPath(LookupInfo &info, bool bCreate)
{
	if (!info.nodeName) return;

	wchar_t *nodeName = (wchar_t *)alloca(sizeof(wchar_t) * (info.nodeName.length+1));
	mir_wstrncpy(nodeName, info.nodeName.p, info.nodeName.length+1);

	if (info.attrName && info.attrValue) {
		wchar_t *attrName = (wchar_t *)alloca(sizeof(wchar_t)* (info.attrName.length + 1));
		mir_wstrncpy(attrName, info.attrName.p, info.attrName.length + 1);
		wchar_t *attrValue = (wchar_t *)alloca(sizeof(wchar_t)* (info.attrValue.length + 1));
		mir_wstrncpy(attrValue, info.attrValue.p, info.attrValue.length + 1);
		HXML hXml = XmlGetChildByTag(m_hXml, nodeName, attrName, attrValue);

		m_hXml = (hXml || !bCreate) ? hXml : (m_hXml << XCHILD(nodeName) << XATTR(attrName, attrValue));
	}
	else if (info.nodeIndex) {
		int idx = _wtoi(info.nodeIndex.p);
		m_hXml = mir_wstrcmp(nodeName, L"*") ? XmlGetNthChild(m_hXml, nodeName, idx) : XmlGetChild(m_hXml, idx - 1);
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
			case '@':
				info.attrName.Begin(p + 1);
				state = S_ATTR_STEP;
				break;
			case '/':
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
			case '[':
				info.nodeName.End(p);
				state = S_NODE_OPENBRACKET;
				break;
			case '/':
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
			case '@':
				info.attrName.Begin(p + 1);
				state = S_NODE_ATTRNAME;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
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
			case ']':
				info.nodeIndex.End(p);
				state = S_NODE_CLOSEBRACKET;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
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
			case '=':
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
			case '\'':
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
			case '\'':
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
			case ']':
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
			case '/':
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
