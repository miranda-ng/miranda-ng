#include "stdafx.h"

namespace
{
	void remove_quotes(tstring& s)
	{
		if (*s.begin() == _T('"'))
		{
			s.erase(s.begin());
		}
		if (*s.rbegin() == _T('"'))
		{
			tstring::iterator i(s.begin());
			std::advance(i, s.size() - 1);
			s.erase(i);
		}
	}

	void remove_end_of_line(tstring& s)
	{
		if (*s.rbegin() == _T('\n'))
		{
			tstring::iterator i(s.begin());
			std::advance(i, s.size() - 1);
			s.erase(i);
		}
		if (*s.rbegin() == _T('\r'))
		{
			tstring::iterator i(s.begin());
			std::advance(i, s.size() - 1);
			s.erase(i);
		}
	}

	bool t2d(const tstring& s, double& d)
	{
		tistringstream stream(s);
		stream >> d;
		return ((false == stream.fail()) && (false == stream.bad()));
		// 		try
		// 		{
		// 			d = boost::lexical_cast<double>(s);
		// 			return true;
		// 		}
		// 		catch(boost::bad_lexical_cast& e)
		// 		{
		// 		}
		// 		return false;
	}

	typedef std::vector<tstring> TStrings;

	bool get_double_from_parsed_line(MCONTACT hContact, const TStrings& rasParsedLine, size_t nIndex, const char* pszDbName)
	{
		if (rasParsedLine.size() > nIndex)
		{
			double d = 0.0;
			if (true == t2d(rasParsedLine[nIndex], d))
			{
				return Quotes_DBWriteDouble(hContact, QUOTES_MODULE_NAME, pszDbName, d);
			}
		}

		db_set_ts(hContact, QUOTES_MODULE_NAME, pszDbName, _T(""));
		return false;
	}
}

void CQuotesProviderYahoo::RefreshQuotes(TContracts& anContacts)
{
	tstring sURL = GetURL();
	bool bUseExtendedStatus = CModuleInfo::GetInstance().GetExtendedStatusFlag();

	typedef std::map<tstring, MCONTACT> TQuoteID2ContractHandles;
	TQuoteID2ContractHandles aQuoteID2Handles;
	tostringstream oURL;
	oURL << sURL << _T("dioksin.txt?s=");
	for (TContracts::const_iterator i = anContacts.begin(); i != anContacts.end() && IsOnline(); ++i)
	{
		MCONTACT hContact = *i;
		if (bUseExtendedStatus)
		{
			SetContactStatus(hContact, ID_STATUS_OCCUPIED);
		}

		tstring sQuoteID = Quotes_DBGetStringT(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_ID);
		aQuoteID2Handles[sQuoteID] = hContact;
		if (i != anContacts.begin())
		{
			oURL << _T("+");
		}
		oURL << sQuoteID;
	}

	if (true == IsOnline())
	{
		oURL << _T("&f=snl1ohgpc1");
		CHTTPSession http;
		if ((true == http.OpenURL(oURL.str())) && (true == IsOnline()))
		{
			tstring sFile;
			if ((true == http.ReadResponce(sFile)) && (true == IsOnline()))
			{
				tistringstream out_str(sFile.c_str());
				while (false == out_str.eof())
				{
					tstring sLine;
					std::getline(out_str, sLine);
					if (false == sLine.empty())
					{
						remove_end_of_line(sLine);

						TStrings asStrings;
						for (tstring::size_type nPos = sLine.find(_T(',')); nPos != tstring::npos; nPos = sLine.find(_T(',')))
						{
							tstring::iterator i(sLine.begin());
							std::advance(i, nPos);
							tstring s(sLine.begin(), i);
							remove_quotes(s);
							asStrings.push_back(s);

							if (i != sLine.end())
							{
								std::advance(i, 1);
							}
							sLine.erase(sLine.begin(), i);
						}

						if (false == sLine.empty())
						{
							remove_quotes(sLine);

							if (false == sLine.empty())
								asStrings.push_back(sLine);
						}

						size_t cItems = asStrings.size();
						if (cItems >= 3)
						{
							enum
							{
								indexSymbol = 0,
								indexName,
								indexLastTrade,
								indexOpen,
								indexDayHigh,
								indexDayLow,
								indexPreviousClose,
								indexChange
							};
							auto it3 = aQuoteID2Handles.find(asStrings[indexSymbol]);
							if (it3 != aQuoteID2Handles.end())
							{
								MCONTACT hContact = it3->second;
								double dRate = 0.0;
								if (true == t2d(asStrings[indexLastTrade], dRate))
								{
									db_set_ts(hContact, QUOTES_MODULE_NAME, DB_STR_QUOTE_DESCRIPTION, asStrings[indexName].c_str());

									get_double_from_parsed_line(hContact, asStrings, indexOpen, DB_STR_YAHOO_OPEN_VALUE);
									get_double_from_parsed_line(hContact, asStrings, indexDayHigh, DB_STR_YAHOO_DAY_HIGH);
									get_double_from_parsed_line(hContact, asStrings, indexDayLow, DB_STR_YAHOO_DAY_LOW);
									get_double_from_parsed_line(hContact, asStrings, indexPreviousClose, DB_STR_YAHOO_PREVIOUS_CLOSE);
									get_double_from_parsed_line(hContact, asStrings, indexChange, DB_STR_YAHOO_CHANGE);
									WriteContactRate(hContact, dRate);
									aQuoteID2Handles.erase(it3);
								}
							}
						}
					}
				}
			}
		}

		if (true == IsOnline())
		{
			std::for_each(aQuoteID2Handles.begin(), aQuoteID2Handles.end(),
				[](const TQuoteID2ContractHandles::value_type& pair){SetContactStatus(pair.second, ID_STATUS_NA); });
		}
	}
}

void CQuotesProviderYahoo::Accept(CQuotesProviderVisitor& visitor)const
{
	CQuotesProviderFinance::Accept(visitor);
	visitor.Visit(*this);
}
