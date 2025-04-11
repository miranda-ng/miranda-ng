#include "stdafx.h"

static int CompareServices(const CCloudService *p1, const CCloudService *p2)
{
	return mir_strcmp(p1->m_szModuleName, p2->m_szModuleName);
}

LIST<CCloudService> g_arServices(10, CompareServices);

CCloudService* FindService(const char *szProto)
{
	for (auto &it : g_arServices)
		if (!mir_strcmp(it->m_szModuleName, szProto))
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

	info->accountName = service->m_szModuleName;
	info->userName = service->m_tszUserName;
	return 0;
}

static INT_PTR EnumServices(WPARAM wParam, LPARAM lParam)
{
	CFSERVICEINFO info = {};
	enumCFServiceFunc enumFunc = (enumCFServiceFunc)wParam;
	void *param = (void*)lParam;

	for (auto &service : g_arServices) {
		info.accountName = service->m_szModuleName;
		info.userName = service->m_tszUserName;
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
