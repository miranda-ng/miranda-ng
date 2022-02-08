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

	CMStringW m_serverDirectory;
	int m_relativePathStart;

	LIST<char> m_links;
	CMStringW m_description;

public:
	FileTransferParam(MCONTACT hContact)
		: m_links(1)
	{
		hFile = NULL;
		id = InterlockedIncrement(&hFileProcess);

		isTerminated = false;

		m_relativePathStart = 0;

		pfts.flags = PFTS_UNICODE | PFTS_SENDING;
		pfts.hContact = hContact;
		pfts.currentFileNumber = -1;
		pfts.currentFileProgress = 0;
		pfts.currentFileSize = 0;
		pfts.currentFileTime = 0;
		pfts.totalBytes = 0;
		pfts.totalFiles = 0;
		pfts.totalProgress = 0;
		pfts.pszFiles.w = (wchar_t**)mir_alloc(sizeof(wchar_t*) * (pfts.totalFiles + 1));
		pfts.pszFiles.w[pfts.totalFiles] = NULL;
		pfts.szWorkingDir.w = NULL;
		pfts.szCurrentFile.w = NULL;

		ProtoBroadcastAck(MODULENAME, pfts.hContact, ACKTYPE_FILE, ACKRESULT_INITIALISING, (HANDLE)id);
	}

	~FileTransferParam()
	{
		CloseCurrentFile();

		if (pfts.szWorkingDir.w)
			mir_free(pfts.szWorkingDir.w);

		if (pfts.pszFiles.a) {
			for (int i = 0; pfts.pszFiles.a[i]; i++)
				mir_free(pfts.pszFiles.a[i]);
			mir_free(pfts.pszFiles.a);
		}

		for (auto &link : m_links)
			mir_free(link);
		m_links.destroy();
	}

	ULONG GetId() const
	{
		return id;
	}

	MCONTACT GetContact() const
	{
		return pfts.hContact;
	}

	const wchar_t* GetDescription() const
	{
		return m_description.GetString();
	}

	const char** GetSharedLinks(size_t &count) const
	{
		count = m_links.getCount();
		return (const char**)m_links.getArray();
	}

	void Terminate()
	{
		isTerminated = true;
	}

	void SetDescription(const wchar_t *description)
	{
		m_description = description;
	}

	void SetWorkingDirectory(const wchar_t *path)
	{
		m_relativePathStart = wcsrchr(path, '\\') - path + 1;
		pfts.szWorkingDir.w = (wchar_t*)mir_calloc(sizeof(wchar_t) * m_relativePathStart);
		mir_wstrncpy(pfts.szWorkingDir.w, path, m_relativePathStart);
		if (PathIsDirectory(path))
			m_serverDirectory = wcsrchr(path, '\\') + 1;
	}

	void SetServerDirectory(const wchar_t *name)
	{
		if (name)
			m_serverDirectory = name;
	}

	const wchar_t* GetServerDirectory() const
	{
		if (m_serverDirectory.IsEmpty())
			return nullptr;
		return m_serverDirectory.GetString();
	}

	void AddFile(const wchar_t *path)
	{
		pfts.pszFiles.w = (wchar_t**)mir_realloc(pfts.pszFiles.w, sizeof(wchar_t*) * (pfts.totalFiles + 2));
		pfts.pszFiles.w[pfts.totalFiles++] = mir_wstrdup(path);
		pfts.pszFiles.w[pfts.totalFiles] = NULL;

		FILE *file = _wfopen(path, L"rb");
		if (file != NULL) {
			_fseeki64(file, 0, SEEK_END);
			pfts.totalBytes += _ftelli64(file);
			fclose(file);
		}
	}

	void AddSharedLink(const char *url)
	{
		m_links.insert(mir_strdup(url));
	}

	const bool IsCurrentFileInSubDirectory() const
	{
		const wchar_t *backslash = wcschr(GetCurrentRelativeFilePath(), L'\\');
		return backslash != nullptr;
	}

	const wchar_t* GetCurrentFilePath() const
	{
		return pfts.pszFiles.w[pfts.currentFileNumber];
	}

	const wchar_t* GetCurrentRelativeFilePath() const
	{
		return &GetCurrentFilePath()[m_relativePathStart];
	}

	const wchar_t* GetCurrentFileName() const
	{
		return wcsrchr(GetCurrentFilePath(), '\\') + 1;
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
		if (pfts.hContact)
			ProtoBroadcastAck(MODULENAME, pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&pfts);
	}

	void FirstFile()
	{
		CloseCurrentFile();

		pfts.currentFileNumber = 0;
		pfts.currentFileProgress = 0;
		pfts.szCurrentFile.w = wcsrchr(pfts.pszFiles.w[pfts.currentFileNumber], '\\') + 1;
		if (pfts.hContact)
			ProtoBroadcastAck(MODULENAME, pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)id, (LPARAM)&pfts);

		OpenCurrentFile();
		CheckCurrentFile();
	}

	bool NextFile()
	{
		CloseCurrentFile();

		if (++pfts.currentFileNumber == pfts.totalFiles)
			return false;

		pfts.currentFileProgress = 0;
		pfts.szCurrentFile.w = wcsrchr(pfts.pszFiles.w[pfts.currentFileNumber], '\\') + 1;
		if (pfts.hContact)
			ProtoBroadcastAck(MODULENAME, pfts.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, (HANDLE)id);

		OpenCurrentFile();
		CheckCurrentFile();

		return true;
	}

	void SetStatus(int status, LPARAM param = 0)
	{
		if (pfts.hContact)
			ProtoBroadcastAck(MODULENAME, pfts.hContact, ACKTYPE_FILE, status, (HANDLE)id, param);
	}
};

#endif //_FILE_TRANSFER_H_