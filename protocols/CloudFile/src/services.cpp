#include "stdafx.h"

static int CompareServices(const CCloudService *p1, const CCloudService *p2)
{
	return mir_strcmp(p1->GetAccountName(), p2->GetAccountName());
}

LIST<CCloudService> Services(10, CompareServices);

CCloudService* FindService(const char *szProto)
{
	for (auto &it : Services)
		if (!mir_strcmp(it->GetAccountName(), szProto))
			return it;
	
	return nullptr;
}

static INT_PTR GetService(WPARAM wParam, LPARAM lParam)
{
	CFSERVICEINFO *info = (CFSERVICEINFO*)lParam;
	if (info == nullptr)
		return 1;

	ptrA accountName(mir_strdup((char*)wParam));
	if (!accountName || !mir_strlen(accountName))
		accountName = g_plugin.getStringA("DefaultService");
	if (accountName == nullptr)
		return 2;

	CCloudService *service = FindService(accountName);
	if (service == nullptr)
		return 3;

	info->accountName = service->GetAccountName();
	info->userName = service->GetUserName();

	return 0;
}

static INT_PTR EnumServices(WPARAM wParam, LPARAM lParam)
{
	CFSERVICEINFO info = {};
	enumCFServiceFunc enumFunc = (enumCFServiceFunc)wParam;
	void *param = (void*)lParam;

	for (auto &service : Services) {
		info.accountName = service->GetAccountName();
		info.userName = service->GetUserName();
		int res = enumFunc(&info, param);
		if (res != 0)
			return res;
	}

	return 0;
}

INT_PTR Upload(WPARAM wParam, LPARAM lParam)
{
	CFUPLOADDATA *uploadData = (CFUPLOADDATA*)wParam;
	if (uploadData == nullptr)
		return 1;

	ptrA accountName(mir_strdup(uploadData->accountName));
	if (!mir_strlen(accountName))
		accountName = g_plugin.getStringA("DefaultService");
	if (accountName == nullptr)
		return 2;

	CCloudService *service = FindService(accountName);
	if (service == nullptr)
		return 3;

	if (PathIsDirectory(uploadData->localPath)) {
		// temporary unsupported
		return 4;
	}

	FileTransferParam ftp(0);
	ftp.SetWorkingDirectory(uploadData->localPath);
	ftp.SetServerDirectory(uploadData->serverFolder);
	ftp.AddFile(uploadData->localPath);

	int res = CCloudService::Upload(service, &ftp);
	if (res == ACKRESULT_SUCCESS && lParam) {
		size_t linkCount = 0;
		const char **links = ftp.GetSharedLinks(linkCount);
		if (linkCount > 0) {
			CFUPLOADRESULT *result = (CFUPLOADRESULT*)lParam;
			result->link = mir_strdup(links[linkCount - 1]);
		}
	}

	return res;
}

void InitializeServices()
{
	Proto_RegisterModule(PROTOTYPE_FILTER, MODULENAME);

	CreateServiceFunction(MODULENAME PSS_FILE, SendFileInterceptor);

	CreateServiceFunction(MS_CLOUDFILE_GETSERVICE, GetService);
	CreateServiceFunction(MS_CLOUDFILE_ENUMSERVICES, EnumServices);
	CreateServiceFunction(MS_CLOUDFILE_UPLOAD, Upload);
}
