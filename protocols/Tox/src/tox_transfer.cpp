#include "common.h"

void CToxProto::AddToTransferList(FileTransferParam *transfer)
{
	if (transfers.find(transfer->number) == transfers.end())
	{
		transfers[transfer->number] = transfer;
	}
}

void CToxProto::RemoveFromTransferList(FileTransferParam *transfer)
{
	if (transfers.find(transfer->number) != transfers.end())
	{
		transfers.erase(transfer->number);
		delete transfer;
	}
}

/* FILE RECEIVING */

// incoming file flow
// send it through miranda
void CToxProto::OnFriendFile(Tox *tox, int32_t number, uint8_t fileNumber, uint64_t fileSize, const uint8_t *fileName, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		TCHAR *name = mir_utf8decodeT((char*)fileName);
		if (name == NULL)
		{
			// uTox send file name in ansi
			name = mir_a2u((char*)fileName);
		}

		FileTransferParam *transfer = new FileTransferParam(fileNumber, name, fileSize);
		transfer->pfts.hContact = hContact;
		transfer->pfts.flags |= PFTS_RECEIVING;
		proto->AddToTransferList(transfer);

		PROTORECVFILET pre = { 0 };
		pre.flags = PREF_TCHAR;
		pre.fileCount = 1;
		pre.timestamp = time(NULL);
		pre.tszDescription = _T("");
		pre.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*) * 2);
		pre.ptszFiles[0] = name;
		pre.ptszFiles[1] = NULL;
		pre.lParam = (LPARAM)transfer;
		ProtoChainRecvFile(hContact, &pre);
	}
}

// file request is allowed
HANDLE __cdecl CToxProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszPath)
{
	std::string id = getStringA(hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(id);
	uint32_t number = tox_get_friend_number(tox, clientId.data());

	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	transfer->pfts.tszWorkingDir = mir_tstrdup(tszPath);

	// stupid fix
	TCHAR fullPath[MAX_PATH];
	mir_sntprintf(fullPath, SIZEOF(fullPath), _T("%s\\%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);
	transfer->ChangeFileName(fullPath);

	if (!ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)transfer, (LPARAM)&transfer->pfts))
	{
		transfer->hFile = _tfopen(transfer->pfts.tszCurrentFile, _T("ab"));
		tox_file_send_control(tox, number, 1, transfer->number, TOX_FILECONTROL_ACCEPT, NULL, 0);
	}

	return hTransfer;
}

// if file is exists
int __cdecl CToxProto::FileResume(HANDLE hTransfer, int *action, const PROTOCHAR **szFilename)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;

	std::string id = getStringA(transfer->pfts.hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(id);
	uint32_t number = tox_get_friend_number(tox, clientId.data());

	switch (*action)
	{
	case FILERESUME_RENAME:
		// rename file
		transfer->ChangeFileName(*szFilename);
		transfer->hFile = _tfopen(transfer->pfts.tszCurrentFile, _T("wb"));
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)transfer, 0);
		tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_ACCEPT, NULL, 0);
		break;

	case FILERESUME_OVERWRITE:
		// truncate file to zero
		transfer->hFile = _tfopen(transfer->pfts.tszCurrentFile, _T("wb"));
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)transfer, 0);
		tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_ACCEPT, NULL, 0);
		break;

	case FILERESUME_SKIP:
		// deny file request
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
		tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_KILL, NULL, 0);
		RemoveFromTransferList(transfer);
		break;

	case FILERESUME_RESUME:
		transfer->hFile = _tfopen(transfer->pfts.tszCurrentFile, _T("ab"));
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)transfer, 0);
		tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_ACCEPT, NULL, 0);
		break;
	}

	return 0;
}

// getting the file data
void CToxProto::OnFileData(Tox *tox, int32_t number, uint8_t fileNumber, const uint8_t *data, uint16_t size, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		//mir_cslock lck(proto->toxLock);

		FileTransferParam *transfer = proto->transfers.at(fileNumber);

		if (transfer->hFile == NULL)
		{
			tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_KILL, NULL, 0);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
		}

		if (fwrite(data, sizeof(uint8_t), size, transfer->hFile) == size)
		{
			transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += size;
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
		}
	}
}

/* FILE SENDING */

// outcoming file flow
// send request through tox
HANDLE __cdecl CToxProto::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
{
	std::string id = getStringA(hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(id);
	uint32_t number = tox_get_friend_number(tox, clientId.data());

	TCHAR *fileName = _tcsrchr(ppszFiles[0], '\\') + 1;

	size_t fileDirLength = fileName - ppszFiles[0];
	TCHAR *fileDir = (TCHAR*)mir_alloc(sizeof(TCHAR)*(fileDirLength + 1));
	_tcsncpy(fileDir, ppszFiles[0], fileDirLength);
	fileDir[fileDirLength] = '\0';

	size_t fileSize = 0;
	FILE *file = _tfopen(ppszFiles[0], _T("rb"));
	if (file != NULL)
	{
		fseek(file, 0, SEEK_END);
		fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		fclose(file);
	}

	char *name = mir_utf8encodeW(fileName);
	int fileNumber = tox_new_file_sender(tox, number, fileSize, (uint8_t*)name, strlen(name));
	if (fileNumber < 0)
	{
		debugLogA("CToxProto::SendFilesAsync: cannot send file");
		return NULL;
	}

	FileTransferParam *transfer = new FileTransferParam(fileNumber, fileName, fileSize);
	transfer->pfts.hContact = hContact;
	transfer->pfts.flags |= PFTS_SENDING;
	transfer->pfts.tszWorkingDir = fileDir;
	AddToTransferList(transfer);

	return (HANDLE)transfer;
}

// start sending
void CToxProto::SendFileAsync(void* arg)
{
	FileTransferParam *transfer = (FileTransferParam*)arg;

	std::string id = getStringA(transfer->pfts.hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(id);
	int32_t number = tox_get_friend_number(tox, clientId.data());
	if (number > TOX_ERROR)
	{
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)transfer, 0);

		size_t fileSize = transfer->pfts.currentFileSize;
		size_t fileProgress = 0;
		TCHAR filePath[MAX_PATH];
		mir_sntprintf(filePath, SIZEOF(filePath), _T("%s%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);

		FILE *hFile = _wfopen(filePath, _T("rb"));
		if (!hFile)
		{
			ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
			tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_KILL, NULL, 0);
			return;
		}

		size_t chunkSize = min(fileSize, (size_t)tox_file_data_size(tox, number));
		uint8_t *data = (uint8_t*)mir_alloc(TOX_FILE_BLOCK_SIZE);
		while (!feof(hFile) && fileProgress < fileSize && !transfer->isTerminated)
		{
			// read file by block of 1mb
			size_t blockSize = min(chunkSize * (TOX_FILE_BLOCK_SIZE / chunkSize), fileSize - fileProgress);
			if (fread(data, sizeof(uint8_t), blockSize, hFile) != blockSize)
			{
				fclose(hFile);
				ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
				tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_KILL, NULL, 0);
				return;
			}

			// send block by chunk of chunkSize
			size_t blockProgress = 0;
			while (blockProgress < blockSize && !transfer->isTerminated)
			{
				size_t size = min(chunkSize, blockSize - blockProgress);
				do
				{
					{
						//mir_cslock lck(toxLock);

						if (tox_file_send_data(tox, number, transfer->number, &data[blockProgress], size) != TOX_ERROR)
						{
							break;
						}
					}
					WaitForSingleObject(hToxEvent, INFINITE);
				} while (!transfer->isTerminated);

				blockProgress += size;
				transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = fileProgress += size;
				ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
			}
		}
		fclose(hFile);
		mir_free(data);

		if (!transfer->isTerminated)
		{
			uint8_t receive_send = transfer->GetTransferStatus();
			tox_file_send_control(tox, number, receive_send, transfer->number, TOX_FILECONTROL_FINISHED, NULL, 0);
		}
	}
}

/* GENERAL */

// file request is cancelled
int __cdecl CToxProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	std::string id = getStringA(hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(id);
	uint32_t number = tox_get_friend_number(tox, clientId.data());

	FileTransferParam *transfer = (FileTransferParam*)hTransfer;

	transfer->isTerminated = true;
	tox_file_send_control(tox, number, transfer->GetTransferStatus(), transfer->number, TOX_FILECONTROL_KILL, NULL, 0);
	RemoveFromTransferList(transfer);

	return 0;
}

// file request is denied
int __cdecl CToxProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR*)
{
	return FileCancel(hContact, hTransfer);
}

void CToxProto::OnFileRequest(Tox *tox, int32_t number, uint8_t receive_send, uint8_t fileNumber, uint8_t type, const uint8_t *data, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers.at(fileNumber);

		switch (type)
		{
		case TOX_FILECONTROL_ACCEPT:
			if (receive_send == 1)
			{
				// if friend allow file request
				proto->ForkThread(&CToxProto::SendFileAsync, transfer);
			}
			break;

			// if file transfer is cancelled
		case TOX_FILECONTROL_KILL:
			transfer->isTerminated = true;
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
			proto->RemoveFromTransferList(transfer);
			break;

			// if file transfer is finished
		case TOX_FILECONTROL_FINISHED:
			if (receive_send == 0)
			{
				tox_file_send_control(proto->tox, number, 1, fileNumber, TOX_FILECONTROL_FINISHED, NULL, 0);
			}
			if (transfer->hFile)
			{
				fclose(transfer->hFile);
			}
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)transfer, 0);
			proto->RemoveFromTransferList(transfer);
			break;
		}
	}
}