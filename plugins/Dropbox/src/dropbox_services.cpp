#include "stdafx.h"

INT_PTR CDropbox::ProtoGetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_OFFLINEFILES;
	}

	return 0;
}

INT_PTR CDropbox::ProtoGetName(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		mir_strncpy((char *)lParam, MODULE, wParam);
		return 0;
	}

	return 1;
}

INT_PTR CDropbox::ProtoLoadIcon(WPARAM wParam, LPARAM)
{
	return (LOWORD(wParam) == PLI_PROTOCOL) ? (INT_PTR)CopyIcon(LoadIconEx(IDI_DROPBOX)) : 0;
}

INT_PTR CDropbox::ProtoGetStatus(WPARAM, LPARAM)
{
	return HasAccessToken() ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
}

INT_PTR CDropbox::ProtoSendFile(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	if (!HasAccessToken()) {
		ProtoBroadcastAck(MODULE, pccsd->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send files when you are not authorized.");
		return 0;
	}

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

	transfers.insert(ftp);

	mir_forkthreadowner(CDropbox::UploadAndReportProgress, this, ftp, 0);

	return ftp->GetId();
}

INT_PTR CDropbox::ProtoSendFileInterceptor(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	const char *proto = GetContactProto(pccsd->hContact);
	if (!IsAccountIntercepted(proto))
	{
		auto it = interceptedContacts.find(pccsd->hContact);
		if (it == interceptedContacts.end())
			return CALLSERVICE_NOTFOUND;
	}
	
	auto it = interceptedContacts.find(pccsd->hContact);
	if (it != interceptedContacts.end())
		interceptedContacts.erase(it);

	return ProtoSendFile(wParam, lParam);
}

INT_PTR CDropbox::ProtoCancelFile(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	HANDLE hTransfer = (HANDLE)pccsd->wParam;
	FileTransferParam *ftp = transfers.find((FileTransferParam*)&hTransfer);
	if (ftp == NULL)
		return 0;

	ftp->Terminate();

	return 0;
}

INT_PTR CDropbox::ProtoSendMessage(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	if (!HasAccessToken()) {
		ProtoBroadcastAck(MODULE, pccsd->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send messages when you are not authorized.");
		return 0;
	}

	char *szMessage = (char*)pccsd->lParam;
	if (*szMessage == '/') {
		// parse commands
		char *sep = strchr(szMessage, ' ');

		struct
		{
			const char *szCommand;
			pThreadFunc pHandler;
		}
		static commands[] =
		{
			{ "help", &CDropbox::CommandHelp },
			{ "list", &CDropbox::CommandList },
			{ "share", &CDropbox::CommandShare },
			{ "search", &CDropbox::CommandSearch },
			{ "delete", &CDropbox::CommandDelete }
		};

		char command[16] = {0};
		mir_strncpy(command, szMessage + 1, sep ? sep - szMessage : mir_strlen(szMessage));
		for (int i = 0; i < _countof(commands); i++) {
			if (!mir_strcmp(command, commands[i].szCommand)) {
				ULONG messageId = InterlockedIncrement(&hMessageProcess);

				CommandParam *param = new CommandParam();
				param->instance = this;
				param->hContact = pccsd->hContact;
				param->hProcess = (HANDLE)messageId;
				param->data = (sep ? sep + 1 : NULL);

				mir_forkthread(commands[i].pHandler, param);

				return messageId;
			}
		}
	}

	ProtoBroadcastAck(MODULE, pccsd->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, 0, 0);
	char help[1024];
	mir_snprintf(help, Translate("\"%s\" is not valid.\nUse \"/help\" for more info."), szMessage);
	ProtoChainSend(GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)help);
	return 0;
}

INT_PTR CDropbox::ProtoReceiveMessage(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	char *message = (char*)pccsd->lParam;

	DBEVENTINFO dbei = {};
	dbei.flags = DBEF_UTF;
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)mir_strlen(message);
	dbei.pBlob = (PBYTE)mir_strdup(message);
	db_event_add(pccsd->hContact, &dbei);

	return 0;
}

INT_PTR CDropbox::UploadToDropbox(WPARAM wParam, LPARAM lParam)
{
	DropboxUploadInfo *uploadInfo = (DropboxUploadInfo*)lParam;

	FileTransferParam *ftp = new FileTransferParam(GetDefaultContact());
	ftp->SetWorkingDirectory(uploadInfo->localPath);
	ftp->SetServerFolder(uploadInfo->serverFolder);

	if (PathIsDirectory(uploadInfo->localPath))
	{
		// temporary unsupported

		transfers.remove(ftp);
		delete ftp;

		return ACKRESULT_FAILED;
	}
	else
		ftp->AddFile(uploadInfo->localPath);

	int res = UploadToDropbox(this, ftp);
	if (res == ACKRESULT_SUCCESS && wParam) {
		char **data = (char**)wParam;
		*data = mir_utf8encodeW(ftp->GetData());
	}

	transfers.remove(ftp);
	delete ftp;

	return res;
}

INT_PTR CDropbox::UploadToDropboxAsync(WPARAM, LPARAM lParam)
{
	DropboxUploadInfo *uploadInfo = (DropboxUploadInfo*)lParam;

	FileTransferParam *ftp = new FileTransferParam(GetDefaultContact());
	ftp->SetWorkingDirectory(uploadInfo->localPath);
	ftp->SetServerFolder(uploadInfo->serverFolder);

	if (PathIsDirectory(uploadInfo->localPath))
	{
		// temporary unsupported

		transfers.remove(ftp);
		delete ftp;

		return NULL;
	}
	else
		ftp->AddFile(uploadInfo->localPath);

	mir_forkthreadowner(CDropbox::UploadAndRaiseEvent, this, ftp, 0);

	return ftp->GetId();
}