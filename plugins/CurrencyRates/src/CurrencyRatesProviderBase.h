#ifndef __3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__
#define __3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRate - one currency

class CCurrencyRate
{
public:
	CCurrencyRate(const tstring& rsID = L"", const tstring& rsSymbol = L"", const tstring& rsName = L"")
		: m_sSymbol(rsSymbol), m_sName(rsName), m_sID(rsID){}

	const tstring& GetSymbol() const{ return m_sSymbol; }
	const tstring& GetName() const{ return m_sName; }
	const tstring& GetID() const{ return m_sID; }

private:
	tstring m_sSymbol;
	tstring m_sName;
	tstring m_sID;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRateSection - block of currency settings

class CCurrencyRateSection
{
public:
	typedef std::vector<CCurrencyRateSection> TSections;
	typedef std::vector<CCurrencyRate> TCurrencyRates;

public:
	CCurrencyRateSection(const tstring& rsName = L"", const TSections& raSections = TSections(), const TCurrencyRates& raCurrencyRates = TCurrencyRates())
		: m_sName(rsName), m_aSections(raSections), m_aCurrencyRates(raCurrencyRates){}

	const tstring& GetName() const
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
	tstring m_sName;
	TSections m_aSections;
	TCurrencyRates m_aCurrencyRates;
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRatesProviderBase - basic set of methods for retrieving currencies

typedef std::vector<MCONTACT> TContacts;

class CCurrencyRatesProviderBase : public ICurrencyRatesProvider
{
	void OnEndRun();

	struct CXMLFileInfo* GetXMLFileInfo() const;

	typedef boost::scoped_ptr<CXMLFileInfo> TXMLFileInfoPtr;
	mutable TXMLFileInfoPtr m_pXMLInfo;
	HANDLE m_hEventSettingsChanged;
	HANDLE m_hEventRefreshContact;
	tstring m_sContactListFormat;
	tstring m_sStatusMsgFormat;
	tstring m_sTendencyFormat;
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
	tstring FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth = 0) const override;

protected:
	const tstring& GetURL() const;
	MCONTACT CreateNewContact(const tstring& rsName);
	static bool IsOnline();
	static void SetContactStatus(MCONTACT hContact, int nNewStatus);
	void WriteContactRate(MCONTACT hContact, double dRate, const tstring& rsSymbol = L"");

	virtual void RefreshCurrencyRates(TContacts &anContacts) = 0;

protected:
	TContacts m_aContacts;
	mutable mir_cs m_cs;
};

#endif //__3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__
