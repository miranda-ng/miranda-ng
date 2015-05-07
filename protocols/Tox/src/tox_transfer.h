#ifndef _TOX_TRANSFERS_H_
#define _TOX_TRANSFERS_H_

struct FileTransferParam
{
	PROTOFILETRANSFERSTATUS pfts;
	FILE *hFile;
	uint32_t friendNumber;
	uint32_t fileNumber;
	uint64_t transferNumber;

	TOX_FILE_KIND transferType;

	FileTransferParam(uint32_t friendNumber, uint32_t fileNumber, const TCHAR *fileName, uint64_t fileSize)
	{
		hFile = NULL;
		this->friendNumber = friendNumber;
		this->fileNumber = fileNumber;
		transferNumber = (((int64_t)friendNumber) << 32) | ((int64_t)fileNumber);

		pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
		pfts.flags = PFTS_TCHAR;
		pfts.totalFiles = 1;
		pfts.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)*(pfts.totalFiles + 1));
		pfts.ptszFiles[0] = pfts.tszCurrentFile = mir_tstrdup(fileName);
		pfts.ptszFiles[pfts.totalFiles] = NULL;
		pfts.totalBytes = pfts.currentFileSize = fileSize;
		pfts.totalProgress = pfts.currentFileProgress = 0;
		pfts.currentFileNumber = 0;
		pfts.tszWorkingDir = NULL;

		transferType = TOX_FILE_KIND_DATA;
	}

	bool OpenFile(const TCHAR *mode)
	{
		hFile = _tfopen(pfts.tszCurrentFile, mode);
		return hFile != NULL;
	}

	void ChangeName(const TCHAR *fileName)
	{
		pfts.ptszFiles[0] = replaceStrT(pfts.tszCurrentFile, fileName);
	}

	uint8_t GetDirection() const
	{
		return pfts.flags & PFTS_SENDING ? 0 : 1;
	}

	~FileTransferParam()
	{
		if (pfts.tszWorkingDir != NULL)
		{
			mir_free(pfts.tszWorkingDir);
		}
		mir_free(pfts.pszFiles[0]);
		mir_free(pfts.pszFiles);
		if (hFile != NULL)
		{
			fclose(hFile);
			hFile = NULL;
		}
	}
};

struct AvatarTransferParam : public FileTransferParam
{
	uint8_t hash[TOX_HASH_LENGTH];

	AvatarTransferParam(uint32_t friendNumber, uint32_t fileNumber, const TCHAR *fileName, uint64_t fileSize)
		: FileTransferParam(friendNumber, fileNumber, fileName, fileSize)
	{
		transferType = TOX_FILE_KIND_AVATAR;
	}
};

class CTransferList
{
private:
	std::map<int64_t, FileTransferParam*> transfers;

public:
	size_t Count() const
	{
		return transfers.size();
	}

	void Add(FileTransferParam *transfer)
	{
		if (transfers.find(transfer->transferNumber) == transfers.end())
		{
			transfers[transfer->transferNumber] = transfer;
		}
	}

	FileTransferParam* Get(uint32_t friendNumber, uint32_t fileNumber)
	{
		int64_t transferNumber = (((int64_t)friendNumber) << 32) | ((int64_t)fileNumber);
		if (transfers.find(transferNumber) != transfers.end())
		{
			return transfers.at(transferNumber);
		}
		return NULL;
	}

	FileTransferParam* GetAt(size_t index)
	{
		if (index < Count())
		{
			std::map<int64_t, FileTransferParam*>::iterator it = transfers.begin();
			std::advance(it, index);
			return it->second;
		}
		return NULL;
	}

	void Remove(FileTransferParam *transfer)
	{
		if (transfers.find(transfer->transferNumber) != transfers.end())
		{
			transfers.erase(transfer->transferNumber);
			delete transfer;
		}
	}
};

#endif //_TOX_TRANSFERS_H_