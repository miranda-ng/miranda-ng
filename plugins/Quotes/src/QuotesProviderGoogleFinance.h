#ifndef __89D3CC58_7DED_484f_AA25_62BDBB57E18B_QuotesProvider_Google_Finance_h__
#define __89D3CC58_7DED_484f_AA25_62BDBB57E18B_QuotesProvider_Google_Finance_h__

#define DB_STR_GOOGLE_FINANCE_OPEN_VALUE "OpenQuotePrice"
#define DB_STR_GOOGLE_FINANCE_DIFF "DifferentFromStartOfDay"
#define DB_STR_GOOGLE_FINANCE_PERCENT_CHANGE_TO_YERSTERDAY_CLOSE "PercentChangeToYersterdayClose"

// #define DB_STR_GOOGLE_FINANCE_COMP_NAME "CompanyName"


class CQuotesProviderGoogleFinance : public CQuotesProviderFinance
{

public:
	CQuotesProviderGoogleFinance();
	~CQuotesProviderGoogleFinance();

private:
	virtual void RefreshQuotes(TContracts& anContacts);
	virtual void Accept(CQuotesProviderVisitor& visitor)const;
};

#endif //__89D3CC58_7DED_484f_AA25_62BDBB57E18B_QuotesProvider_Google_Finance_h__
