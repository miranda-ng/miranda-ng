#ifndef __2b5ddd05_9255_4be0_9408_e59768b70568_CurrencyRatesProviderVisitorFormater_h__
#define __2b5ddd05_9255_4be0_9408_e59768b70568_CurrencyRatesProviderVisitorFormater_h__

class CCurrencyRatesProviderVisitorFormater : public CCurrencyRatesProviderVisitor
{
public:
	CCurrencyRatesProviderVisitorFormater(MCONTACT hContact, wchar_t chr, int nWidth);
	~CCurrencyRatesProviderVisitorFormater();

	const tstring& GetResult() const;

private:
	virtual void Visit(const CCurrencyRatesProviderBase& rProvider) override;
	virtual void Visit(const CCurrencyRatesProviderCurrencyConverter& rProvider) override;

private:
	void FormatDoubleHelper(LPCSTR pszDbSet, const tstring sInvalid = L"-");

private:
	MCONTACT m_hContact;
	wchar_t m_chr;
	tstring m_sResult;
	int m_nWidth;
};

#endif //__2b5ddd05_9255_4be0_9408_e59768b70568_CurrencyRatesProviderVisitorFormater_h__
