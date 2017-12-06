#ifndef __148306d1_da2a_43df_b1ad_0cdc8ef8a79e_QuotesProviders_h__
#define __148306d1_da2a_43df_b1ad_0cdc8ef8a79e_QuotesProviders_h__

class IQuotesProvider;

class CQuotesProviders
{
public:
	typedef boost::shared_ptr<IQuotesProvider> TQuotesProviderPtr;
	typedef std::vector<TQuotesProviderPtr> TQuotesProviders;

public:
	CQuotesProviders();
	~CQuotesProviders();

	TQuotesProviderPtr FindProvider(const tstring& rsName)const;
	TQuotesProviderPtr GetContactProviderPtr(MCONTACT hContact)const;
	const TQuotesProviders& GetProviders()const;

private:
	void InitProviders();
	void CreateProviders();
	void ClearProviders();

private:
	TQuotesProviders m_apProviders;
};

#endif //__148306d1_da2a_43df_b1ad_0cdc8ef8a79e_QuotesProviders_h__
