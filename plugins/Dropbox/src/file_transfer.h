#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

struct FileTransferParam
{
	HANDLE hProcess;
	MCONTACT hContact;
	PROTOFILETRANSFERSTATUS pfts;

	int totalFolders;
	wchar_t **pwszFolders;
	int relativePathStart;

	wchar_t **pwszUrls;

	FileTransferParam()
	{
		totalFolders = 0;
		pwszFolders = NULL;
		relativePathStart = 0;

		pfts.cbSize = sizeof(this->pfts);
		pfts.flags = PFTS_UNICODE;
		pfts.currentFileNumber = 0;
		pfts.currentFileProgress = 0;
		pfts.currentFileSize = 0;
		pfts.totalBytes = 0;
		pfts.totalFiles = 0;
		pfts.totalProgress = 0;
		pfts.pszFiles = NULL;
		pfts.tszWorkingDir = NULL;
		pfts.wszCurrentFile = NULL;

		pwszUrls = NULL;
	}

	~FileTransferParam()
	{
		if (pfts.wszWorkingDir)
			mir_free(pfts.wszWorkingDir);

		if (pfts.pszFiles)
		{
			for (int i = 0; pfts.pszFiles[i]; i++)
			{
				if (pfts.pszFiles[i]) mir_free(pfts.pszFiles[i]);
			}
			mir_free(pfts.pszFiles);
		}

		if (pwszFolders)
		{
			for (int i = 0; pwszFolders[i]; i++)
			{
				if (pwszFolders[i]) mir_free(pwszFolders[i]);
			}
			mir_free(pwszFolders);
		}

		if (pwszUrls)
		{
			for (int i = 0; pwszUrls[i]; i++)
			{
				if (pwszUrls[i]) mir_free(pwszUrls[i]);
			}
			mir_free(pwszUrls);
		}
	}

	void AddUrl(const wchar_t *url)
	{
		int count = 0;
		if (pwszUrls == NULL)
			pwszUrls = (wchar_t**)mir_alloc(sizeof(wchar_t*) * 2);
		else
		{
			for (; pwszUrls[count]; count++);
			pwszUrls = (wchar_t**)mir_realloc(pwszUrls, sizeof(wchar_t*) * (count + 2));
		}
		
		size_t length = wcslen(url);
		pwszUrls[count] = (wchar_t*)mir_alloc(sizeof(wchar_t) * (length + 1));
		wcscpy(pwszUrls[count], url);
		pwszUrls[count][length] = '\0';
		pwszUrls[count + 1] = NULL;
	}
};

#endif //_FILE_TRANSFER_H_