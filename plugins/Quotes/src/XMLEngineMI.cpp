#include "StdAfx.h"

class CXMLNodeMI : public IXMLNode, private boost::noncopyable
{
public:
	typedef boost::shared_ptr<IXMLNode> TXMLNodePtr;

public:
	explicit CXMLNodeMI(HXML hXMl, bool bDestroy = false) : m_hXML(hXMl), m_bDestroy(bDestroy)
	{
		assert(m_hXML);
	}

	virtual ~CXMLNodeMI()
	{
		if (m_bDestroy)
			xmlDestroyNode(m_hXML);
	}

	virtual size_t GetChildCount()const
	{
		return xmlGetChildCount(m_hXML);
	}

	virtual TXMLNodePtr GetChildNode(size_t nIndex)const
	{
		HXML h = xmlGetChild(m_hXML, (int)nIndex);
		if (h)
			return TXMLNodePtr(new CXMLNodeMI(h));

		return TXMLNodePtr();
	}

	virtual tstring GetText()const
	{
		tstring sResult;
		LPCTSTR psz = xmlGetText(m_hXML);
		if (psz)
			sResult = psz;

		return sResult;
	}

	virtual tstring GetName()const
	{
		tstring sResult;
		LPCTSTR psz = xmlGetName(m_hXML);
		if (psz)
			sResult = psz;

		return sResult;
	}

	virtual bool AddChild(const TXMLNodePtr& pNode)
	{
		CXMLNodeMI* pXML = dynamic_cast<CXMLNodeMI*>(pNode.get());
		if (pXML) {
			xmlAddChild2(pXML->m_hXML, m_hXML);
			pXML->m_bDestroy = false;
			return true;
		}

		return false;
	}

	virtual bool AddAttribute(const tstring& rsName, const tstring& rsValue)
	{
		xmlAddAttr(m_hXML, rsName.c_str(), rsValue.c_str());
		return true;
	}

	virtual tstring GetAttributeValue(const tstring& rsAttrName)
	{
		LPCTSTR pszValue = xmlGetAttrValue(m_hXML, rsAttrName.c_str());
		return ((NULL != pszValue) ? tstring(pszValue) : tstring());
	}

	virtual void Write(tostream& o)const
	{
		safe_string<TCHAR> ss(xmlToString(m_hXML, NULL));
		if (ss.m_p)
			o << (char*)T2Utf(ss.m_p);
	}

private:
	HXML m_hXML;
	bool m_bDestroy;
};

CXMLEngineMI::CXMLEngineMI()
{
}

CXMLEngineMI::~CXMLEngineMI()
{
}

IXMLNode::TXMLNodePtr CXMLEngineMI::LoadFile(const tstring& rsFileName)const
{
	IXMLNode::TXMLNodePtr pResult;
	FILE* stream;
	if (0 == ::_tfopen_s(&stream, rsFileName.c_str(), _T("r"))) {
		struct _stat st;
		if (-1 != ::_fstat(::_fileno(stream), &st)) {
			std::vector<char> aBuffer(st.st_size + 1);
			char* pBuffer = &*(aBuffer.begin());
			size_t cBytes = ::fread(pBuffer, sizeof(char), st.st_size, stream);
			if (cBytes > 0 && cBytes <= static_cast<size_t>(st.st_size)) {
				pBuffer[cBytes] = '\0';

				int nLen = (int)cBytes;
				mir_safe_string<TCHAR> ss(mir_utf8decodeT(pBuffer));
				if (ss.m_p) {
					HXML h = xmlParseString(ss.m_p, &nLen, NULL);
					if (h)
						pResult = IXMLNode::TXMLNodePtr(new CXMLNodeMI(h, true));
				}
			}
		}
		::fclose(stream);
	}

	return pResult;
}

static IXMLNode::TXMLNodePtr create_node(const tstring& rsName, const tstring& rsText, bool bIsDecl)
{
	IXMLNode::TXMLNodePtr pResult;
	HXML h = xmlCreateNode(rsName.c_str(), rsText.c_str(), bIsDecl);
	if (h)
		pResult = IXMLNode::TXMLNodePtr(new CXMLNodeMI(h, true));

	return pResult;
}

bool CXMLEngineMI::SaveFile(const tstring& rsFileName, const IXMLNode::TXMLNodePtr& pNode)const
{
	CXMLNodeMI* pXML = dynamic_cast<CXMLNodeMI*>(pNode.get());
	if (pXML) {
		tofstream file(rsFileName.c_str());
		if (file.good()) {
			IXMLNode::TXMLNodePtr pRoot(create_node(_T("xml"), tstring(), true));
			if (pRoot) {
				pRoot->AddAttribute(_T("version"), _T("1.0"));
				pRoot->AddAttribute(_T("encoding"), _T("UTF-8"));
				file << *pRoot;
			}

			if (file.good()) {
				file << *pNode;
			}
		}

		return file.good();
	}

	return false;
}

IXMLNode::TXMLNodePtr CXMLEngineMI::CreateNode(const tstring& rsName, const tstring& rsText)const
{
	return create_node(rsName, rsText, false);
}
