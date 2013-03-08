#ifndef __E927F394_5452_458E_AF48_71E44F9EE793_QuotesProviderYahoo_h__
#define __E927F394_5452_458E_AF48_71E44F9EE793_QuotesProviderYahoo_h__

#define DB_STR_YAHOO_OPEN_VALUE "OpenQuotePrice"
#define DB_STR_YAHOO_DAY_HIGH "DayHigh"
#define DB_STR_YAHOO_DAY_LOW "DayLow"
#define DB_STR_YAHOO_PREVIOUS_CLOSE "PreviousClose"
#define DB_STR_YAHOO_CHANGE "Change"

class CQuotesProviderYahoo : public CQuotesProviderFinance
{
private:
	virtual void RefreshQuotes(TContracts& anContacts);
	virtual void Accept(CQuotesProviderVisitor& visitor)const;
};

#endif //__E927F394_5452_458E_AF48_71E44F9EE793_QuotesProviderYahoo_h__
