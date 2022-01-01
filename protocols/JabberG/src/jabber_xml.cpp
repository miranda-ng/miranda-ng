/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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
// XmlNodeHash class members

XmlNodeHash::XmlNodeHash()
{
	mir_md5_init(&state);
}

bool XmlNodeHash::VisitEnter(const TiXmlElement &pNode, const tinyxml2::XMLAttribute *attr)
{
	add(pNode.Name());
	add(pNode.GetText());

	for (auto *p = attr; p != nullptr; p = p->Next()) {
		add(p->Name());
		add(p->Value());
	}

	return true;
}

void XmlNodeHash::add(const char *str)
{
	if (str)
		mir_md5_append(&state, (const uint8_t *)str, strlen(str));
}

CMStringA XmlNodeHash::getResult()
{
	uint8_t digest[16];
	mir_md5_finish(&state, digest);

	CMStringA res;
	res.Truncate(33);
	bin2hex(digest, sizeof(digest), res.GetBuffer());
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// XmlNodeIq class members

XmlNodeIq::XmlNodeIq(const char *type, int id, const char *to) :
	XmlNode("iq")
{
	if (type != nullptr)
		m_hXml->SetAttribute("type", type);
	if (to != nullptr)
		m_hXml->SetAttribute("to", to);
	if (id != -1)
		XmlAddAttrID(*this, id);
}

XmlNodeIq::XmlNodeIq(const char *type, const char *idStr, const char *to) :
	XmlNode("iq")
{
	if (type != nullptr)
		m_hXml->SetAttribute("type", type);
	if (to != nullptr)
		m_hXml->SetAttribute("to", to);
	if (idStr != nullptr)
		m_hXml->SetAttribute("id", idStr);
}

XmlNodeIq::XmlNodeIq(const char *type, TiXmlElement *node, const char *to) :
	XmlNode("iq")
{
	if (type != nullptr)
		m_hXml->SetAttribute("type", type);
	if (to != nullptr)
		m_hXml->SetAttribute("to", to);
	if (node != nullptr) {
		const char *iqId = XmlGetAttr(node, "id");
		if (iqId != nullptr)
			m_hXml->SetAttribute("id", iqId);
	}
}

XmlNodeIq::XmlNodeIq(CJabberIqInfo *pInfo) :
	XmlNode("iq")
{
	if (pInfo) {
		if (pInfo->GetCharIqType() != nullptr)
			m_hXml->SetAttribute("type", pInfo->GetCharIqType());
		if (pInfo->GetReceiver() != nullptr)
			m_hXml->SetAttribute("to", pInfo->GetReceiver());
		if (pInfo->GetIqId() != -1)
			XmlAddAttrID(*this, pInfo->GetIqId());
	}
}

XmlNodeIq::XmlNodeIq(const char *type, CJabberIqInfo *pInfo) :
	XmlNode("iq")
{
	if (type != nullptr)
		m_hXml->SetAttribute("type", type);
	
	if (pInfo) {
		if (pInfo->GetFrom() != nullptr)
			m_hXml->SetAttribute("to", pInfo->GetFrom());
		if (pInfo->GetIdStr() != nullptr)
			m_hXml->SetAttribute("id", pInfo->GetIdStr());
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
	if (ptszText)
		m_hXml->SetText(ptszText);
}

/////////////////////////////////////////////////////////////////////////////////////////

TiXmlElement* __fastcall operator<<(TiXmlElement *node, const XCHILDNS &child)
{
	TiXmlElement *res = XmlAddChild(node, child.name);
	res->SetAttribute("xmlns", child.ns);
	return res;
}

TiXmlElement* __fastcall operator<<(TiXmlElement *node, const XQUERY &child)
{
	TiXmlElement *res = XmlAddChild(node, "query");
	res->SetAttribute("xmlns", child.ns);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

void XmlAddAttrID(TiXmlElement *hXml, int id)
{
	hXml->SetAttribute("id", ptrA(JabberId2string(id)).get());
}
