#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

#include "common.h"

struct FileTransferParam
{
	HANDLE hProcess;
	MCONTACT hContact;
	PROTOFILETRANSFERSTATUS pfts;

	int totalFolders;
	wchar_t **pwszFolders;
	int relativePathStart;

	bool withVisualisation;

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
	}
};

#endif //_FILE_TRANSFER_H_