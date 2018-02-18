#include "stdafx.h"

LIST<FileTransferParam> Transfers(1, HandleKeySortT);

INT_PTR CCloudService::SendFileInterceptor(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;
	for (size_t i = 0; i < Services.getCount(); i++) {
		CCloudService *service = Services[i];
		auto it = service->InterceptedContacts.find(pccsd->hContact);
		if (it == service->InterceptedContacts.end())
			return CALLSERVICE_NOTFOUND;
		service->InterceptedContacts.erase(it);
		return (INT_PTR)service->SendFile(pccsd->hContact, (wchar_t*)pccsd->wParam, (wchar_t**)pccsd->lParam);
	}
	return CALLSERVICE_NOTFOUND;
}

INT_PTR CCloudService::FileCancel(MCONTACT, HANDLE hTransfer)
{
	FileTransferParam *ftp = Transfers.find((FileTransferParam*)&hTransfer);
	if (ftp)
		ftp->Terminate();

	return 0;
}

HANDLE CCloudService::SendFile(MCONTACT hContact, const wchar_t *description, wchar_t **paths)
{
	FileTransferParam *ftp = new FileTransferParam(hContact);

	if (description && description[0])
		ftp->AppendFormatData(L"%s\r\n", description);

	ftp->SetWorkingDirectory(paths[0]);
	for (int i = 0; paths[i]; i++) {
		if (PathIsDirectory(paths[i]))
			continue;
		ftp->AddFile(paths[i]);
	}

	Transfers.insert(ftp);

	mir_forkthreadowner(UploadAndReportProgressThread, this, ftp);

	return (HANDLE)ftp->GetId();
}

void CCloudService::OpenUploadDialog(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if (!mir_strcmpi(proto, META_PROTO))
		hContact = CallService(MS_MC_GETMOSTONLINECONTACT, hContact);

	auto it = InterceptedContacts.find(hContact);
	if (it == InterceptedContacts.end())
	{
		HWND hwnd = (HWND)CallService(MS_FILE_SENDFILE, hContact, 0);
		InterceptedContacts[hContact] = hwnd;
	}
	else
		SetActiveWindow(it->second);
}

UINT UploadAndReportProgressThread(void *owner, void *arg)
{
	CCloudService *service = (CCloudService*)owner;
	FileTransferParam *ftp = (FileTransferParam*)arg;

	int res = service->Upload(ftp);
	if (res == ACKRESULT_SUCCESS)
		service->Report(ftp->GetHContact(), ftp->GetData());

	Transfers.remove(ftp);
	delete ftp;

	return res;
}
