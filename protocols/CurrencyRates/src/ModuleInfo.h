#ifndef __d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__
#define __d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__

class CCurrencyRatesProviders;

class CModuleInfo
{
public:
	typedef boost::shared_ptr<CCurrencyRatesProviders> TCurrencyRatesProvidersPtr;

public:
	static void OnMirandaShutdown(void);
	static MWindowList GetWindowList(const std::string& rsKey, bool bAllocateIfNonExist = true);
};

#endif //__d0f22b66_3135_4bbe_bee5_a31ea631ce58_ModuleInfo__
