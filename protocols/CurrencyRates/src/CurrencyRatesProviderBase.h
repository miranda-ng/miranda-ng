#ifndef __3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__
#define __3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRate - one currency

class CCurrencyRate
{
public:
	CCurrencyRate(const std::wstring& rsID = L"", const std::wstring& rsSymbol = L"", const std::wstring& rsName = L"")
		: m_sSymbol(rsSymbol), m_sName(rsName), m_sID(rsID){}

	const std::wstring& GetSymbol() const{ return m_sSymbol; }
	const std::wstring& GetName() const{ return m_sName; }
	const std::wstring& GetID() const{ return m_sID; }

private:
	std::wstring m_sSymbol;
	std::wstring m_sName;
	std::wstring m_sID;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRateSection - block of currency settings

class CCurrencyRateSection
{
public:
	typedef std::vector<CCurrencyRateSection> TSections;
	typedef std::vector<CCurrencyRate> TCurrencyRates;

public:
	CCurrencyRateSection(const std::wstring& rsName = L"", const TSections& raSections = TSections(), const TCurrencyRates& raCurrencyRates = TCurrencyRates())
		: m_sName(rsName), m_aSections(raSections), m_aCurrencyRates(raCurrencyRates){}

	const std::wstring& GetName() const
	{
		return m_sName;
	}

	size_t GetSectionCount() const
	{
		return m_aSections.size();
	}

	CCurrencyRateSection GetSection(size_t nIndex) const
	{
		return ((nIndex < m_aSections.size()) ? m_aSections[nIndex] : CCurrencyRateSection());
	}

	size_t GetCurrencyRateCount() const
	{
		return m_aCurrencyRates.size();
	}

	CCurrencyRate GetCurrencyRate(size_t nIndex) const
	{
		return ((nIndex < m_aCurrencyRates.size()) ? m_aCurrencyRates[nIndex] : CCurrencyRate());
	}

private:
	std::wstring m_sName;
	TSections m_aSections;
	TCurrencyRates m_aCurrencyRates;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRatesProviderBase - basic set of methods for retrieving currencies

typedef std::vector<MCONTACT> TContacts;

class CCurrencyRatesProviderBase : public ICurrencyRatesProvider
{
	void OnEndRun();

	struct CXMLFileInfo *m_pXMLInfo = nullptr;

	HANDLE m_hEventSettingsChanged;
	HANDLE m_hEventRefreshContact;
	std::wstring m_sContactListFormat;
	std::wstring m_sStatusMsgFormat;
	std::wstring m_sTendencyFormat;
	TContacts m_aRefreshingContacts;
	bool m_bRefreshInProgress;

public:
	CCurrencyRatesProviderBase();
	~CCurrencyRatesProviderBase();

	const CCurrencyRateSection& GetCurrencyRates() const;

	bool Init() override;
	const CProviderInfo& GetInfo() const override;
	
	void AddContact(MCONTACT hContact) override;
	void DeleteContact(MCONTACT hContact) override;
	
	void Run() override;
	
	void RefreshAllContacts() override;
	void RefreshSettings() override;
	void RefreshContact(MCONTACT hContact) override;
	
	void FillFormat(TFormatSpecificators&) const override;
	bool ParseSymbol(MCONTACT hContact, wchar_t c, double &d) override;
	std::wstring FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth = 0) const override;

protected:
	const std::wstring& GetURL() const;
	MCONTACT CreateNewContact(const std::wstring& rsName);
	static bool IsOnline();
	static void SetContactStatus(MCONTACT hContact, int nNewStatus);
	void WriteContactRate(MCONTACT hContact, double dRate, const std::wstring& rsSymbol = L"");

	virtual void RefreshCurrencyRates(TContacts &anContacts) = 0;

protected:
	TContacts m_aContacts;
	mutable mir_cs m_cs;
};

#endif //__3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__
