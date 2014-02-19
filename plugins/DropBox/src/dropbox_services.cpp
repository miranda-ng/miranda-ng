#include "dropbox.h"

INT_PTR CDropbox::GetCaps(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND | PF1_AUTHREQ;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_FORCEAUTH;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)MODULE " ID";
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"uid";
	}

	return 0;
}

INT_PTR CDropbox::SendFile(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	FileTransferParam *ftp = new FileTransferParam();
	ftp->pfts.flags = PFTS_SENDING | PFTS_UTF;
	ftp->pfts.hContact = pccsd->hContact;

	char **files = (char**)pccsd->lParam;

	for (int i = 0; files[i]; i++)
	{
		if (PathIsDirectoryA(files[i]))
			continue;
		ftp->pfts.totalFiles++;
	}

	ftp->pfts.pszFiles = new char*[ftp->pfts.totalFiles + 1];
	ftp->pfts.pszFiles[ftp->pfts.totalFiles] = NULL;
	for (int i = 0, j = 0; files[i]; i++)
	{
		if (PathIsDirectoryA(files[i]))
			continue;

		ftp->pfts.pszFiles[j] = mir_strdup(files[i]);

		FILE *file = fopen(files[j], "rb");
		if (file != NULL)
		{
			fseek(file, 0, SEEK_END);
			ftp->pfts.totalBytes += ftell(file);
			fseek(file, 0, SEEK_SET);
			fclose(file);
		}

		j++;
	}
	ULONG fileId = InterlockedIncrement(&g_dropbox->hFileProcess);
	ftp->hProcess = (HANDLE)fileId;

	mir_forkthread(CDropbox::SendFileAsync, ftp);

	return fileId;
}

INT_PTR CDropbox::SendMessage( WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR  CDropbox::RequeriedApiAccess(WPARAM wParam, LPARAM lParam)
{
	int result = MessageBox(NULL, TranslateT("Are you sure you want to requeried access?"), TranslateT("Requeried access"), MB_YESNO | MB_ICONQUESTION);
	if (result == IDYES  && g_dropbox->HasAccessToken())
	{
		g_dropbox->DestroyAcceessToken();
		g_dropbox->RequestAcceessToken();
	}

	return 0;
}