#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

struct FileTransferParam
{
	FILE *hFile;
	HANDLE hProcess;
	MCONTACT hContact;
	PROTOFILETRANSFERSTATUS pfts;

	bool isTerminated;

	int relativePathStart;

	LIST<char> urls;

	TCHAR *description;

	FileTransferParam() : urls(1)
	{
		hFile = NULL;
		hProcess = NULL;
		hContact = NULL;

		isTerminated = false;

		relativePathStart = 0;

		pfts.cbSize = sizeof(this->pfts);
		pfts.flags = PFTS_TCHAR | PFTS_SENDING;
		pfts.hContact = NULL;
		pfts.currentFileNumber = -1;
		pfts.currentFileProgress = 0;
		pfts.currentFileSize = 0;
		pfts.currentFileTime = 0;
		pfts.totalBytes = 0;
		pfts.totalFiles = 0;
		pfts.totalProgress = 0;
		pfts.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*) * (pfts.totalFiles + 1));
		pfts.ptszFiles[pfts.totalFiles] = NULL;
		pfts.tszWorkingDir = NULL;
		pfts.tszCurrentFile = NULL;
		
		description = NULL;

		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, hProcess, 0);
	}

	~FileTransferParam()
	{
		CloseCurrentFile();

		if (pfts.tszWorkingDir)
			mir_free(pfts.tszWorkingDir);

		if (pfts.pszFiles)
		{
			for (int i = 0; pfts.pszFiles[i]; i++)
			{
				if (pfts.pszFiles[i]) mir_free(pfts.pszFiles[i]);
			}
			mir_free(pfts.pszFiles);
		}

		for (int i = 0; i < urls.getCount(); i++)
			mir_free(urls[i]);
		urls.destroy();

		if (description)
			mir_free(description);
	}

	void SetDescription(const TCHAR *text)
	{
		if (text[0] == 0)
			return;
		description = mir_tstrdup(text);
	}

	void SetWorkingDirectory(const TCHAR *path)
	{
		relativePathStart = _tcsrchr(path, '\\') - path + 1;
		pfts.tszWorkingDir = (TCHAR*)mir_alloc(sizeof(TCHAR) * relativePathStart);
		mir_tstrncpy(pfts.tszWorkingDir, path, relativePathStart);
		pfts.tszWorkingDir[relativePathStart - 1] = '\0';
	}

	void AddFile(const TCHAR *path)
	{
		pfts.ptszFiles = (TCHAR**)mir_realloc(pfts.ptszFiles, sizeof(TCHAR*) * (pfts.totalFiles + 2));
		pfts.ptszFiles[pfts.totalFiles++] = mir_tstrdup(path);
		pfts.ptszFiles[pfts.totalFiles] = NULL;

		FILE *hFile = _tfopen(path, L"rb");
		if (hFile != NULL) {
			_fseeki64(hFile, 0, SEEK_END);
			pfts.totalBytes += _ftelli64(hFile);
			fclose(hFile);
		}
	}

	void AddUrl(const char *url)
	{
		urls.insert(mir_strdup(url));
	}

	const TCHAR* GetCurrentFilePath() const
	{
		return pfts.ptszFiles[pfts.currentFileNumber];
	}

	const TCHAR* GetCurrentFileName() const
	{
		return _tcsrchr(pfts.ptszFiles[pfts.currentFileNumber], '\\') + 1;
	}

	void OpenCurrentFile()
	{
		hFile = _tfopen(GetCurrentFilePath(), _T("rb"));
		if (!hFile)
			throw DropboxException("Unable to open file");
		_fseeki64(hFile, 0, SEEK_END);
		pfts.currentFileSize = _ftelli64(hFile);
		rewind(hFile);
	}

	size_t ReadCurrentFile(void *data, size_t count)
	{
		return fread(data, sizeof(char), count, hFile);
	}
	
	void CheckCurrentFile()
	{
		if (ferror(hFile))
			throw DropboxException("Error while file sending");

		if (isTerminated)
			throw DropboxException("Transfer was terminated");
	}

	void CloseCurrentFile()
	{
		if (hFile != NULL)
		{
			fclose(hFile);
			hFile = NULL;
		}
	}

	const uint64_t GetCurrentFileSize() const
	{
		return pfts.currentFileSize;
	}

	const uint64_t GetCurrentFileChunkSize() const
	{
		int chunkSize = 1024 * 1024;
		if (pfts.currentFileSize < chunkSize)
			chunkSize = min(pfts.currentFileSize, chunkSize / 4);
		else if (pfts.currentFileSize > 20 * chunkSize)
			chunkSize = chunkSize * 4;
		return chunkSize;
	}

	void Progress(size_t count)
	{
		pfts.currentFileProgress += count;
		pfts.totalProgress += count;
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, hProcess, (LPARAM)&pfts);
	}

	void FirstFile()
	{
		CloseCurrentFile();

		pfts.currentFileNumber = 0;
		pfts.currentFileProgress = 0;
		pfts.tszCurrentFile = _tcsrchr(pfts.ptszFiles[pfts.currentFileNumber], '\\') + 1;
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, hProcess, (LPARAM)&pfts);

		OpenCurrentFile();
		CheckCurrentFile();
	}

	bool NextFile()
	{
		CloseCurrentFile();

		if (++pfts.currentFileNumber == pfts.totalFiles)
			return false;

		pfts.currentFileProgress = 0;
		pfts.tszCurrentFile = _tcsrchr(pfts.ptszFiles[pfts.currentFileNumber], '\\') + 1;
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, hProcess, 0);

		OpenCurrentFile();
		CheckCurrentFile();

		return true;
	}

	void SetStatus(int status, LPARAM param = 0)
	{
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, status, hProcess, param);
	}
};

#endif //_FILE_TRANSFER_H_