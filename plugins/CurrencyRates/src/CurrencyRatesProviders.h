#ifndef __148306d1_da2a_43df_b1ad_0cdc8ef8a79e_CurrencyRatesProviders_h__
#define __148306d1_da2a_43df_b1ad_0cdc8ef8a79e_CurrencyRatesProviders_h__

class ICurrencyRatesProvider;

class CCurrencyRatesProviders
{
public:
	typedef boost::shared_ptr<ICurrencyRatesProvider> TCurrencyRatesProviderPtr;
	typedef std::vector<TCurrencyRatesProviderPtr> TCurrencyRatesProviders;

public:
	CCurrencyRatesProviders();
	~CCurrencyRatesProviders();

	TCurrencyRatesProviderPtr FindProvider(const tstring& rsName)const;
	TCurrencyRatesProviderPtr GetContactProviderPtr(MCONTACT hContact)const;
	const TCurrencyRatesProviders& GetProviders()const;

private:
	void InitProviders();
	void CreateProviders();
	void ClearProviders();

private:
	TCurrencyRatesProviders m_apProviders;
};

#endif //__148306d1_da2a_43df_b1ad_0cdc8ef8a79e_CurrencyRatesProviders_h__
