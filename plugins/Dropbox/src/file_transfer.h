#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

#include "common.h"

struct FileTransferParam
{
	HANDLE hProcess;
	MCONTACT hContact;
	CDropbox *instance;
	PROTOFILETRANSFERSTATUS pfts;

	int totalFolders;
	char **pszFolders;
	int relativePathStart;

	FileTransferParam(CDropbox *instance)
	{
		this->instance = instance;

		totalFolders = 0;
		pszFolders = NULL;
		relativePathStart = 0;

		pfts.cbSize = sizeof(this->pfts);
		pfts.flags = PFTS_UTF;
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
		if (pfts.pszFiles)
		{
			for (int i = 0; pfts.pszFiles[i]; i++)
			{
				if (pfts.pszFiles[i]) mir_free(pfts.pszFiles[i]);
			}
			delete pfts.pszFiles;
		}

		if (pszFolders)
		{
			for (int i = 0; pszFolders[i]; i++)
			{
				if (pszFolders[i]) mir_free(pszFolders[i]);
			}
			delete pszFolders;
		}
	}
};

#endif //_FILE_TRANSFER_H_