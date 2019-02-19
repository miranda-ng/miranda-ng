/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-19 Miranda NG team

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

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNodeIq class members

XmlNodeIq::XmlNodeIq(const char *type, int id, const char *to) :
	XmlNode("iq")
{
	if (type != nullptr) *this << XATTR("type", type);
	if (to   != nullptr) *this << XATTR("to",   to);
	if (id   != -1  ) *this << XATTRID(id);
}

XmlNodeIq::XmlNodeIq(const char *type, const char *idStr, const char *to) :
	XmlNode("iq")
{
	if (type  != nullptr) *this << XATTR("type", type );
	if (to    != nullptr) *this << XATTR("to",   to   );
	if (idStr != nullptr) *this << XATTR("id",   idStr);
}

XmlNodeIq::XmlNodeIq(const char *type, TiXmlElement *node, const char *to) :
	XmlNode("iq")
{
	if (type  != nullptr) *this << XATTR("type", type );
	if (to    != nullptr) *this << XATTR("to",   to   );
	if (node  != nullptr) {
		const char *iqId = node->Attribute("id");
		if (iqId != nullptr)
			*this << XATTR("id", iqId);
	}
}

XmlNodeIq::XmlNodeIq(CJabberIqInfo *pInfo) :
	XmlNode("iq")
{
	if (pInfo) {
		if (pInfo->GetCharIqType() != nullptr) *this << XATTR("type", pInfo->GetCharIqType());
		if (pInfo->GetReceiver()   != nullptr) *this << XATTR("to", pInfo->GetReceiver());
		if (pInfo->GetIqId()       != -1)   *this << XATTRID(pInfo->GetIqId());
	}
}

XmlNodeIq::XmlNodeIq(const char *type, CJabberIqInfo *pInfo) :
	XmlNode("iq")
{
	if (type != nullptr) *this << XATTR("type", type);
	if (pInfo) {
		if (pInfo->GetFrom()  != nullptr) *this << XATTR("to", pInfo->GetFrom());
		if (pInfo->GetIdStr() != nullptr) *this << XATTR("id", pInfo->GetIdStr());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNode class members

XmlNode::XmlNode(const char *pszName)
{
	m_hXml = NewElement(pszName); InsertEndChild(m_hXml);
}

XmlNode::XmlNode(const char *pszName, const char *ptszText)
{
	m_hXml = NewElement(pszName); InsertEndChild(m_hXml);
	m_hXml->SetText(ptszText);
}

/////////////////////////////////////////////////////////////////////////////////////////

TiXmlElement*operator<<(TiXmlElement *node, const XCHILDNS &child)
{
	TiXmlElement *res = XmlAddChild(node, child.name);
	res->SetAttribute("xmlns", child.ns);
	return res;
}

TiXmlElement* operator<<(TiXmlElement *node, const XQUERY &child)
{
	TiXmlElement *n = node->GetDocument()->NewElement("query");
	if (n)
		n->SetAttribute("xmlns", child.ns);
	return n;
}

/////////////////////////////////////////////////////////////////////////////////////////

void XmlAddAttr(TiXmlElement *hXml, const char *name, const char *value)
{
	if (value)
		hXml->SetAttribute(name, value);
}

void XmlAddAttrID(TiXmlElement *hXml, int id)
{
	char text[100];
	mir_snprintf(text, JABBER_IQID "%d", id);
	hXml->SetAttribute("id", text);
}

/////////////////////////////////////////////////////////////////////////////////////////

TiXmlElement* XmlAddChild(TiXmlElement *hXml, const char *name)
{
	if (hXml == nullptr)
		return nullptr;

	auto *res = hXml->GetDocument()->NewElement(name);
	hXml->InsertEndChild(res);
	return res;
}

TiXmlElement* XmlAddChild(TiXmlElement *hXml, const char *name, const char *value)
{
	if (hXml == nullptr)
		return nullptr;

	auto *res = hXml->GetDocument()->NewElement(name);
	if (value)
		res->SetText(value);
	hXml->InsertEndChild(res);
	return res;
}

TiXmlElement* XmlAddChild(TiXmlElement *hXml, const char *name, int value)
{
	if (hXml == nullptr)
		return nullptr;

	auto *res = hXml->GetDocument()->NewElement(name);
	if (value)
		res->SetText(value);
	hXml->InsertEndChild(res);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

const char* XmlGetChildText(const TiXmlElement *hXml, const char *key)
{
	if (hXml == nullptr)
		return nullptr;

	auto *pChild = hXml->FirstChildElement(key);
	return (pChild == nullptr) ? nullptr : pChild->GetText();
}

int XmlGetChildInt(const TiXmlElement *hXml, const char *key)
{
	if (hXml == nullptr)
		return 0;

	auto *pChild = hXml->FirstChildElement(key);
	return (pChild == nullptr) ? 0 : atoi(pChild->GetText());
}

const TiXmlElement* XmlGetChildByTag(const TiXmlElement *hXml, const char *key, const char *attrName, const char *attrValue)
{
	if (hXml == nullptr)
		return nullptr;

	auto *pChild = hXml->FirstChildElement(key);
	if (pChild == nullptr)
		return nullptr;

	return (pChild->Attribute(attrName, attrValue)) ? pChild : nullptr;
}

int XmlGetChildCount(const TiXmlElement *hXml)
{
	int iCount = 0;
	for (auto *it : TiXmlEnum(hXml)) {
		UNREFERENCED_PARAMETER(it);
		iCount++;
	}
	return iCount;
}

/////////////////////////////////////////////////////////////////////////////////////////

void XPath::ProcessPath(LookupInfo &info)
{
	if (!info.nodeName) return;

	char *nodeName = (char *)alloca(sizeof(char) * (info.nodeName.length+1));
	mir_strncpy(nodeName, info.nodeName.p, info.nodeName.length+1);

	if (info.attrName && info.attrValue) {
		char *attrName = (char *)alloca(sizeof(char)* (info.attrName.length + 1));
		mir_strncpy(attrName, info.attrName.p, info.attrName.length + 1);
		char *attrValue = (char *)alloca(sizeof(char)* (info.attrValue.length + 1));
		mir_strncpy(attrValue, info.attrValue.p, info.attrValue.length + 1);
		m_hXml = XmlGetChildByTag(m_hXml, nodeName, attrName, attrValue);
	}
	else m_hXml = m_hXml->FirstChildElement(nodeName);

	info.Reset();
}

XPath::PathType XPath::LookupImpl()
{
	LookupState state = S_START;
	LookupInfo info = {};

	for (const char *p = m_szPath; state < S_FINAL; ++p) {
		switch (state) {
		case S_START:
			ProcessPath(info);
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
		ProcessPath(info);
		return T_NODE;
	case S_FINAL_NODESET:
		m_szParam = info.nodeName.p;
		return T_NODESET;
	}

	return T_ERROR;
}
