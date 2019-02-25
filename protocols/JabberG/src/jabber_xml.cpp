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
	if (type != nullptr) XmlAddAttr(*this, "type", type);
	if (to   != nullptr) XmlAddAttr(*this, "to",   to);
	if (id   != -1  )    XmlAddAttrID(*this, id);
}

XmlNodeIq::XmlNodeIq(const char *type, const char *idStr, const char *to) :
	XmlNode("iq")
{
	if (type  != nullptr) XmlAddAttr(*this, "type", type );
	if (to    != nullptr) XmlAddAttr(*this, "to",   to   );
	if (idStr != nullptr) XmlAddAttr(*this, "id",   idStr);
}

XmlNodeIq::XmlNodeIq(const char *type, TiXmlElement *node, const char *to) :
	XmlNode("iq")
{
	if (type  != nullptr) XmlAddAttr(*this, "type", type );
	if (to    != nullptr) XmlAddAttr(*this, "to",   to   );
	if (node  != nullptr) {
		const char *iqId = node->Attribute("id");
		if (iqId != nullptr)
			XmlAddAttr(*this, "id", iqId);
	}
}

XmlNodeIq::XmlNodeIq(CJabberIqInfo *pInfo) :
	XmlNode("iq")
{
	if (pInfo) {
		if (pInfo->GetCharIqType() != nullptr) XmlAddAttr(*this, "type", pInfo->GetCharIqType());
		if (pInfo->GetReceiver()   != nullptr) XmlAddAttr(*this, "to", pInfo->GetReceiver());
		if (pInfo->GetIqId()       != -1)      XmlAddAttrID(*this, pInfo->GetIqId());
	}
}

XmlNodeIq::XmlNodeIq(const char *type, CJabberIqInfo *pInfo) :
	XmlNode("iq")
{
	if (type != nullptr) XmlAddAttr(*this, "type", type);
	if (pInfo) {
		if (pInfo->GetFrom()  != nullptr) XmlAddAttr(*this, "to", pInfo->GetFrom());
		if (pInfo->GetIdStr() != nullptr) XmlAddAttr(*this, "id", pInfo->GetIdStr());
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
	for (auto *pChild : TiXmlFilter(hXml, key))
		if (pChild->Attribute(attrName, attrValue))
			return pChild;

	return nullptr;
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
