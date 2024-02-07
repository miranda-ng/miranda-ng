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

	FileTransferParam(uint32_t friendNumber, uint32_t fileNumber, const char *fileName, uint64_t fileSize)
	{
		hFile = nullptr;
		this->friendNumber = friendNumber;
		this->fileNumber = fileNumber;
		transferNumber = (((int64_t)friendNumber) << 32) | ((int64_t)fileNumber);

		pfts.flags = PFTS_UNICODE;
		pfts.hContact = NULL;
		pfts.totalFiles = 1;
		pfts.pszFiles.w = (wchar_t**)mir_alloc(sizeof(wchar_t*)*(pfts.totalFiles + 1));
		pfts.pszFiles.w[0] = pfts.szCurrentFile.w = mir_utf8decodeW(fileName);
		pfts.pszFiles.w[pfts.totalFiles] = nullptr;
		pfts.totalBytes = pfts.currentFileSize = fileSize;
		pfts.totalProgress = pfts.currentFileProgress = 0;
		pfts.currentFileNumber = 0;
		pfts.currentFileTime = now();
		pfts.szWorkingDir.w = nullptr;

		transferType = TOX_FILE_KIND_DATA;
	}

	~FileTransferParam()
	{
		mir_free(pfts.szWorkingDir.w);
		mir_free(pfts.pszFiles.w[0]);
		mir_free(pfts.pszFiles.w);
		if (hFile) {
			fclose(hFile);
			hFile = nullptr;
		}
	}

	bool Resume()
	{
		if (hFile)
			return true;
		hFile = _wfopen(pfts.szCurrentFile.w, L"wb+");
		if (hFile)
			_chsize_s(_fileno(hFile), pfts.currentFileSize);
		return hFile != nullptr;
	}

	void Pause()
	{
		if (hFile) {
			fclose(hFile);
			hFile = nullptr;
		}
	}

	void ChangeName(const wchar_t *fileName)
	{
		pfts.pszFiles.w[0] = replaceStrW(pfts.szCurrentFile.w, fileName);
	}

	uint8_t GetDirection() const
	{
		return (pfts.flags & PFTS_SENDING) ? 0 : 1;
	}
};

struct AvatarTransferParam : public FileTransferParam
{
	uint8_t hash[TOX_HASH_LENGTH];

	AvatarTransferParam(uint32_t friendNumber, uint32_t fileNumber, const char *fileName, uint64_t fileSize)
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
			transfers[transfer->transferNumber] = transfer;
	}

	FileTransferParam* Get(uint32_t friendNumber, uint32_t fileNumber)
	{
		int64_t transferNumber = (((int64_t)friendNumber) << 32) | ((int64_t)fileNumber);
		if (transfers.find(transferNumber) != transfers.end())
			return transfers.at(transferNumber);
		return nullptr;
	}

	FileTransferParam* GetAt(size_t index)
	{
		if (index < Count()) {
			auto it = transfers.begin();
			std::advance(it, index);
			return it->second;
		}
		return nullptr;
	}

	void Remove(FileTransferParam *transfer)
	{
		if (transfer == nullptr)
			return;
		if (transfers.erase(transfer->transferNumber))
			delete transfer;
	}
};

#endif //_TOX_TRANSFERS_H_