#include "stdafx.h"

static int CompareServices(const CCloudService *p1, const CCloudService *p2)
{
	return mir_strcmp(p1->GetAccountName(), p2->GetAccountName());
}

LIST<CCloudService> Services(10, CompareServices);

static INT_PTR GetServiceCount(WPARAM, LPARAM)
{
	return Services.getCount();
}

static INT_PTR GetService(WPARAM wParam, LPARAM lParam)
{
	ptrA accountName(mir_strdup((char*)wParam));
	if (!accountName || !mir_strlen(accountName))
		accountName = db_get_sa(NULL, MODULE, "DefaultService");
	if (accountName == nullptr)
		return 1;
	CCloudServiceSearch search(accountName);
	CCloudService *service = Services.find(&search);
	if (service == nullptr)
		return 2;
	CFSERVICEINFO *info = (CFSERVICEINFO*)lParam;
	if (info != nullptr) {
		info->AccountName = service->GetAccountName();
		info->UserName = service->GetUserName();
	}
	return 0;
}

static INT_PTR EnumServices(WPARAM wParam, LPARAM lParam)
{
	CFSERVICEINFO info = {};
	enumCFServiceFunc enumFunc = (enumCFServiceFunc)wParam;
	void *param = (void*)lParam;
	for (auto &service : Services) {
		info.AccountName = service->GetAccountName();
		info.UserName = service->GetUserName();
		int res = enumFunc(&info, param);
		if (res != 0)
			return res;
	}
	return 0;
}

void InitServices()
{
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.type = PROTOTYPE_PROTOCOL;

	pd.szName = MODULE "/Dropbox";
	pd.fnInit = (pfnInitProto)CDropboxService::Init;
	pd.fnUninit = (pfnUninitProto)CDropboxService::UnInit;
	Proto_RegisterModule(&pd);

	pd.szName = MODULE "/GDrive";
	pd.fnInit = (pfnInitProto)CGDriveService::Init;
	pd.fnUninit = (pfnUninitProto)CGDriveService::UnInit;
	Proto_RegisterModule(&pd);

	pd.szName = MODULE "/OneDrivre";
	pd.fnInit = (pfnInitProto)COneDriveService::Init;
	pd.fnUninit = (pfnUninitProto)COneDriveService::UnInit;
	Proto_RegisterModule(&pd);

	pd.szName = MODULE "/YandexDisk";
	pd.fnInit = (pfnInitProto)CYandexService::Init;
	pd.fnUninit = (pfnUninitProto)CYandexService::UnInit;
	Proto_RegisterModule(&pd);

	pd.szName = MODULE;
	pd.type = PROTOTYPE_FILTER;
	Proto_RegisterModule(&pd);

	CreateServiceFunction(MODULE PSS_FILE, &CCloudService::SendFileInterceptor);

	CreateServiceFunction(MS_CLOUDFILE_GETSERVICE, GetService);
	CreateServiceFunction(MS_CLOUDFILE_ENUMSERVICES, EnumServices);
}