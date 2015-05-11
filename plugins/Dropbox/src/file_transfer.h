#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

class TransferException
{
	CMStringA message;

public:
	TransferException(const char *message) :
		message(message)
	{
	}

	const char* what() const throw()
	{
		return message.c_str();
	}
};

struct FileTransferParam
{
	HANDLE hProcess;
	MCONTACT hContact;
	PROTOFILETRANSFERSTATUS pfts;

	int totalFolders;
	TCHAR **pwszFolders;
	int relativePathStart;

	LIST<char> urlList;

	FileTransferParam() : urlList(1)
	{
		totalFolders = 0;
		pwszFolders = NULL;
		relativePathStart = 0;

		pfts.cbSize = sizeof(this->pfts);
		pfts.flags = PFTS_TCHAR;
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

		for (int i = 0; i < urlList.getCount(); i++)
			mir_free(urlList[i]);
		urlList.destroy();
	}

	void AddUrl(const char *url)
	{
		urlList.insert(mir_strdup(url));
	}
};

#endif //_FILE_TRANSFER_H_