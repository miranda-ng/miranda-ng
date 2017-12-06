#ifndef __93121758_68c7_4836_b571_da84dfe82b84_QuotesProviderDukasCopy_h__
#define __93121758_68c7_4836_b571_da84dfe82b84_QuotesProviderDukasCopy_h__

#define DB_STR_REFRESH_RATE_TYPE "RefreshRateType"
#define DB_STR_REFRESH_RATE_VALUE "RefreshRateValue"
#define DB_STR_DC_DISPLAY_NAME_FORMAT "DC_DisplayNameFormat"
// #define DB_STR_DC_LOG_FILE_FORMAT "DC_LogFileFormat"
// #define DB_STR_DC_HISTORY_FORMAT "DC_HistoryFormat"

class CQuotesProviderDukasCopy : public CQuotesProviderBase
{
public:
	CQuotesProviderDukasCopy();
	~CQuotesProviderDukasCopy();

	bool WatchForQuote(const CQuote& rQuote, bool bWatch);
	bool IsQuoteWatched(const CQuote& rQuote)const;

	MCONTACT GetContactByQuoteID(const tstring& rsQuoteID)const;
	// #ifdef CHART_IMPLEMENT
	// 	bool Chart(MCONTACT hContact,const tstring& url)const;
	// #endif

private:
	//IQuotesProvider implementation
	virtual void RefreshQuotes(TContracts& anContacts);
	virtual void ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp);
	virtual void Accept(CQuotesProviderVisitor& visitor)const;

private:
	tstring BuildHTTPURL()const;

};

#endif //__93121758_68c7_4836_b571_da84dfe82b84_QuotesProviderDukasCopy_h__
