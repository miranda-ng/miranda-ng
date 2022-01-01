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

#ifndef _JABBER_XML_H_
#define _JABBER_XML_H_

#define JCPF_IN    0x01UL
#define JCPF_OUT   0x02UL
#define JCPF_ERROR 0x04UL

void XmlAddAttrID(TiXmlElement*, int id);

class XmlNodeHash : public tinyxml2::XMLVisitor
{
	mir_md5_state_t state;

	void add(const char *);

public:
	XmlNodeHash();

	CMStringA getResult();

	virtual bool VisitEnter(const TiXmlElement &, const tinyxml2::XMLAttribute *);
};

/////////////////////////////////////////////////////////////////////////////////////////

class XmlNode : public TiXmlDocument, private MNonCopyable
{
protected:
	TiXmlElement *m_hXml;

public:
	XmlNode(const char *name);
	XmlNode(const char *pszName, const char *ptszText);

	__forceinline void operator +=(const TiXmlElement *pNode)
	{
		m_hXml->InsertEndChild(pNode->DeepClone(this)->ToElement());
	}

	__forceinline TiXmlElement* node() const
	{	return m_hXml;
	}

	__forceinline operator TiXmlElement*()
	{	return m_hXml;
	}
};

class CJabberIqInfo;

struct XmlNodeIq : public XmlNode
{
	XmlNodeIq(const char *type, int id = -1, const char *to = nullptr);
	XmlNodeIq(const char *type, const char *idStr, const char *to);
	XmlNodeIq(const char *type, TiXmlElement *node, const char *to);
	// new request
	XmlNodeIq(CJabberIqInfo *pInfo);
	// answer to request
	XmlNodeIq(const char *type, CJabberIqInfo *pInfo);
};

typedef void (*JABBER_XML_CALLBACK)(TiXmlElement*, void*);

/////////////////////////////////////////////////////////////////////////////////////////

struct XATTR
{
	const char *name, *value;

	__forceinline XATTR(const char *_name, const char *_value) :
		name(_name),
		value(_value)
		{}
};

__forceinline TiXmlElement *operator<<(TiXmlElement *node, const XATTR& attr)
{
	XmlAddAttr(node, attr.name, attr.value);
	return node;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct XATTRI
{
	const char *name;
	int value;

	__forceinline XATTRI(const char *_name, int _value) :
		name(_name),
		value(_value)
		{}
};

__forceinline TiXmlElement *operator<<(TiXmlElement *node, const XATTRI& attr)
{
	node->SetAttribute(attr.name, attr.value);
	return node;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct XATTRI64
{
	const char *name;
	__int64 value;

	__forceinline XATTRI64(const char *_name, __int64 _value) :
		name(_name),
		value(_value)
		{}
};

__forceinline TiXmlElement *operator<<(TiXmlElement *node, const XATTRI64& attr)
{
	node->SetAttribute(attr.name, attr.value);
	return node;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct XATTRID
{
	int id;

	__forceinline XATTRID(int _value) :
		id(_value)
		{}
};

__forceinline TiXmlElement *operator<<(TiXmlElement *node, const XATTRID& attr)
{
	node->SetAttribute("id", attr.id);
	return node;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct XCHILD
{
	const char *name, *value;

	__forceinline XCHILD(const char *_name, const char *_value = nullptr) :
		name(_name),
		value(_value)
		{}
};

__forceinline TiXmlElement *operator<<(TiXmlElement *node, const XCHILD &child)
{
	return XmlAddChildA(node, child.name, child.value);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct XCHILDNS
{
	const char *name, *ns;

	__forceinline XCHILDNS(const char *_name, const char *_ns) :
		name(_name),
		ns(_ns)
		{}
};

TiXmlElement* __fastcall operator<<(TiXmlElement *node, const XCHILDNS &child);

/////////////////////////////////////////////////////////////////////////////////////////

struct XQUERY
{
	const char *ns;

	__forceinline XQUERY(const char *_ns) :
		ns(_ns)
		{}
};

TiXmlElement* __fastcall operator<<(TiXmlElement *node, const XQUERY& child);

#endif
