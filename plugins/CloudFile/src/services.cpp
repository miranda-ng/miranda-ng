#include "stdafx.h"

static int CompareServices(const CCloudService *p1, const CCloudService *p2)
{
	return mir_strcmp(p1->GetAccountName(), p2->GetAccountName());
}

LIST<CCloudService> Services(10, CompareServices);

static INT_PTR GetService(WPARAM wParam, LPARAM lParam)
{
	ptrA accountName(mir_strdup((char*)wParam));
	if (!accountName || !mir_strlen(accountName))
		accountName = db_get_sa(NULL, MODULE, "DefaultService");
	if (accountName == nullptr)
		return 2;

	CCloudServiceSearch search(accountName);
	CCloudService *service = Services.find(&search);
	if (service == nullptr)
		return 3;

	CFSERVICEINFO *info = (CFSERVICEINFO*)lParam;
	if (info != nullptr) {
		info->accountName = service->GetAccountName();
		info->userName = service->GetUserName();
	}

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
		accountName = db_get_sa(NULL, MODULE, "DefaultService");
	if (accountName == nullptr)
		return 2;

	CCloudServiceSearch search(uploadData->accountName);
	CCloudService *service = Services.find(&search);
	if (service == nullptr)
		return 3;

	if (PathIsDirectory(uploadData->localPath)) {
		// temporary unsupported
		return 4;
	}

	FileTransferParam ftp(0);
	ftp.SetWorkingDirectory(uploadData->localPath);
	ftp.SetServerFolder(uploadData->serverFolder);
	ftp.AddFile(uploadData->localPath);

	int res = service->Upload(&ftp);
	if (res == ACKRESULT_SUCCESS && lParam) {
		CFUPLOADRESULT *result = (CFUPLOADRESULT*)lParam;
		const char **links = ftp.GetSharedLinks(result->linkCount);
		result->links = (char**)mir_calloc(sizeof(char*) * result->linkCount);
		for (size_t i = 0; i < result->linkCount; i++)
			result->links[i] = mir_strdup(links[i]);
		result->description = mir_wstrdup(ftp.GetDescription());
	}

	return res;
}

void InitializeServices()
{
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.type = PROTOTYPE_FILTER;
	pd.szName = MODULE;
	Proto_RegisterModule(&pd);

	CreateServiceFunction(MODULE PSS_FILE, SendFileInterceptor);

	CreateServiceFunction(MS_CLOUDFILE_GETSERVICE, GetService);
	CreateServiceFunction(MS_CLOUDFILE_ENUMSERVICES, EnumServices);
	CreateServiceFunction(MS_CLOUDFILE_UPLOAD, Upload);
}
