#ifndef __AD721194_E9944366_9CF1_0307460EF32F_QuotesProviderVisitorTendency_h__
#define __AD721194_E9944366_9CF1_0307460EF32F_QuotesProviderVisitorTendency_h__

class CQuotesProviderVisitorTendency : public CQuotesProviderVisitor
{
public:
	CQuotesProviderVisitorTendency(MCONTACT hContact, TCHAR chr);

	bool IsValid()const{ return m_bValid; }
	double GetResult()const{ return m_dResult; }

private:
	virtual void Visit(const CQuotesProviderBase& rProvider);
	virtual void Visit(const CQuotesProviderGoogleFinance& rProvider);
	virtual void Visit(const CQuotesProviderYahoo& rProvider);

private:
	void GetValue(LPCSTR pszDbKeyName);

private:
	MCONTACT m_hContact;
	TCHAR m_chr;
	bool m_bValid;
	double m_dResult;
};

#endif //__AD721194_E9944366_9CF1_0307460EF32F_QuotesProviderVisitorTendency_h__
