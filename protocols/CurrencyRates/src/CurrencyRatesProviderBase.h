#ifndef __3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__
#define __3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRate - one currency

class CCurrencyRate
{
	CMStringW m_sSymbol, m_sName, m_sID;

public:
	CCurrencyRate(const CMStringW &rsID = L"", const CMStringW &rsSymbol = L"", const CMStringW &rsName = L"") :
		m_sSymbol(rsSymbol), m_sName(rsName), m_sID(rsID)
	{}

	const CMStringW& GetSymbol() const{ return m_sSymbol; }
	const CMStringW& GetName() const{ return m_sName; }
	const CMStringW& GetID() const{ return m_sID; }

	const CMStringW& MakeName() const { return m_sName.IsEmpty() ? m_sID : m_sName; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRateSection - block of currency settings

class CCurrencyRateSection
{
public:
	typedef std::vector<CCurrencyRate> TCurrencyRates;

private:
	CMStringW m_sName;
	TCurrencyRates m_aCurrencyRates;

public:
	CCurrencyRateSection(const CMStringW &rsName = L"", const TCurrencyRates &raCurrencyRates = TCurrencyRates()) :
		m_sName(rsName), m_aCurrencyRates(raCurrencyRates)
	{}

	const CMStringW& GetName() const
	{
		return m_sName;
	}

	const TCurrencyRates& GetCurrencyRates() const
	{
		return m_aCurrencyRates;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// CCurrencyRatesProviderBase - basic set of methods for retrieving currencies

typedef std::vector<MCONTACT> TContacts;

class CCurrencyRatesProviderBase : public ICurrencyRatesProvider
{
	struct CXMLFileInfo *m_pXMLInfo = nullptr;

	HANDLE m_hEventSettingsChanged;
	HANDLE m_hEventRefreshContact;
	CMStringW m_sContactListFormat;
	CMStringW m_sStatusMsgFormat;
	CMStringW m_sTendencyFormat;
	TContacts m_aRefreshingContacts;
	bool m_bRefreshInProgress;

public:
	CCurrencyRatesProviderBase();
	~CCurrencyRatesProviderBase();

	const CCurrencyRateSection& GetSection() const;

	bool Init() override;
	const CProviderInfo& GetInfo() const override;

	void Run() override;
	
	void RefreshAllContacts() override;
	void RefreshSettings() override;
	void RefreshContact(MCONTACT hContact) override;

	__forceinline bool HasAuth() const { return m_bRequiresAuth; }

	MCONTACT ImportContact(const TiXmlNode *) override;

	using TRateInfo = std::pair<CCurrencyRate, CCurrencyRate>;
	bool GetWatchedRateInfo(MCONTACT hContact, TRateInfo &rRateInfo);
	bool WatchForRate(const TRateInfo &ri, bool bWatch);
	MCONTACT GetContactByID(const CMStringW &rsFromID, const CMStringW &rsToID) const;

	virtual double Convert(double dAmount, const CCurrencyRate &from, const CCurrencyRate &to) const = 0;

	void FillFormat(TFormatSpecificators&) const override;
	bool ParseSymbol(MCONTACT hContact, wchar_t c, double &d) const override;
	CMStringW FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth = 0) const override;

protected:
	const CMStringW& GetURL() const;
	MCONTACT CreateNewContact(const CMStringW &rsName);
	static void SetContactStatus(MCONTACT hContact, int nNewStatus);
	void WriteContactRate(MCONTACT hContact, double dRate, const CMStringW &rsSymbol = L"");

	virtual wchar_t* GetXmlFilename() const = 0;
	virtual void RefreshCurrencyRates(TContacts &anContacts) = 0;

protected:
	mir_cs m_cs;
	bool m_bRequiresAuth = true;
};

bool show_popup(const ICurrencyRatesProvider *pProvider, MCONTACT hContact, int nComparison, const CMStringW &rsFormat);

#endif //__3e6cb4ec_fc47_468f_a2c8_a77941176bc9_CurrencyRatesProviderBase_h__
