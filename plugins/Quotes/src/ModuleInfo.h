#ifndef __d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__
#define __d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__

class CQuotesProviders;
class IXMLEngine;
class IHTMLEngine;
// class IHTMLParser;

class CModuleInfo
{
public:
	typedef boost::shared_ptr<CQuotesProviders> TQuotesProvidersPtr;
	typedef boost::shared_ptr<IXMLEngine> TXMLEnginePtr;
	typedef boost::shared_ptr<IHTMLEngine> THTMLEnginePtr;

private:
	CModuleInfo();
	~CModuleInfo(void);

public:
	static CModuleInfo& GetInstance();

	void OnMirandaShutdown();
	HANDLE GetWindowList(const std::string& rsKey, bool bAllocateIfNonExist = true);
	bool GetExtendedStatusFlag()const;

	static bool Verify();

	static TQuotesProvidersPtr GetQuoteProvidersPtr();

	static TXMLEnginePtr GetXMLEnginePtr();
	// 	static void SetXMLEnginePtr(TXMLEnginePtr pEngine);

	static THTMLEnginePtr GetHTMLEngine();
	static void SetHTMLEngine(THTMLEnginePtr pEngine);

private:
	typedef std::map<std::string, HANDLE> THandles;
	THandles m_ahWindowLists;
	bool m_bExtendedStatusInfo;
};

#endif //__d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__
