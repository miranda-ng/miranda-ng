#include "stdafx.h"

LIST<FileTransferParam> Transfers(1, HandleKeySortT);

INT_PTR ProtoSendFile(void *obj, WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;
	CCloudService *service = (CCloudService*)obj;

	FileTransferParam *ftp = new FileTransferParam(pccsd->hContact);

	const wchar_t *description = (wchar_t*)pccsd->wParam;
	if (description && description[0])
		ftp->AppendFormatData(L"%s\r\n", (wchar_t*)pccsd->wParam);

	wchar_t **paths = (wchar_t**)pccsd->lParam;
	ftp->SetWorkingDirectory(paths[0]);
	for (int i = 0; paths[i]; i++) {
		if (PathIsDirectory(paths[i]))
			continue;
		ftp->AddFile(paths[i]);
	}

	Transfers.insert(ftp);

	mir_forkthreadowner(UploadAndReportProgressThread, service, ftp);

	return ftp->GetId();
}

INT_PTR ProtoSendFileInterceptor(void *obj, WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;
	CCloudService *service = (CCloudService*)obj;

	auto it = service->InterceptedContacts.find(pccsd->hContact);
	if (it == service->InterceptedContacts.end())
		return CALLSERVICE_NOTFOUND;
	service->InterceptedContacts.erase(it);

	return ProtoSendFile(obj, wParam, lParam);
}

INT_PTR ProtoCancelFile(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	HANDLE hTransfer = (HANDLE)pccsd->wParam;
	FileTransferParam *ftp = Transfers.find((FileTransferParam*)&hTransfer);
	if (ftp)
		ftp->Terminate();

	return 0;
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