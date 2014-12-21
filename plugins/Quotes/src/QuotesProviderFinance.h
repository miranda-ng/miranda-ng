#ifndef __95A13A68_0DF0_43FA_B6C1_81D83AED59AA_QuotesProviderFinance_h__
#define __95A13A68_0DF0_43FA_B6C1_81D83AED59AA_QuotesProviderFinance_h__

class CQuotesProviderFinance : public CQuotesProviderBase
{
public:
	typedef std::vector<CQuotesProviderBase::CQuote> TQuotes;

public:
	void GetWatchedQuotes(TQuotes& raQuotes)const;
	bool WatchForQuote(const CQuote& rQuote, bool bWatch);
	MCONTACT GetContactByQuoteID(const tstring& rsQuoteID)const;

protected:
	virtual void ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp);
	virtual void Accept(CQuotesProviderVisitor& visitor)const;
};

#endif //__95A13A68_0DF0_43FA_B6C1_81D83AED59AA_QuotesProviderFinance_h__