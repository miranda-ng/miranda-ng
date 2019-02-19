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

#ifndef _JABBER_XML_H_
#define _JABBER_XML_H_

int XmlGetChildCount(const TiXmlElement*);

TiXmlElement* XmlAddChild(TiXmlElement*, const char *pszName);
TiXmlElement* XmlAddChild(TiXmlElement*, const char *pszName, const char *ptszValue);
TiXmlElement* XmlAddChild(TiXmlElement*, const char *pszName, int iValue);

int XmlGetChildInt(const TiXmlElement *hXml, const char *key);
const char* XmlGetChildText(const TiXmlElement *hXml, const char *key);
const TiXmlElement* XmlGetChildByTag(const TiXmlElement *hXml, const char *key, const char *attrName, const char *attrValue);

void XmlAddAttr(TiXmlElement*, const char *pszName, const char *ptszValue);
void XmlAddAttrID(TiXmlElement*, int id);

class XmlNode : public TiXmlDocument, private MNonCopyable
{
	TiXmlElement *m_hXml;

public:
	XmlNode(const char *name);
	XmlNode(const char *pszName, const char *ptszText);

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
	node->SetAttribute(attr.name, attr.value);
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
	XmlAddAttrID(node, attr.id);
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

__forceinline TiXmlElement *operator<<(TiXmlElement *node, const XCHILD& child)
{
	return XmlAddChild(node, child.name, child.value);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct XCHILDNS
{
	const char *name, *ns;

	__forceinline XCHILDNS(const char *_name, const char *_ns = nullptr) :
		name(_name),
		ns(_ns)
		{}
};

TiXmlElement *__fastcall operator<<(TiXmlElement *node, const XCHILDNS& child);

/////////////////////////////////////////////////////////////////////////////////////////

struct XQUERY
{
	const char *ns;

	__forceinline XQUERY(const char *_ns) :
		ns(_ns)
		{}
};

TiXmlElement *__fastcall operator<<(TiXmlElement *node, const XQUERY& child);

/////////////////////////////////////////////////////////////////////////////////////////
// Limited XPath support
//     path should look like: "node-spec/node-spec/.../result-spec"
//     where "node-spec" can be "node-name", "node-name[@attr-name='attr-value']" or "node-name[node-index]"
//     result may be either "node-spec", or "@attr-name"
//
// Samples:
//    const char *s = XPath(node, "child/subchild[@attr='value']");          // get node text
//    XPath(node, "child/subchild[@name='test']/@attr") = L"Hello";   // create path if needed and set attribute value
//
//    XPath(node, "child/subchild[@name='test']") = L"Hello";         // TODO: create path if needed and set node text

class XPath
{
public:
	__forceinline XPath(const TiXmlElement *hXml, char *path):
		m_type(T_UNKNOWN),
		m_szPath(path),
		m_hXml(hXml),
		m_szParam(nullptr)
	{}

	// Read data
	operator const TiXmlElement*()
	{
		switch (Lookup())
		{
			case T_NODE:    return m_hXml;
			case T_NODESET: return m_hXml->FirstChildElement(m_szParam);
		}
		return nullptr;
	}
	operator LPCSTR()
	{
		switch (Lookup())
		{
			case T_ATTRIBUTE: return m_hXml->Attribute(m_szParam);
			case T_NODE:      return m_hXml->GetText();
			case T_NODESET:   return (m_hXml->FirstChildElement()) ? m_hXml->FirstChildElement()->GetText() : 0;
		}
		return nullptr;
	}
	__forceinline bool operator== (char *str)
	{
		return !mir_strcmp(*this, str);
	}
	__forceinline bool operator!= (char *str)
	{
		return mir_strcmp(*this, str) ? true : false;
	}

private:
	enum PathType
	{
		T_UNKNOWN,
		T_ERROR,
		T_NODE,
		T_ATTRIBUTE,
		T_NODESET
	};

	__forceinline PathType Lookup()
	{
		return (m_type == T_UNKNOWN) ? LookupImpl() : m_type;
	}

	enum LookupState
	{
		S_START,
		S_ATTR_STEP,
		S_NODE_NAME,
		S_NODE_OPENBRACKET,
		S_NODE_ATTRNAME,
		S_NODE_ATTREQUALS,
		S_NODE_ATTRVALUE,
		S_NODE_ATTRCLOSEVALUE,
		S_NODE_CLOSEBRACKET,

		S_FINAL,
		S_FINAL_ERROR,
		S_FINAL_ATTR,
		S_FINAL_NODESET,
		S_FINAL_NODE
	};

	struct LookupString
	{
		void Begin(const char *p_) { p = p_; }
		void End(const char *p_) { length = p_ - p; }
		operator bool() { return p ? true : false; }

		const char *p;
		int length;

	};

	struct LookupInfo
	{
		void Reset() { memset(this, 0, sizeof(*this)); }
		LookupString nodeName;
		LookupString attrName;
		LookupString attrValue;
	};

	void ProcessPath(LookupInfo &info);
	PathType LookupImpl();

	PathType m_type;
	const TiXmlElement *m_hXml;
	const char *m_szPath;
	const char *m_szParam;
};

class XPathFmt: public XPath
{
public:
	enum { BUFSIZE = 512 };
	XPathFmt(const TiXmlElement *hXml, char *path, ...): XPath(hXml, m_buf)
	{
		*m_buf = 0;
		char buf[BUFSIZE];

		va_list args;
		va_start(args, path);
		mir_vsnprintf(buf, BUFSIZE, path, args);
		buf[BUFSIZE-1] = 0;
		va_end(args);
	}

private:
	char m_buf[BUFSIZE];
};

#endif
