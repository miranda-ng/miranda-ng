#ifndef __d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__
#define __d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__

class CCurrencyRatesProviders;
class IXMLEngine;
class IHTMLEngine;
// class IHTMLParser;

class CModuleInfo
{
public:
	typedef boost::shared_ptr<CCurrencyRatesProviders> TCurrencyRatesProvidersPtr;
	typedef boost::shared_ptr<IXMLEngine> TXMLEnginePtr;
	typedef boost::shared_ptr<IHTMLEngine> THTMLEnginePtr;

private:
	CModuleInfo();
	~CModuleInfo(void);

public:
	static CModuleInfo& GetInstance();

	void OnMirandaShutdown();
	MWindowList GetWindowList(const std::string& rsKey, bool bAllocateIfNonExist = true);

	static bool Verify();

	static TCurrencyRatesProvidersPtr GetCurrencyRateProvidersPtr();

	static TXMLEnginePtr GetXMLEnginePtr();
	// 	static void SetXMLEnginePtr(TXMLEnginePtr pEngine);

	static THTMLEnginePtr GetHTMLEngine();
	static void SetHTMLEngine(THTMLEnginePtr pEngine);

private:
	typedef std::map<std::string, MWindowList> THandles;
	THandles m_ahWindowLists;
};

#endif //__d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__
