#include "StdAfx.h"

XML_API xi;

namespace
{
	class CXMLNodeMI : public IXMLNode,
		private boost::noncopyable
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
			{
				xi.destroyNode(m_hXML);
			}
		}

		virtual size_t GetChildCount()const
		{
			return xi.getChildCount(m_hXML);
		}

		virtual TXMLNodePtr GetChildNode(size_t nIndex)const
		{
			HXML h = xi.getChild(m_hXML, (int)nIndex);
			if (h)
			{
				return TXMLNodePtr(new CXMLNodeMI(h));
			}
			else
			{
				return TXMLNodePtr();
			}
		}

		virtual tstring GetText()const
		{
			tstring sResult;
			LPCTSTR psz = xi.getText(m_hXML);
			if (psz)
			{
				sResult = psz;
			}

			return sResult;
		}

		virtual tstring GetName()const
		{
			tstring sResult;
			LPCTSTR psz = xi.getName(m_hXML);
			if (psz)
			{
				sResult = psz;
			}

			return sResult;
		}

		virtual bool AddChild(const TXMLNodePtr& pNode)
		{
			CXMLNodeMI* pXML = dynamic_cast<CXMLNodeMI*>(pNode.get());
			if (pXML)
			{
				xi.addChild2(pXML->m_hXML, m_hXML);
				pXML->m_bDestroy = false;
				return true;
			}
			else
			{
				return false;
			}
		}

		virtual bool AddAttribute(const tstring& rsName, const tstring& rsValue)
		{
			xi.addAttr(m_hXML, rsName.c_str(), rsValue.c_str());
			return true;
		}

		virtual tstring GetAttributeValue(const tstring& rsAttrName)
		{
			LPCTSTR pszValue = xi.getAttrValue(m_hXML, rsAttrName.c_str());
			return ((NULL != pszValue) ? tstring(pszValue) : tstring());
		}

		virtual void Write(tostream& o)const
		{
			// 			struct safe_string
			// 			{				
			// 				safe_string(LPTSTR p):m_p(p){}
			// 				~safe_string(){xi.freeMem(m_p);}
			// 
			// 				LPTSTR m_p;
			// 			};
			// 
			// 			struct mir_safe_string
			// 			{
			// 				mir_safe_string(LPSTR p) : m_p(p){}
			// 				~mir_safe_string(){mir_free(m_p);}
			// 
			// 				LPSTR m_p;
			// 			};


			safe_string<TCHAR> ss(xi.toString(m_hXML, NULL));
			if (ss.m_p)
			{
				mir_safe_string<char> mss(mir_utf8encodeT(ss.m_p));
				if (mss.m_p)
				{
					o << mss.m_p;
				}
			}
		}

	private:
		HXML m_hXML;
		bool m_bDestroy;
	};
}

CXMLEngineMI::CXMLEngineMI()
{
}

CXMLEngineMI::~CXMLEngineMI()
{
}

IXMLNode::TXMLNodePtr CXMLEngineMI::LoadFile(const tstring& rsFileName)const
{
	// 	struct mir_safe_string
	// 	{
	// 		mir_safe_string(LPTSTR p) : m_p(p){}
	// 		~mir_safe_string(){mir_free(m_p);}
	// 
	// 		LPTSTR m_p;
	// 	};


	IXMLNode::TXMLNodePtr pResult;
	FILE* stream;
	if (0 == ::_tfopen_s(&stream, rsFileName.c_str(), _T("r")))
	{
		struct _stat st;
		if (-1 != ::_fstat(::_fileno(stream), &st))
		{
			std::vector<char> aBuffer(st.st_size + 1);
			char* pBuffer = &*(aBuffer.begin());
			size_t cBytes = ::fread(pBuffer, sizeof(char), st.st_size, stream);
			if (cBytes > 0 && cBytes <= static_cast<size_t>(st.st_size))
			{
				pBuffer[cBytes] = '\0';

				int nLen = (int)cBytes;
				mir_safe_string<TCHAR> ss(mir_utf8decodeT(pBuffer));
				if (ss.m_p)
				{
					HXML h = xi.parseString(ss.m_p, &nLen, NULL);
					if (h)
					{
						pResult = IXMLNode::TXMLNodePtr(new CXMLNodeMI(h, true));
					}
				}
			}
		}
		::fclose(stream);
	}

	return pResult;
}

namespace
{
	IXMLNode::TXMLNodePtr create_node(const tstring& rsName, const tstring& rsText, bool bIsDecl)
	{
		IXMLNode::TXMLNodePtr pResult;
		HXML h = xi.createNode(rsName.c_str(), rsText.c_str(), bIsDecl);
		if (h)
		{
			pResult = IXMLNode::TXMLNodePtr(new CXMLNodeMI(h, true));
		}

		return pResult;
	}
}

bool CXMLEngineMI::SaveFile(const tstring& rsFileName, const IXMLNode::TXMLNodePtr& pNode)const
{
	CXMLNodeMI* pXML = dynamic_cast<CXMLNodeMI*>(pNode.get());
	if (pXML)
	{
		tofstream file(rsFileName.c_str());
		if (file.good())
		{
			IXMLNode::TXMLNodePtr pRoot(create_node(_T("xml"), tstring(), true));
			if (pRoot)
			{
				pRoot->AddAttribute(_T("version"), _T("1.0"));
				pRoot->AddAttribute(_T("encoding"), _T("UTF-8"));
				file << *pRoot;
			}

			if (file.good())
			{
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