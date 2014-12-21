#ifndef __c0e48a95_b3f0_4227_8adc_455e265f3c14_QuotesProviderGoogle_h__
#define __c0e48a95_b3f0_4227_8adc_455e265f3c14_QuotesProviderGoogle_h__

#define DB_STR_FROM_ID "FromID"
#define DB_STR_TO_ID "ToID"
#define DB_STR_FROM_DESCRIPTION "FromDesc"
#define DB_STR_TO_DESCRIPTION "ToDesc"
#define DB_STR_GOOGLE_REFRESH_RATE_TYPE "Google_RefreshRateType"
#define DB_STR_GOOGLE_REFRESH_RATE_VALUE "Google_RefreshRateValue"
#define DB_STR_GOOGLE_DISPLAY_NAME_FORMAT "Google_DspNameFrmt"
// #define DB_STR_GOOGLE_LOG_FILE_FORMAT "Google_LogFileFormat"
// #define DB_STR_GOOGLE_HISTORY_FORMAT "Google_HistoryFormat"

class CQuotesProviderGoogle : public CQuotesProviderBase
{
public:
	struct CRateInfo
	{
		CQuotesProviderBase::CQuote m_from;
		CQuotesProviderBase::CQuote m_to;
	};
public:
	CQuotesProviderGoogle();
	~CQuotesProviderGoogle();

	bool WatchForRate(const CRateInfo& ri, bool bWatch);
	size_t GetWatchedRateCount()const;
	bool GetWatchedRateInfo(size_t nIndex, CRateInfo& rRateInfo);

	MCONTACT GetContactByID(const tstring& rsFromID, const tstring& rsToID)const;

	double Convert(double dAmount, const CQuote& from, const CQuote& to)const;

private:
	virtual void RefreshQuotes(TContracts& anContacts);
	virtual void ShowPropertyPage(WPARAM wp, OPTIONSDIALOGPAGE& odp);
	virtual void Accept(CQuotesProviderVisitor& visitor)const;
};

#endif //__c0e48a95_b3f0_4227_8adc_455e265f3c14_QuotesProviderGoogle_h__
