#ifndef _TOX_TRANSFERS_H_
#define _TOX_TRANSFERS_H_

enum FILE_TRANSFER_STATUS
{
	NONE,
	STARTED,
	PAUSED,
	BROKEN,
	FAILED,
	CANCELED,
	FINISHED,
	DESTROYED
};

struct FileTransferParam
{
	PROTOFILETRANSFERSTATUS pfts;
	FILE_TRANSFER_STATUS status;
	FILE *hFile;
	int32_t friendNumber;
	uint8_t fileNumber;
	int64_t transferNumber;

	FileTransferParam(int32_t friendNumber, uint8_t fileNumber, const TCHAR *fileName, uint64_t fileSize)
	{
		status = NONE;
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
		status = DESTROYED;
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

	FileTransferParam* Get(int32_t friendNumber, uint8_t fileNumber)
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

	void Remove(int32_t friendNumber, uint8_t fileNumber)
	{
		int64_t transferNumber = (((int64_t)friendNumber) << 32) | ((int64_t)fileNumber);
		if (transfers.find(transferNumber) != transfers.end())
		{
			FileTransferParam *transfer = transfers.at(transferNumber);
			transfers.erase(transferNumber);
			delete transfer;
		}
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