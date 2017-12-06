#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

class FileTransferParam
{
private:
	static ULONG hFileProcess;

	ULONG id;
	FILE *hFile;
	PROTOFILETRANSFERSTATUS pfts;

	bool isTerminated;

	const wchar_t* folderName;
	int relativePathStart;
	
	CMStringW data;

public:
	FileTransferParam(MCONTACT hContact)
	{
		hFile = NULL;
		id = InterlockedIncrement(&hFileProcess);

		isTerminated = false;

		folderName = NULL;
		relativePathStart = 0;

		pfts.cbSize = sizeof(this->pfts);
		pfts.flags = PFTS_UNICODE | PFTS_SENDING;
		pfts.hContact = hContact;
		pfts.currentFileNumber = -1;
		pfts.currentFileProgress = 0;
		pfts.currentFileSize = 0;
		pfts.currentFileTime = 0;
		pfts.totalBytes = 0;
		pfts.totalFiles = 0;
		pfts.totalProgress = 0;
		pfts.ptszFiles = (wchar_t**)mir_alloc(sizeof(wchar_t*) * (pfts.totalFiles + 1));
		pfts.ptszFiles[pfts.totalFiles] = NULL;
		pfts.tszWorkingDir = NULL;
		pfts.tszCurrentFile = NULL;

		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)id, 0);
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
	}

	ULONG GetId() const
	{
		return id;
	}

	MCONTACT GetHContact() const
	{
		return pfts.hContact;
	}

	const wchar_t* GetData() const
	{
		if (data.IsEmpty())
			return NULL;
		return data;
	}

	void Terminate()
	{
		isTerminated = true;
	}

	void SetWorkingDirectory(const wchar_t *path)
	{
		relativePathStart = wcsrchr(path, '\\') - path + 1;
		pfts.tszWorkingDir = (wchar_t*)mir_calloc(sizeof(wchar_t) * relativePathStart);
		mir_wstrncpy(pfts.tszWorkingDir, path, relativePathStart);
		if (PathIsDirectory(path))
			folderName = wcsrchr(path, '\\') + 1;
	}

	const wchar_t* IsFolder() const
	{
		return folderName;
	}

	const wchar_t* GetFolderName() const
	{
		return folderName;
	}

	void AddFile(const wchar_t *path)
	{
		pfts.ptszFiles = (wchar_t**)mir_realloc(pfts.ptszFiles, sizeof(wchar_t*) * (pfts.totalFiles + 2));
		pfts.ptszFiles[pfts.totalFiles++] = mir_wstrdup(path);
		pfts.ptszFiles[pfts.totalFiles] = NULL;

		FILE *file = _wfopen(path, L"rb");
		if (file != NULL) {
			_fseeki64(file, 0, SEEK_END);
			pfts.totalBytes += _ftelli64(file);
			fclose(file);
		}
	}

	void AppendFormatData(const wchar_t *format, ...)
	{
		va_list args;
		va_start(args, format);
		data.AppendFormatV(format, args);
		va_end(args);
	}

	const wchar_t* GetCurrentFilePath() const
	{
		return pfts.ptszFiles[pfts.currentFileNumber];
	}

	const wchar_t* GetCurrentRelativeFilePath() const
	{
		return &GetCurrentFilePath()[relativePathStart];
	}

	const wchar_t* GetCurrentFileName() const
	{
		return wcsrchr(pfts.ptszFiles[pfts.currentFileNumber], '\\') + 1;
	}

	void OpenCurrentFile()
	{
		hFile = _wfopen(GetCurrentFilePath(), L"rb");
		if (!hFile)
			throw Exception("Unable to open file");
		_fseeki64(hFile, 0, SEEK_END);
		pfts.currentFileSize = _ftelli64(hFile);
		rewind(hFile);
	}

	size_t ReadCurrentFile(void *buffer, size_t count)
	{
		return fread(buffer, sizeof(char), count, hFile);
	}
	
	void CheckCurrentFile()
	{
		if (ferror(hFile))
			throw Exception("Error while file sending");

		if (isTerminated)
			throw Exception("Transfer was terminated");
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

	const size_t GetCurrentFileChunkSize() const
	{
		size_t chunkSize = 1024 * 1024;
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
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&pfts);
	}

	void FirstFile()
	{
		CloseCurrentFile();

		pfts.currentFileNumber = 0;
		pfts.currentFileProgress = 0;
		pfts.tszCurrentFile = wcsrchr(pfts.ptszFiles[pfts.currentFileNumber], '\\') + 1;
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&pfts);

		OpenCurrentFile();
		CheckCurrentFile();
	}

	bool NextFile()
	{
		CloseCurrentFile();

		if (++pfts.currentFileNumber == pfts.totalFiles)
			return false;

		pfts.currentFileProgress = 0;
		pfts.tszCurrentFile = wcsrchr(pfts.ptszFiles[pfts.currentFileNumber], '\\') + 1;
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)id, 0);

		OpenCurrentFile();
		CheckCurrentFile();

		return true;
	}

	void SetStatus(int status, LPARAM param = 0)
	{
		ProtoBroadcastAck(MODULE, pfts.hContact, ACKTYPE_FILE, status, (HANDLE)id, param);
	}
};

#endif //_FILE_TRANSFER_H_