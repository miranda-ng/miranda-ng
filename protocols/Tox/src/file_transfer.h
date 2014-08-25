#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

class CFileTransfer;

class CFile
{
private:
	HANDLE hProcess;

	const CFileTransfer *transfer;
	
	char *name;
	const TCHAR *path;
	size_t size;

public:
	CFile(const CFileTransfer *fileTransfer, const TCHAR *filePath, size_t fileSize)
	{
		transfer = fileTransfer;

		path = filePath;
		name = strrchr(ptrA(mir_utf8encodeT(path)), '\\');
		size = fileSize;
	}

	~CFile()
	{
		mir_free(name);
	}

	void SetHandle(HANDLE hFileProcess)
	{
		hProcess = hFileProcess;
	}

	const TCHAR* GetPath() const
	{
		return path;
	}

	const char* GetName() const
	{
		return name;
	}

	size_t GetSize() const
	{
		return size;
	}
};

class CFileTransfer
{
private:
	HANDLE hProcess;
	LIST<CFile> files;

public:
	PROTOFILETRANSFERSTATUS pfts;

	CFileTransfer(MCONTACT hContact, ULONG hProcess, DWORD flags) :
		files(1)
	{
		pfts.cbSize = sizeof(pfts);
		pfts.flags = PFTS_TCHAR | flags;
		pfts.hContact = hContact;
		pfts.currentFileNumber = 0;
		pfts.currentFileProgress = 0;
		pfts.currentFileSize = 0;
		pfts.totalBytes = 0;
		pfts.totalFiles = 0;
		pfts.totalProgress = 0;
		pfts.pszFiles = NULL;
		pfts.tszWorkingDir = NULL;
		pfts.tszCurrentFile = NULL;

		this->hProcess = (HANDLE)hProcess;
	}

	~CFileTransfer()
	{
		if (pfts.tszWorkingDir)
			mir_free(pfts.tszWorkingDir);
		if (pfts.ptszFiles)
		{
			for (int i = 0; pfts.ptszFiles[i]; i++)
			{
				if (pfts.ptszFiles[i]) mir_free(pfts.ptszFiles[i]);
			}
			mir_free(pfts.ptszFiles);
		}
	}

	void ProcessTransferedFiles(TCHAR** ppszFiles)
	{
		for (pfts.totalFiles = 0; ppszFiles[pfts.totalFiles]; pfts.totalFiles++);
		pfts.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)*(pfts.totalFiles + 1));
		pfts.ptszFiles[pfts.totalFiles] = NULL;
		for (int i = 0; ppszFiles[i]; i++)
		{
			if (!pfts.tszWorkingDir)
			{
				wchar_t *path = ppszFiles[i];
				int length = wcsrchr(path, '\\') - path;
				pfts.tszWorkingDir = (TCHAR*)mir_alloc(sizeof(TCHAR)*(length + 1));
				lstrcpyn(pfts.tszWorkingDir, ppszFiles[i], length + 1);
				pfts.tszWorkingDir[length] = '\0';
			}

			pfts.ptszFiles[i] = mir_tstrdup(ppszFiles[i]);

			size_t fileSize = 0;
			FILE *file = _tfopen(ppszFiles[i], _T("rb"));
			if (file != NULL)
			{
				fseek(file, 0, SEEK_END);
				pfts.totalBytes += fileSize = ftell(file);
				fseek(file, 0, SEEK_SET);
				fclose(file);
			}

			files.insert(new CFile(this, pfts.ptszFiles[i], fileSize));
		}
	}

	int GetFileCount() const
	{
		return files.getCount();
	}

	CFile * const GetFileAt(int idx) const
	{
		return files[idx];
	}

	HANDLE GetTransferHandler() const
	{
		return hProcess;
	}
};

class CFileSendTransfer : public CFileTransfer
{
public:
	CFileSendTransfer(MCONTACT hContact, ULONG hProcess) : CFileTransfer(hContact, hProcess, PFTS_SENDING)
	{
	}
};

#endif //_FILE_TRANSFER_H_