#ifndef _TOX_TRANSFERS_H_
#define _TOX_TRANSFERS_H_

enum FILE_TRANSFER_STATUS
{
	NONE,
	STARTED,
	PAUSED,
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
	int friendNumber;
	int fileNumber;

	FileTransferParam(int friendNumber, int fileNumber, const TCHAR* fileName, size_t fileSize)
	{
		status = NONE;
		hFile = NULL;
		this->friendNumber = friendNumber;
		this->fileNumber = fileNumber;

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

	void Start(Tox *tox)
	{
		status = STARTED;
		tox_file_send_control(tox, friendNumber, GetDirection(), fileNumber, TOX_FILECONTROL_ACCEPT, NULL, 0);
	}

	void Resume(Tox *tox)
	{
		status = STARTED;
		tox_file_send_control(tox, friendNumber, GetDirection(), fileNumber, TOX_FILECONTROL_RESUME_BROKEN, (uint8_t*)&pfts.currentFileProgress, sizeof(uint64_t));
	}

	void Fail(Tox *tox)
	{
		status = FAILED;
		tox_file_send_control(tox, friendNumber, GetDirection(), fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
	}

	void Cancel(Tox *tox)
	{
		status = FINISHED;
		tox_file_send_control(tox, friendNumber, GetDirection(), fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
	}

	void Finish(Tox *tox)
	{
		status = FINISHED;
		tox_file_send_control(tox, friendNumber, GetDirection(), fileNumber, TOX_FILECONTROL_FINISHED, NULL, 0);
	}

	void RenameName(const TCHAR* fileName)
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
		if (hFile)
		{
			fclose(hFile);
		}
	}
};

class CTransferList
{
private:
	std::map<uint8_t, FileTransferParam*> transfers;

public:
	int Count() const
	{
		return transfers.size();
	}

	void Add(FileTransferParam *transfer)
	{
		if (transfers.find(transfer->fileNumber) == transfers.end())
		{
			transfers[transfer->fileNumber] = transfer;
		}
	}

	FileTransferParam * Get(uint8_t fileNumber)
	{
		if (transfers.find(fileNumber) != transfers.end())
		{
			return transfers.at(fileNumber);
		}
		return NULL;
	}

	FileTransferParam * At(int index)
	{
		if (Count() < index)
		{
			std::map<uint8_t, FileTransferParam*>::iterator it = transfers.begin();
			std::advance(it, index);
			return it->second;
		}
		return NULL;
	}

	void Remove(uint8_t fileNumber)
	{
		if (transfers.find(fileNumber) != transfers.end())
		{
			FileTransferParam *transfer = transfers.at(fileNumber);
			transfers.erase(fileNumber);
			delete transfer;
		}
	}

	void Remove(FileTransferParam *transfer)
	{
		if (transfers.find(transfer->fileNumber) != transfers.end())
		{
			transfers.erase(transfer->fileNumber);
			delete transfer;
		}
	}
};

#endif //_TOX_TRANSFERS_H_