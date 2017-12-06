#ifndef __2b5ddd05_9255_4be0_9408_e59768b70568_QuotesProviderVisitorFormater_h__
#define __2b5ddd05_9255_4be0_9408_e59768b70568_QuotesProviderVisitorFormater_h__

class CQuotesProviderVisitorFormater : public CQuotesProviderVisitor
{
public:
	CQuotesProviderVisitorFormater(MCONTACT hContact, wchar_t chr, int nWidth);
	~CQuotesProviderVisitorFormater();

	const tstring& GetResult()const;

private:
	virtual void Visit(const CQuotesProviderDukasCopy& rProvider);
	virtual void Visit(const CQuotesProviderGoogle& rProvider);
	virtual void Visit(const CQuotesProviderBase& rProvider);
	virtual void Visit(const CQuotesProviderGoogleFinance& rProvider);
	virtual void Visit(const CQuotesProviderYahoo& rProvider);

private:
	void FormatDoubleHelper(LPCSTR pszDbSet, const tstring sInvalid = L"-");
	// 	void FormatChangeValueHelper(bool bPercentage);

private:
	MCONTACT m_hContact;
	wchar_t m_chr;
	tstring m_sResult;
	int m_nWidth;
};

#endif //__2b5ddd05_9255_4be0_9408_e59768b70568_QuotesProviderVisitorFormater_h__
