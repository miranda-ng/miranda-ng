#include "common.h"

INT_PTR CDropbox::ProtoGetCaps(WPARAM wParam, LPARAM lParam)
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

INT_PTR CDropbox::ProtoSendFile(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	FileTransferParam *ftp = new FileTransferParam();
	ftp->pfts.flags = PFTS_SENDING | PFTS_UTF;
	ftp->pfts.hContact = pccsd->hContact;

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
	ULONG fileId = InterlockedIncrement(&Singleton<CDropbox>::GetInstance()->hFileProcess);
	ftp->hProcess = (HANDLE)fileId;

	mir_forkthread(CDropbox::SendFileAsync, ftp);

	return fileId;
}

INT_PTR CDropbox::ProtoSendMessage( WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR  CDropbox::RequestApiAuthorization(WPARAM wParam, LPARAM lParam)
{
	mir_forkthread(CDropbox::RequestApiAuthorizationAsync, (void*)wParam);

	return 0;
}

INT_PTR CDropbox::SendFilesToDropbox(WPARAM wParam, LPARAM lParam)
{
	TCHAR filter[128], *pfilter;
	wchar_t buffer[4096] = {0};

	OPENFILENAME ofn = {0};
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	//ofn.hwndOwner = hwndDlg;
	lstrcpy(filter, TranslateT("All files"));
	lstrcat(filter, _T(" (*)"));
	pfilter = filter + lstrlen(filter)+1;
	lstrcpy(pfilter, _T("*"));
	pfilter = filter + lstrlen(filter)+1;
	pfilter[ 0 ] = '\0';
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = 4096;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
	if (GetOpenFileName(&ofn))
	{
		wchar_t **files = NULL;

		if (buffer[ofn.nFileOffset - 1] != '\0')
		{
			// Single-Select
			files = (wchar_t**)mir_alloc(sizeof(wchar_t*) * 2);

			files[0] = mir_wstrdup(buffer);
			files[1] = NULL;
		}
		else
		{
			// Multi-Select
			int i = 0;
			wchar_t *p = buffer;

			while (*p)
			{
				p += lstrlen(p) + 1;
				i++;
			}

			files = (wchar_t**)mir_alloc(sizeof(wchar_t*) * i);
			p = buffer; i = 0;

			while (*p)
			{
				p += lstrlen(p) + 1;
				if (lstrlen(p) > 0)
				{
					int len = lstrlen(buffer) + lstrlen(p) + 1;
					files[i] = (wchar_t*)mir_alloc(sizeof(wchar_t) * len);
					lstrcpy(files[i], buffer);
					lstrcat(files[i], L"\\");
					lstrcat(files[i], p);
					files[i++][len] = '\0';
				}
			}
			files[i] = NULL;
		}

		/*char *cHandle = (char*)wParam;
		char hHandle[16]= { 0 };
		strcat(&hHandle[1], cHandle);*/

		CallContactService(wParam, PSS_FILET, wParam, (LPARAM)files);
	}

	return 0;
}