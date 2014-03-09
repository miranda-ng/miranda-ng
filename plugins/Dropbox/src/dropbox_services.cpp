#include "common.h"

INT_PTR CDropbox::ProtoGetCaps(WPARAM wParam, LPARAM)
{
	switch(wParam)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)MODULE " ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"uid";
	}

	return 0;
}

INT_PTR CDropbox::ProtoSendFile(void *obj, WPARAM wParam, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	if (!instance->HasAccessToken())
		return ACKRESULT_FAILED;

	CCSDATA *pccsd = (CCSDATA*)lParam;

	FileTransferParam *ftp = new FileTransferParam(instance);
	ftp->pfts.flags = PFTS_SENDING | PFTS_UTF;
	ftp->pfts.hContact = pccsd->hContact;
	ftp->hContact = (instance->hTransferContact) ? instance->hTransferContact : pccsd->hContact;
	instance->hTransferContact = 0;

	char **files = (char**)pccsd->lParam;

	for (int i = 0; files[i]; i++)
	{
		if (PathIsDirectoryA(files[i]))
			ftp->totalFolders++;
		else
			ftp->pfts.totalFiles++;
	}

	ftp->pszFolders = new char*[ftp->totalFolders + 1];
	ftp->pszFolders[ftp->totalFolders] = NULL;

	ftp->pfts.pszFiles = new char*[ftp->pfts.totalFiles + 1];
	ftp->pfts.pszFiles[ftp->pfts.totalFiles] = NULL;

	for (int i = 0, j = 0, k = 0; files[i]; i++)
	{
		if (PathIsDirectoryA(files[i]))
		{
			if (!ftp->relativePathStart)
			{
				char *rootFolder = files[j];
				char *relativePath = strrchr(rootFolder, '\\') + 1;
				ftp->relativePathStart = relativePath - rootFolder;
			}

			ftp->pszFolders[j] = mir_strdup(&files[i][ftp->relativePathStart]);

			j++;
		}
		else
		{
			ftp->pfts.pszFiles[k] = mir_strdup(files[i]);

			FILE *file = fopen(files[i], "rb");
			if (file != NULL)
			{
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

	mir_forkthread(CDropbox::SendFileAsync, ftp);

	return fileId;
}

INT_PTR CDropbox::ProtoSendMessage(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	CCSDATA *pccsd = (CCSDATA*)lParam;

	char *message = (char*)pccsd->lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = strlen(message);
	dbei.pBlob = (PBYTE)message;
	dbei.flags = DBEF_SENT | DBEF_UTF;
	db_event_add(pccsd->hContact, &dbei);

	char help[1024];

	if (message[0] && message[0] == '/')
	{
		// parse commands
		char *sep = strchr(message, ' ');
		int len = strlen(message) - (sep ? strlen(sep) : 0) - 1;
		ptrA cmd((char*)mir_alloc(len + 1));
		strncpy(cmd, message + 1, len);
		cmd[len] = 0;
		if (instance->commands.find((char*)cmd) != instance->commands.end())
		{
			ULONG messageId = InterlockedIncrement(&instance->hMessageProcess);

			CommandParam *param = new CommandParam();
			param->instance = instance;
			param->hContact = pccsd->hContact;
			param->hProcess = (HANDLE)messageId;
			param->data = (sep ? sep + 1 : NULL);

			mir_forkthread(instance->commands[(char*)cmd], param);

			return messageId;
		}
		else
		{
			mir_snprintf(
				help,
				SIZEOF(help),
				Translate("Unknown command \"%s\".\nUse \"/help\" for more info."),
				message);

			CallContactService(instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)help);

			return 0;
		}
	}

	mir_snprintf(
		help,
		SIZEOF(help),
		Translate("\"%s\" is not valid.\nUse \"/help\" for more info."),
		message);

	CallContactService(instance->GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)help);

	return 0;
}

INT_PTR CDropbox::ProtoReceiveMessage(void *obj, WPARAM, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	CCSDATA *pccsd = (CCSDATA*)lParam;

	char *message = (char*)pccsd->lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = strlen(message);
	dbei.pBlob = (PBYTE)message;
	db_event_add(pccsd->hContact, &dbei);

	return 0;
}

INT_PTR CDropbox::SendFileToDropbox(void *obj, WPARAM hContact, LPARAM lParam)
{
	CDropbox *instance = (CDropbox*)obj;

	const char *filePath = (char*)lParam;
	const char *fileName = strrchr(filePath, '\\') + 1;

	FILE *file = fopen(filePath, "rb");
	if (file != NULL)
	{
		fseek(file, 0, SEEK_END);
		DWORD fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		int chunkSize = DROPBOX_FILE_CHUNK_SIZE;
		if (fileSize <= (DROPBOX_FILE_CHUNK_SIZE))
		{
			char *data = new char[fileSize + 1];
			int count = (int)fread(data, sizeof(char), fileSize, file);

			instance->SendFile(fileName, data, fileSize);
		}
		else
		{
			int offset = 0;
			bool error = false;
			char *uploadId = new char[32];

			while (!feof(file) && !ferror(file))
			{
				char *data = new char[chunkSize + 1];
				int count = (int)fread(data, sizeof(char), chunkSize, file);

				if (!offset)
				{
					if (instance->SendFileChunkedFirst(data, count, uploadId, offset))
					{
						error = true;
						break;
					}
				}
				else
				{
					if (instance->SendFileChunkedNext(data, count, uploadId, offset))
					{
						error = true;
						break;
					}
				}
			}

			fclose(file);

			if (!error)
			{
				if (instance->SendFileChunkedLast(fileName, uploadId))
				{
					error = true;
				}
			}
		}
	}

	return 0;
}

INT_PTR CDropbox::SendFilesToDropbox(void *obj, WPARAM hContact, LPARAM)
{
	CDropbox *instance = (CDropbox*)obj;

	instance->hTransferContact = hContact;

	HWND hwnd =  (HWND)CallService(MS_FILE_SENDFILE, instance->GetDefaultContact(), 0);

	instance->dcftp[hwnd] = hContact;

	BBButton bbd = { sizeof(bbd) };
	bbd.pszModuleName = MODULE;
	bbd.dwButtonID = BBB_ID_FILE_SEND;
	bbd.bbbFlags = BBSF_DISABLED;

	CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&bbd);

	return 0;
}