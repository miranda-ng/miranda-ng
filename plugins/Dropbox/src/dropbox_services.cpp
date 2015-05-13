#include "stdafx.h"

int g_nStatus = ID_STATUS_OFFLINE;

HANDLE CDropbox::CreateProtoServiceFunctionObj(const char *szService, MIRANDASERVICEOBJ serviceProc, void *obj)
{
	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, SIZEOF(str), "%s%s", MODULE, szService);
	str[MAXMODULELABELLENGTH - 1] = 0;
	return CreateServiceFunctionObj(str, serviceProc, obj);
}

INT_PTR CDropbox::ProtoGetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam)
	{
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
	if (lParam)
	{
		mir_strncpy((char *)lParam, MODULE, wParam);
		return 0;
	}

	return 1;
}

INT_PTR CDropbox::ProtoLoadIcon(WPARAM wParam, LPARAM)
{
	return (LOWORD(wParam) == PLI_PROTOCOL) ? (INT_PTR)CopyIcon(LoadIconEx(IDI_DROPBOX)) : 0;
}

INT_PTR CDropbox::ProtoGetStatus(void *obj, WPARAM, LPARAM)
{
	CDropbox *instance = (CDropbox*)obj;
	return instance->HasAccessToken() ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
}

INT_PTR CDropbox::ProtoSendFile(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;
	if (!instance->HasAccessToken())
		return ACKRESULT_FAILED;

	CCSDATA *pccsd = (CCSDATA*)lParam;

	FileTransferParam *ftp = new FileTransferParam();
	ftp->pfts.flags |= PFTS_SENDING;
	ftp->pfts.hContact = pccsd->hContact;
	ftp->hContact = (instance->hTransferContact) ? instance->hTransferContact : pccsd->hContact;
	instance->hTransferContact = 0;

	TCHAR **paths = (TCHAR**)pccsd->lParam;

	for (int i = 0; paths[i]; i++)
	{
		if (PathIsDirectory(paths[i]))
			ftp->totalFolders++;
		else
			ftp->pfts.totalFiles++;
	}

	ftp->ptszFolders = (TCHAR**)mir_alloc(sizeof(TCHAR*) * (ftp->totalFolders + 1));
	ftp->ptszFolders[ftp->totalFolders] = NULL;

	ftp->pfts.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*) * (ftp->pfts.totalFiles + 1));
	ftp->pfts.ptszFiles[ftp->pfts.totalFiles] = NULL;

	for (int i = 0, j = 0, k = 0; paths[i]; i++)
	{
		if (PathIsDirectory(paths[i]))
		{
			if (!ftp->relativePathStart)
			{
				TCHAR *rootFolder = paths[j];
				TCHAR *relativePath = wcsrchr(rootFolder, '\\') + 1;
				ftp->relativePathStart = relativePath - rootFolder;
			}

			ftp->ptszFolders[j] = mir_tstrdup(&paths[i][ftp->relativePathStart]);

			j++;
		}
		else
		{
			if (!ftp->pfts.tszWorkingDir)
			{
				TCHAR *path = paths[j];
				int length = wcsrchr(path, '\\') - path;
				ftp->pfts.tszWorkingDir = (TCHAR*)mir_alloc(sizeof(TCHAR) * (length + 1));
				mir_tstrncpy(ftp->pfts.tszWorkingDir, paths[j], length + 1);
				ftp->pfts.tszWorkingDir[length] = '\0';

			}

			ftp->pfts.ptszFiles[k] = mir_wstrdup(paths[i]);

			FILE *file = _wfopen(paths[i], L"rb");
			if (file != NULL) {
				fseek(file, 0, SEEK_END);
				ftp->pfts.totalBytes += ftell(file);
				fseek(file, 0, SEEK_SET);
				fclose(file);
			}
			k++;
		}
	}

	ULONG fileId = InterlockedIncrement(&instance->hFileProcess);
	ftp->hProcess = (HANDLE)fileId;
	instance->transfers.insert(ftp);

	mir_forkthreadowner(CDropbox::SendFilesAndReportAsync, obj, ftp, 0);

	return fileId;
}

INT_PTR CDropbox::ProtoCancelFile(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;
	if (!instance->HasAccessToken())
		return ACKRESULT_FAILED;

	CCSDATA *pccsd = (CCSDATA*)lParam;

	HANDLE hTransfer = (HANDLE)pccsd->wParam;
	FileTransferParam *ftp = instance->transfers.find((FileTransferParam*)&hTransfer);
	if (ftp == NULL)
		return 0;

	ftp->isTerminated = true;

	return 0;
}

INT_PTR CDropbox::ProtoSendMessage(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;
	if (!instance->HasAccessToken())
		return ACKRESULT_FAILED;

	CCSDATA *pccsd = (CCSDATA*)lParam;

	char *message = NULL;
	char *szMessage = (char*)pccsd->lParam;
	if (pccsd->wParam & PREF_UNICODE)
		message = mir_utf8encodeW((wchar_t*)&szMessage[mir_strlen(szMessage) + 1]);
	else if (pccsd->wParam & PREF_UTF)
		message = mir_strdup(szMessage);
	else
		message = mir_utf8encode(szMessage);

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)strlen(message);
	dbei.pBlob = (PBYTE)message;
	dbei.flags = DBEF_SENT | DBEF_READ | DBEF_UTF;
	db_event_add(pccsd->hContact, &dbei);

	if (message[0] && message[0] == '/')
	{
		// parse commands
		char *sep = strchr(message, ' ');
		if (sep != NULL) *sep = 0;

		struct
		{
			const char *szCommand;
			pThreadFunc pHandler;
		}
		static commands[] =
		{
			{ "help", &CDropbox::CommandHelp },
			{ "content", &CDropbox::CommandContent },
			{ "share", &CDropbox::CommandShare },
			{ "delete", &CDropbox::CommandDelete }
		};

		for (int i = 0; i < SIZEOF(commands); i++)
		{
			if (!strcmp(message + 1, commands[i].szCommand))
			{
				ULONG messageId = InterlockedIncrement(&instance->hMessageProcess);

				CommandParam *param = new CommandParam();
				param->instance = instance;
				param->hContact = pccsd->hContact;
				param->hProcess = (HANDLE)messageId;
				param->data = (sep ? sep + 1 : NULL);

				mir_forkthread(commands[i].pHandler, param);

				return messageId;
			}
		}
	}

	char help[1024];
	mir_snprintf(help, SIZEOF(help), Translate("\"%s\" is not valid.\nUse \"/help\" for more info."), message);
	CallContactService(instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)help);

	return 0;
}

INT_PTR CDropbox::ProtoReceiveMessage(void *, WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	char *message = (char*)pccsd->lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)strlen(message);
	dbei.pBlob = (PBYTE)message;
	db_event_add(pccsd->hContact, &dbei);

	return 0;
}

INT_PTR CDropbox::SendFileToDropbox(void *obj, WPARAM hContact, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;
	if (!instance->HasAccessToken())
		return 0;

	if (hContact == NULL)
		hContact = instance->GetDefaultContact();

	TCHAR *filePath = (TCHAR*)lParam;

	FileTransferParam *ftp = new FileTransferParam();
	ftp->pfts.flags |= PFTS_SENDING;
	ftp->pfts.hContact = hContact;
	ftp->pfts.totalFiles = 1;
	ftp->hContact = (instance->hTransferContact) ? instance->hTransferContact : hContact;
	instance->hTransferContact = 0;

	int length = _tcsrchr(filePath, '\\') - filePath;
	ftp->pfts.tszWorkingDir = (TCHAR*)mir_alloc(sizeof(TCHAR) * (length + 1));
	mir_tstrncpy(ftp->pfts.tszWorkingDir, filePath, length + 1);
	ftp->pfts.tszWorkingDir[length] = '\0';

	ftp->pfts.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*) * (ftp->pfts.totalFiles + 1));
	ftp->pfts.ptszFiles[0] = mir_wstrdup(filePath);
	ftp->pfts.ptszFiles[ftp->pfts.totalFiles] = NULL;

	ULONG fileId = InterlockedIncrement(&instance->hFileProcess);
	ftp->hProcess = (HANDLE)fileId;

	mir_forkthreadowner(CDropbox::SendFilesAndEventAsync, obj, ftp, 0);

	return fileId;
}
