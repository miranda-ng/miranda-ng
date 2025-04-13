#include "stdafx.h"

LIST<FileTransferParam> Transfers(1, HandleKeySortT);

INT_PTR SendFileInterceptor(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;
	for (auto &service : g_arServices) {
		auto it = service->InterceptedContacts.find(pccsd->hContact);
		if (it == service->InterceptedContacts.end())
			continue;
		service->InterceptedContacts.erase(it);
		return (INT_PTR)service->SendFile(pccsd->hContact, (wchar_t*)pccsd->wParam, (wchar_t**)pccsd->lParam);
	}
	return CALLSERVICE_NOTFOUND;
}

UINT UploadAndReportProgressThread(void *owner, void *arg)
{
	CCloudService *service = (CCloudService*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = CCloudService::Upload(service, ftp);
	if (res == ACKRESULT_SUCCESS) {
		CMStringW data = ftp->GetDescription();
		size_t linkCount;
		auto links = ftp->GetSharedLinks(linkCount);
		for (size_t i = 0; i < linkCount; i++) {
			data.Append(ptrW(mir_utf8decodeW(links[i])));
			data.AppendChar(0x0A);
		}
		Report(ftp->GetContact(), data);
	}

	Transfers.remove(ftp);
	delete ftp;

	return res;
}
