#include "StdAfx.h"

using _com_util::CheckError;

namespace
{
	class CHTMLNode : public IHTMLNode
	{
	public:
		typedef CComPtr<IDispatch> TComPtr;
		typedef CComPtr<IHTMLDocument3> TDocumentPtr;

	protected:
		typedef CComPtr<IHTMLElementCollection> TElementCollectionPtr;

	public:
		CHTMLNode(const TComPtr& pElement, const TDocumentPtr& pDocument)
			: m_pElement(pElement), m_pDocument(pDocument){}

		virtual THTMLNodePtr GetElementByID(const tstring& rsID)const
		{
			if (m_pDocument)
			{
				CComPtr<IHTMLElement> pElement;
				if (SUCCEEDED(m_pDocument->getElementById(bstr_t(rsID.c_str()), &pElement))
					&& pElement)
				{
					TComPtr p(pElement);
					return THTMLNodePtr(new CHTMLNode(p, m_pDocument));
				}
			}

			return THTMLNodePtr();
		}

		virtual size_t GetChildCount()const
		{
			TElementCollectionPtr pColl = GetElementCollectionPtr();
			if (pColl)
			{
				LONG celem = 0;
				HRESULT hr = pColl->get_length(&celem);
				if (S_OK == hr)
				{
					return celem;
				}
			}

			return 0;
		}

		virtual THTMLNodePtr GetChildPtr(size_t nIndex)
		{
			TElementCollectionPtr pColl = GetElementCollectionPtr();
			if (pColl)
			{
				VARIANT varIndex;
				varIndex.vt = VT_UINT;
				varIndex.lVal = (LONG)nIndex;
				VARIANT var2;
				VariantInit(&var2);
				TComPtr pDisp;
				HRESULT hr = pColl->item(varIndex, var2, &pDisp);
				if (S_OK == hr && pDisp)
				{
					return THTMLNodePtr(new CHTMLNode(pDisp, m_pDocument));
				}
			}

			return THTMLNodePtr();
		}

		virtual bool Is(EType nType)const
		{
			switch (nType)
			{
			case Table:
			{
				CComPtr<IHTMLTable> pTable;
				return (SUCCEEDED(m_pElement->QueryInterface(IID_IHTMLTable, reinterpret_cast<void**>(&pTable))) && (pTable));
			}
			case TableRow:
			{
				CComPtr<IHTMLTableRow> pRow;
				return (SUCCEEDED(m_pElement->QueryInterface(IID_IHTMLTableRow, reinterpret_cast<void**>(&pRow))) && (pRow));
			}
			case TableColumn:
			{
				CComPtr<IHTMLTableCol> pCol;
				return (SUCCEEDED(m_pElement->QueryInterface(IID_IHTMLTableCol, reinterpret_cast<void**>(&pCol))) && (pCol));
			}
			}

			return false;
		}

		virtual tstring GetAttribute(const tstring& rsAttrName)const
		{
			USES_CONVERSION;

			tstring sAttr;
			CComPtr<IHTMLElement> pElement;
			if (SUCCEEDED(m_pElement->QueryInterface(IID_IHTMLElement, reinterpret_cast<void**>(&pElement))) && pElement)
			{
				_variant_t vAttribute;
				BSTR pbstrAttrName = T2BSTR(rsAttrName.c_str());
				if (SUCCEEDED(pElement->getAttribute(pbstrAttrName, 1, &vAttribute))
					&& VT_NULL != vAttribute.vt && VT_EMPTY != vAttribute.vt)
				{
					try
					{
						_bstr_t b(vAttribute);
						LPCTSTR psz = b;
						if (psz)
						{
							sAttr = psz;
						}
					}
					catch (_com_error&)
					{
					}
				}
			}

			return sAttr;
		}

		virtual tstring GetText()const
		{
			// 			USES_CONVERSION;

			tstring sText;
			CComPtr<IHTMLElement> pElement;
			if (SUCCEEDED(m_pElement->QueryInterface(IID_IHTMLElement, reinterpret_cast<void**>(&pElement))) && pElement)
			{
				BSTR bstrText;
				if (SUCCEEDED(pElement->get_innerText(&bstrText)) && bstrText)
				{
					try
					{
						sText = _bstr_t(bstrText);
					}
					catch (_com_error&)
					{
					}

					::SysFreeString(bstrText);
				}
			}

			return sText;
		}

	protected:
		virtual TElementCollectionPtr GetElementCollectionPtr()const
		{
			TElementCollectionPtr pColl;
			HRESULT hr = m_pElement->QueryInterface(IID_IHTMLElementCollection, reinterpret_cast<void**>(&pColl));
			if (FAILED(hr))
			{
				CComPtr<IHTMLElement> pElement;
				if (SUCCEEDED(m_pElement->QueryInterface(IID_IHTMLElement, reinterpret_cast<void**>(&pElement))) && pElement)
				{
					CComPtr<IDispatch> pDisp;
					if (SUCCEEDED(pElement->get_children(&pDisp)) && pDisp)
					{
						hr = pDisp->QueryInterface(IID_IHTMLElementCollection, reinterpret_cast<void**>(&pColl));
					}
				}
			}

			return pColl;
		}

	private:
		TComPtr m_pElement;
		TDocumentPtr m_pDocument;
	};
}

CHTMLParserMS::CHTMLParserMS() : m_bCallUninit(false)
{
	try
	{
		CheckError(::CoInitialize(NULL));

		m_bCallUninit = true;

		_com_util::CheckError(
			::CoCreateInstance(CLSID_HTMLDocument,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IHTMLDocument2,
			(LPVOID*)&m_pDoc)
			);

		CComPtr<IPersistStreamInit> pPersist;
		_com_util::CheckError(m_pDoc->QueryInterface(IID_IPersistStreamInit,
			(LPVOID*)&pPersist));

		_com_util::CheckError(pPersist->InitNew());

		_com_util::CheckError(m_pDoc->QueryInterface(IID_IMarkupServices,
			(LPVOID*)&m_pMS));

		if (m_pMS)
		{
			_com_util::CheckError(m_pMS->CreateMarkupPointer(&m_pMkStart));
			_com_util::CheckError(m_pMS->CreateMarkupPointer(&m_pMkFinish));
		}
	}
	catch (_com_error&/* e*/)
	{
		// 		show_com_error_msg(e);
	}

}

CHTMLParserMS::~CHTMLParserMS()
{
	if (true == m_bCallUninit)
	{
		::CoUninitialize();
	}
}

CHTMLParserMS::THTMLNodePtr CHTMLParserMS::ParseString(const tstring& rsHTML)
{
	USES_CONVERSION;

	try
	{
		mir_cslock lck(m_cs);

		OLECHAR* p = T2OLE(const_cast<LPTSTR>(rsHTML.c_str()));
		CComPtr<IMarkupContainer>  pMC;
		_com_util::CheckError(m_pMS->ParseString(p, 0, &pMC, m_pMkStart, m_pMkFinish));

		if (pMC)
		{
			CComPtr<IHTMLDocument2> pNewDoc;

			_com_util::CheckError(pMC->QueryInterface(IID_IHTMLDocument,
				(LPVOID*)&pNewDoc));

			if (pNewDoc)
			{
				CComPtr<IHTMLElementCollection> pColl;
				_com_util::CheckError(pNewDoc->get_all(&pColl));

				CHTMLNode::TDocumentPtr pDoc;
				pMC->QueryInterface(IID_IHTMLDocument3, (LPVOID*)&pDoc);


				return THTMLNodePtr(new CHTMLNode(CHTMLNode::TComPtr(pColl), pDoc));
			}
		}
	}
	catch (_com_error&/* e*/)
	{
		// 		show_com_error_msg(e);
	}

	return THTMLNodePtr();
}

bool CHTMLParserMS::IsInstalled()
{
	bool bResult = true;
	bool bCallUninit = false;
	try
	{
		CheckError(::CoInitialize(NULL));

		bCallUninit = true;

		CComPtr<IHTMLDocument2> pDoc;
		_com_util::CheckError(
			::CoCreateInstance(CLSID_HTMLDocument,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IHTMLDocument2,
			reinterpret_cast<LPVOID*>(&pDoc))
			);
	}
	catch (_com_error&/* e*/)
	{
		bResult = false;
	}

	if (bCallUninit)
	{
		::CoUninitialize();
	}

	return bResult;
}

CHTMLEngineMS::CHTMLEngineMS()
{

}

CHTMLEngineMS::~CHTMLEngineMS()
{

}

CHTMLEngineMS::THTMLParserPtr CHTMLEngineMS::GetParserPtr()const
{
	return THTMLParserPtr(new CHTMLParserMS);
}
