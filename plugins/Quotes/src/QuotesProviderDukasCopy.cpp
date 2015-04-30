#include "StdAfx.h"

CQuotesProviderDukasCopy::CQuotesProviderDukasCopy()
{
}

CQuotesProviderDukasCopy::~CQuotesProviderDukasCopy()
{
}

namespace
{
	inline tstring get_quote_id(MCONTACT hContact)
	{
		return Quotes_DBGetStringT(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_ID);
	}

	inline bool is_quote_id_equal(MCONTACT hContact, const tstring& sID)
	{
		return sID == get_quote_id(hContact);
	}
}

bool CQuotesProviderDukasCopy::IsQuoteWatched(const CQuote& rQuote)const
{
	return m_aContacts.end() != std::find_if(m_aContacts.begin(), m_aContacts.end(),
		boost::bind(is_quote_id_equal, _1, rQuote.GetID()));
}

bool CQuotesProviderDukasCopy::WatchForQuote(const CQuote& rQuote, bool bWatch)
{
	const tstring& sQuoteID = rQuote.GetID();
	TContracts::iterator i =
		std::find_if(m_aContacts.begin(), m_aContacts.end(),
		boost::bind(is_quote_id_equal, _1, sQuoteID));

	if ((false == bWatch) && (i != m_aContacts.end()))
	{
		MCONTACT hContact = *i;
		{// for CCritSection
			mir_cslock lck(m_cs);
			m_aContacts.erase(i);
		}

		CallService(MS_DB_CONTACT_DELETE, WPARAM(hContact), 0);
		return true;
	}
	else if ((true == bWatch) && (i == m_aContacts.end()))
	{
		MCONTACT hContact = CreateNewContact(rQuote.GetSymbol());
		if (hContact)
		{
			db_set_ts(hContact, QUOTES_PROTOCOL_NAME, DB_STR_QUOTE_ID, sQuoteID.c_str());
			if (false == rQuote.GetName().empty())
			{
				db_set_ts(hContact, QUOTES_PROTOCOL_NAME, DB_STR_QUOTE_DESCRIPTION, rQuote.GetName().c_str());
			}

			return true;
		}

	}
	return false;
}

tstring CQuotesProviderDukasCopy::BuildHTTPURL()const
{
	tostringstream sURL;
	sURL << GetURL();

	{
		mir_cslock lck(m_cs);
		for (TContracts::const_iterator i = m_aContacts.begin(); i != m_aContacts.end(); ++i)
		{
			MCONTACT hContact = *i;
			tstring sID = get_quote_id(hContact);
			if (false == sID.empty())
			{
				sURL << sID << _T(",");
			}
		}
	}

	return sURL.str();
}

namespace
{
	struct CEconomicRateInfo
	{
		CEconomicRateInfo() : m_dCurRate(0.0), m_dPrevRate(0.0) {}
		tstring m_sName;
		double m_dCurRate;
		double m_dPrevRate;
		tstring m_sID;
	};

	typedef std::vector<CEconomicRateInfo> TEconomicRates;
	typedef IHTMLNode::THTMLNodePtr THTMLNodePtr;

	bool string2double(const TCHAR* pszText, double& rValue)
	{
		assert(pszText);
		try
		{
			rValue = boost::lexical_cast<double>(pszText);
		}
		catch (boost::bad_lexical_cast &)
		{
			return false;
		}

		return true;
	}


	void parse_row(const THTMLNodePtr& pRow, TEconomicRates& raRates)
	{
		CEconomicRateInfo ri;
		ri.m_sID = pRow->GetAttribute(_T("sid"));
		if (false == ri.m_sID.empty())
		{
			unsigned short cColsHandled = 0;
			tostringstream sSidID;
			sSidID << _T("id") << ri.m_sID;
			size_t cNodes = pRow->GetChildCount();
			for (size_t i = 0; i < cNodes && cColsHandled < 2; ++i)
			{
				THTMLNodePtr pCol = pRow->GetChildPtr(i);
				if (pCol)
				{
					tstring sColID = pCol->GetAttribute(_T("id"));
					if (false == sColID.empty())
					{
						if (0 == quotes_stricmp(sColID.c_str(), _T("stock")))
						{
							ri.m_sName = pCol->GetText();
							if (false == ri.m_sName.empty())
							{
								++cColsHandled;
							}
						}
						else if (0 == quotes_stricmp(sSidID.str().c_str(), sColID.c_str()))
						{
							tstring sRate = pCol->GetText();
							if ((false == sRate.empty()) && (true == string2double(sRate.c_str(), ri.m_dCurRate)))
							{
								ri.m_dPrevRate = ri.m_dCurRate;
								++cColsHandled;
								tstring sOldRate = pCol->GetAttribute(_T("oldPrice"));
								if (false == sOldRate.empty())
								{
									string2double(sOldRate.c_str(), ri.m_dPrevRate);
								}
							}
						}
					}
				}
			}

			if (2 == cColsHandled)
			{
				raRates.push_back(ri);
			}
		}
	}

	// 	void parse_table(const THTMLNodePtr& pNode,TEconomicRates& raRates)
	// 	{
	// 		size_t cNodes = pNode->GetChildCount();
	// 		for(size_t i = 0;i < cNodes;++i)
	// 		{
	// 			THTMLNodePtr pChild = pNode->GetChildPtr(i);
	// 			if(pChild && pChild->Is(IHTMLNode::TableRow))
	// 			{
	// 				parse_row(pChild,raRates);
	// 			}
	// 		}
	// 
	// 	}

	void parser_html_node(const THTMLNodePtr& pNode, TEconomicRates& raRates)
	{
		size_t cNodes = pNode->GetChildCount();
		for (size_t i = 0; i < cNodes; ++i)
		{
			THTMLNodePtr pChild = pNode->GetChildPtr(i);
			if (pChild && pChild->Is(IHTMLNode::TableRow))
			{
				parse_row(pChild, raRates);
			}
		}
	}

	bool parse_HTML(const tstring& sHTML, TEconomicRates& raRates)
	{
		IHTMLEngine::THTMLParserPtr pHTMLParser = CModuleInfo::GetHTMLEngine()->GetParserPtr();
		THTMLNodePtr pRoot = pHTMLParser->ParseString(sHTML);
		if (pRoot)
		{
			parser_html_node(pRoot, raRates);
			return true;
		}
		else
		{
			return false;
		}
	}
}


void CQuotesProviderDukasCopy::RefreshQuotes(TContracts& anContacts)
{
	if (CModuleInfo::GetInstance().GetExtendedStatusFlag())
	{
		std::for_each(anContacts.begin(), anContacts.end(),
			boost::bind(SetContactStatus, _1, ID_STATUS_OCCUPIED));
	}

	tstring sURL = BuildHTTPURL();

	CHTTPSession http;
	// 	LogIt(Info,sURL);
	if (true == http.OpenURL(sURL))
	{
		// 		if(true == IsOnline())
		{
			tstring sHTML;
			if (true == http.ReadResponce(sHTML))
			{
				// 				LogIt(Info,sHTML);
				// 				if(true == IsOnline())
				{
					TEconomicRates aRates;
					if (true == parse_HTML(sHTML, aRates)
						&& (true == IsOnline()))
					{
						for (TEconomicRates::const_iterator it = aRates.begin(); (it != aRates.end()) && (true == IsOnline()); ++it)
						{
							const CEconomicRateInfo& ri = *it;

							TContracts::iterator i = std::find_if(anContacts.begin(), anContacts.end(),
								boost::bind(is_quote_id_equal, _1, ri.m_sID));
							if (i != anContacts.end() && (true == IsOnline()))
							{
								MCONTACT hContact = *i;
								anContacts.erase(i);

								WriteContactRate(hContact, ri.m_dCurRate, ri.m_sName);
							}
						}
					}
				}
			}
		}
	}

	std::for_each(anContacts.begin(), anContacts.end(),
		boost::bind(&SetContactStatus, _1, ID_STATUS_NA));
}

void CQuotesProviderDukasCopy::ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp)
{
	ShowDukasCopyPropPage(this, wp, odp);
}

void CQuotesProviderDukasCopy::Accept(CQuotesProviderVisitor& visitor)const
{
	CQuotesProviderBase::Accept(visitor);
	visitor.Visit(*this);
}

MCONTACT CQuotesProviderDukasCopy::GetContactByQuoteID(const tstring& rsQuoteID)const
{
	mir_cslock lck(m_cs);

	TContracts::const_iterator i = std::find_if(m_aContacts.begin(), m_aContacts.end(),
		boost::bind(std::equal_to<tstring>(), rsQuoteID, boost::bind(get_quote_id, _1)));
	if (i != m_aContacts.end())
		return *i;

	return NULL;
}
