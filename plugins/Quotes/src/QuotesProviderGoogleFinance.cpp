#include "StdAfx.h"

CQuotesProviderGoogleFinance::CQuotesProviderGoogleFinance()
{
}

CQuotesProviderGoogleFinance::~CQuotesProviderGoogleFinance()
{
}

namespace
{
	tstring build_url(MCONTACT hContact, const tstring& rsURL)
	{
		tostringstream o;
		o << rsURL << _T("?q=") << Quotes_DBGetStringT(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_ID);
		return o.str();
	}

	struct CGoogleInfo
	{
		enum
		{
			giRate = 0x0001,
			giOpen = 0x0002,
			giPercentChangeAfterHours = 0x0004,
			giPercentChangeToYesterdayClose = 0x0008
		};
		CGoogleInfo()
			: m_dRate(0.0), m_dOpenValue(0.0), m_dPercentChangeAfterHours(0.0), m_dPercentChangeToYersterdayClose(0.0), m_nFlags(0){}
		// 		tstring m_sCmpID;
		tstring m_sCmpName;
		double m_dRate;
		double m_dOpenValue;
		double m_dPercentChangeAfterHours;
		double m_dPercentChangeToYersterdayClose;

		// 		tstring m_sRateID;
		// 		tstring m_sDiffID;
		byte m_nFlags;
	};

	tstring make_rate_id_value(const tstring& rsCmpID, int nFlags)
	{
		tostringstream o;
		o << _T("ref_") << rsCmpID;
		switch (nFlags)
		{
		default:
			assert(!"Unknown type of value");
		case CGoogleInfo::giRate:
			o << _T("_l");
			break;
		case CGoogleInfo::giPercentChangeAfterHours:
			o << _T("_ecp");
			break;
		case CGoogleInfo::giPercentChangeToYesterdayClose:
			o << _T("_cp");
			break;
		}

		return o.str();
	}

	tstring get_var_value(const tstring& rsHTML, LPCTSTR pszVarName, size_t cVarNameLength)
	{
		tstring sResult;
		tstring::size_type n = rsHTML.find(pszVarName);
		if (tstring::npos != n)
		{
			size_t cLengthHTML = rsHTML.size();
			for (size_t i = n + cVarNameLength; i < cLengthHTML; ++i)
			{
				TCHAR c = rsHTML[i];
				if (_T(';') == c)
				{
					break;
				}
				else
				{
					sResult.push_back(c);
				}
			}
		}

		return sResult;
	}

	tstring get_company_id(const tstring& rsHTML)
	{
		static LPCTSTR pszVarName = _T("setCompanyId(");
		static size_t cVarNameLength = mir_tstrlen(pszVarName);

		tstring sResult;
		tstring::size_type n = rsHTML.find(pszVarName);
		if (tstring::npos != n)
		{
			size_t cLengthHTML = rsHTML.size();
			for (size_t i = n + cVarNameLength; i < cLengthHTML; ++i)
			{
				TCHAR c = rsHTML[i];
				if (_T(')') == c)
				{
					break;
				}
				else
				{
					sResult.push_back(c);
				}
			}
		}
		return sResult;
		// 		return get_var_value(rsHTML,pszVarName,cVarNameLength);		
	}

	tstring get_company_name(const tstring& rsHTML)
	{
		static LPCTSTR pszVarName = _T("var _companyName = ");
		static size_t cVarNameLength = mir_tstrlen(pszVarName);

		tstring s = get_var_value(rsHTML, pszVarName, cVarNameLength);
		if (s.size() > 0 && _T('\'') == s[0])
		{
			s.erase(s.begin());
		}

		if (s.size() > 0 && _T('\'') == s[s.size() - 1])
		{
			s.erase(s.rbegin().base() - 1);
		}

		return s;
	}

	bool get_double_value(const tstring& rsText, double& rdValue)
	{
		tistringstream input(rsText);
		input.imbue(std::locale("English_United States.1252"));
		input >> rdValue;

		if ((true == input.bad()) || (true == input.fail()))
		{
			tistringstream inputSys(rsText);
			input.imbue(GetSystemLocale());
			input >> rdValue;
			return (false == inputSys.bad()) && (false == inputSys.fail());
		}
		else
		{
			return true;
		}
	}

	bool get_rate(const IHTMLNode::THTMLNodePtr& pRate, CGoogleInfo& rInfo)
	{
		tstring sRate = pRate->GetText();

		if (true == get_double_value(sRate, rInfo.m_dRate))
		{
			rInfo.m_nFlags |= CGoogleInfo::giRate;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool get_inline_data(const IHTMLNode::THTMLNodePtr& pNode, CGoogleInfo& rInfo)
	{
		size_t cChild = pNode->GetChildCount();
		for (size_t i = 0; i < cChild; ++i)
		{
			IHTMLNode::THTMLNodePtr pChild = pNode->GetChildPtr(i);
			size_t c = pChild->GetChildCount();
			assert(2 == c);
			if (c >= 2)
			{
				IHTMLNode::THTMLNodePtr pName = pChild->GetChildPtr(0);

				tstring sName = pName->GetText();
				if (0 == quotes_stricmp(sName.c_str(), _T("Open")))
				{
					IHTMLNode::THTMLNodePtr pValue = pChild->GetChildPtr(1);
					tstring sValue = pValue->GetText();
					if (true == get_double_value(sValue, rInfo.m_dOpenValue))
					{
						rInfo.m_nFlags |= CGoogleInfo::giOpen;
					}
					return true;
				}
			}
		}

		return false;
	}

	bool get_dif_value(const IHTMLNode::THTMLNodePtr& pNode, CGoogleInfo& rInfo, int nItem)
	{
		tstring sDiff = pNode->GetText();
		// this value is in brackets and it has percentage sign. 
		// Remove these symbols.
		for (tstring::iterator i = sDiff.begin(); i != sDiff.end();)
		{
			TCHAR s = *i;
			if (_T('(') == s || _T(')') == s || _T('%') == s)
			{
				i = sDiff.erase(i);
			}
			else
			{
				++i;
			}
		}

		double* pValue = NULL;
		switch (nItem)
		{
		case CGoogleInfo::giPercentChangeAfterHours:
			pValue = &rInfo.m_dPercentChangeAfterHours;
			break;
		case CGoogleInfo::giPercentChangeToYesterdayClose:
			pValue = &rInfo.m_dPercentChangeToYersterdayClose;
			break;
		}

		assert(pValue);

		if ((pValue) && (true == get_double_value(sDiff, *pValue)))
		{
			rInfo.m_nFlags |= nItem;
			return true;
		}
		else
		{
			return false;
		}

	}

	bool parse_responce(const tstring& rsHTML, CGoogleInfo& rInfo)
	{
		IHTMLEngine::THTMLParserPtr pHTMLParser = CModuleInfo::GetHTMLEngine()->GetParserPtr();
		IHTMLNode::THTMLNodePtr pRoot = pHTMLParser->ParseString(rsHTML);
		if (pRoot)
		{
			tstring sCmpID = get_company_id(rsHTML);
			if (false == sCmpID.empty())
			{
				tstring sRateID = make_rate_id_value(sCmpID, CGoogleInfo::giRate);
				IHTMLNode::THTMLNodePtr pRate = pRoot->GetElementByID(sRateID);
				if (pRate && get_rate(pRate, rInfo))
				{
					rInfo.m_sCmpName = get_company_name(rsHTML);

					IHTMLNode::THTMLNodePtr pInline = pRoot->GetElementByID(_T("snap-data"));
					if (pInline)
					{
						get_inline_data(pInline, rInfo);
					}

					tstring sDiffID = make_rate_id_value(sCmpID, CGoogleInfo::giPercentChangeAfterHours);
					IHTMLNode::THTMLNodePtr pDiff = pRoot->GetElementByID(sDiffID);
					if (pDiff)
					{
						get_dif_value(pDiff, rInfo, CGoogleInfo::giPercentChangeAfterHours);
					}

					sDiffID = make_rate_id_value(sCmpID, CGoogleInfo::giPercentChangeToYesterdayClose);
					pDiff = pRoot->GetElementByID(sDiffID);
					if (pDiff)
					{
						get_dif_value(pDiff, rInfo, CGoogleInfo::giPercentChangeToYesterdayClose);
					}

					return true;
				}

				//return (true == parse_html_node(pRoot,rInfo));
			}
		}

		return false;
	}
}

void CQuotesProviderGoogleFinance::RefreshQuotes(TContracts& anContacts)
{
	CHTTPSession http;
	tstring sURL = GetURL();
	bool bUseExtendedStatus = CModuleInfo::GetInstance().GetExtendedStatusFlag();

	for (TContracts::const_iterator i = anContacts.begin(); i != anContacts.end() && IsOnline(); ++i)
	{
		MCONTACT hContact = *i;

		if (bUseExtendedStatus)
		{
			SetContactStatus(hContact, ID_STATUS_OCCUPIED);
		}

		tstring sFullURL = build_url(hContact, sURL);
		// 		LogIt(Info,sFullURL);
		if ((true == http.OpenURL(sFullURL)) && (true == IsOnline()))
		{
			tstring sHTML;
			if ((true == http.ReadResponce(sHTML)) && (true == IsOnline()))
			{
				// 				LogIt(Info,sHTML);

				CGoogleInfo Info;
				parse_responce(sHTML, Info);
				if (true == IsOnline())
				{
					if (Info.m_nFlags&CGoogleInfo::giRate)
					{
						if (Info.m_nFlags&CGoogleInfo::giOpen)
						{
							Quotes_DBWriteDouble(hContact, QUOTES_MODULE_NAME, DB_STR_GOOGLE_FINANCE_OPEN_VALUE, Info.m_dOpenValue);
						}
						if (Info.m_nFlags&CGoogleInfo::giPercentChangeAfterHours)
						{
							Quotes_DBWriteDouble(hContact, QUOTES_MODULE_NAME, DB_STR_GOOGLE_FINANCE_DIFF, Info.m_dPercentChangeAfterHours);
						}
						if (Info.m_nFlags&CGoogleInfo::giPercentChangeToYesterdayClose)
						{
							Quotes_DBWriteDouble(hContact, QUOTES_MODULE_NAME, DB_STR_GOOGLE_FINANCE_PERCENT_CHANGE_TO_YERSTERDAY_CLOSE, Info.m_dPercentChangeToYersterdayClose);
						}
						if (false == Info.m_sCmpName.empty())
						{
							db_set_ts(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION, Info.m_sCmpName.c_str());
						}

						WriteContactRate(hContact, Info.m_dRate);
						continue;
					}
				}
			}
		}

		SetContactStatus(hContact, ID_STATUS_NA);
	}

}


void CQuotesProviderGoogleFinance::Accept(CQuotesProviderVisitor& visitor)const
{
	CQuotesProviderFinance::Accept(visitor);
	visitor.Visit(*this);
}

