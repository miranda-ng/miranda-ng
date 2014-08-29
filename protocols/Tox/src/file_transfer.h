#ifndef _FILE_TRANSFER_H_
#define _FILE_TRANSFER_H_

class CFileTransfer;

class CFile
{
private:
	int number;
	char *name;
	const TCHAR *path;
	size_t size;

	const CFileTransfer *transfer;

public:
	CFile(const CFileTransfer *fileTransfer, const TCHAR *filePath, size_t fileSize) :
		number(-1)
	{
		transfer = fileTransfer;

		path = filePath;
		name = strrchr(ptrA(mir_utf8encodeT(path)), '\\');
		size = fileSize;
	}

	CFile(int number) : number(number), name(NULL) { }

	~CFile()
	{
		number = -1;
		if (name != NULL)
		{
			mir_free(name);
		}
	}

	const CFileTransfer *GetTransfer() const
	{
		return this->transfer;
	}

	void SetNumber(int fileNumber)
	{
		number = fileNumber;
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
	ULONG number;

	HANDLE hWait;
	LIST<CFile> files;
	PROTOFILETRANSFERSTATUS pfts;
	const PROTO_INTERFACE *proto;

public:
	CFileTransfer(const PROTO_INTERFACE *proto, MCONTACT hContact, ULONG transferNumber, DWORD flags) :
		files(1, NumericKeySortT)
	{
		this->proto = proto;

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

		number = transferNumber;
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

			files.insert(new CFile(this, ppszFiles[i], fileSize));
		}
	}

	const PROTO_INTERFACE *GetProtoInstance() const
	{
		return proto;
	}

	ULONG GetTransferNumber() const
	{
		return number;
	}

	MCONTACT GetContactHandle() const
	{
		return pfts.hContact;
	}

	int GetFileCount() const
	{
		return pfts.totalFiles;
	}

	CFile *GetFileAt(int idx) const
	{
		return files[idx];
	}

	CFile *GetFileByNumber(int number) const
	{
		CFile *search = new CFile(number);
		CFile *file = files.find(search);
		delete search;

		return file;
	}

	bool HasFile(int number) const
	{
		const CFile *file = GetFileByNumber(number);

		return file != NULL;
	}

	void Wait()
	{
		WaitForSingleObject(hWait, INFINITE);
	}
};

class CFileSendTransfer : public CFileTransfer
{
public:
	CFileSendTransfer(MCONTACT hContact, ULONG hProcess) : CFileTransfer(NULL, hContact, hProcess, PFTS_SENDING)
	{
	}
};

#endif //_FILE_TRANSFER_H_