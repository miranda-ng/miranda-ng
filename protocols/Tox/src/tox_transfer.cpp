#include "common.h"

/* FILE RECEIVING */

// incoming file flow
// send it through miranda
void CToxProto::OnFriendFile(Tox *tox, int32_t friendNumber, uint8_t fileNumber, uint64_t fileSize, const uint8_t *fileName, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(friendNumber);
	if (hContact)
	{
		TCHAR *name = mir_utf8decodeT((char*)fileName);
		if (name == NULL)
		{
			// uTox send file name in ansi
			name = mir_a2u((char*)fileName);
		}

		FileTransferParam *transfer = new FileTransferParam(friendNumber, fileNumber, name, fileSize);
		transfer->pfts.hContact = hContact;
		transfer->pfts.flags |= PFTS_RECEIVING;
		proto->transfers->Add(transfer);

		PROTORECVFILET pre = { 0 };
		pre.flags = PREF_TCHAR;
		pre.fileCount = 1;
		pre.timestamp = time(NULL);
		pre.tszDescription = _T("");
		pre.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)*2);
		pre.ptszFiles[0] = name;
		pre.ptszFiles[1] = NULL;
		pre.lParam = (LPARAM)transfer;
		ProtoChainRecvFile(hContact, &pre);
	}
}

// file request is allowed
HANDLE __cdecl CToxProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR *tszPath)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	transfer->pfts.tszWorkingDir = mir_tstrdup(tszPath);

	// stupid fix
	TCHAR fullPath[MAX_PATH];
	mir_sntprintf(fullPath, SIZEOF(fullPath), _T("%s\\%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);
	transfer->RenameName(fullPath);

	if (!ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)transfer, (LPARAM)&transfer->pfts))
	{
		if (!transfer->OpenFile(_T("wb")))
		{
			debugLogA("CToxProto::FileAllow: cannot to open file (%d)", transfer->fileNumber);
			transfer->Fail(tox);
			transfers->Remove(transfer);
			return NULL;
		}

		debugLogA("CToxProto::FileAllow: start receiving file (%d)", transfer->fileNumber);
		transfer->Start(tox);
	}

	return hTransfer;
}

// if file is exists
int __cdecl CToxProto::FileResume(HANDLE hTransfer, int *action, const PROTOCHAR **szFilename)
{
	bool result = false;
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;

	switch (*action)
	{
	case FILERESUME_RENAME:
		transfer->RenameName(*szFilename);
		result = transfer->OpenFile(_T("wb"));
		break;

	case FILERESUME_OVERWRITE:
		result = transfer->OpenFile(_T("wb"));
		break;

	case FILERESUME_SKIP:
		result = false;
		return 0;

	case FILERESUME_RESUME:
		result = transfer->OpenFile(_T("ab"));
		break;
	}

	if (result)
	{
		transfer->Start(tox);
	}
	else
	{
		transfer->Cancel(tox);
		transfers->Remove(transfer);
	}
	ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, result ? ACKRESULT_CONNECTED : ACKRESULT_DENIED, (HANDLE)transfer, 0);

	return 0;
}

// getting the file data
void CToxProto::OnFileData(Tox *tox, int32_t friendNumber, uint8_t fileNumber, const uint8_t *data, uint16_t size, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(friendNumber);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers->Get(fileNumber);
		if (transfer = NULL)
		{
			tox_file_send_control(tox, friendNumber, 1, fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
			return;
		}

		if (fwrite(data, sizeof(uint8_t), size, transfer->hFile) != size)
		{
			proto->debugLogA("CToxProto::OnFileData: cannot write to file (%d)", fileNumber);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
			transfer->Fail(tox);
		}

		transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += size;
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
	}
}

/* FILE SENDING */

// outcoming file flow
// send request through tox
HANDLE __cdecl CToxProto::SendFile(MCONTACT hContact, const PROTOCHAR *szDescription, PROTOCHAR **ppszFiles)
{
	std::string id = getStringA(hContact, TOX_SETTINGS_ID);
	std::vector<uint8_t> clientId = HexStringToData(id);
	uint32_t friendNumber = tox_get_friend_number(tox, clientId.data());

	TCHAR *fileName = _tcsrchr(ppszFiles[0], '\\') + 1;

	size_t fileDirLength = fileName - ppszFiles[0];
	TCHAR *fileDir = (TCHAR*)mir_alloc(sizeof(TCHAR)*(fileDirLength + 1));
	_tcsncpy(fileDir, ppszFiles[0], fileDirLength);
	fileDir[fileDirLength] = '\0';

	size_t fileSize = 0;
	FILE *hFile = _tfopen(ppszFiles[0], _T("rb"));
	if (hFile == NULL)
	{
		debugLogA("CToxProto::SendFilesAsync: cannot open file");
		return NULL;
	}
	fseek(hFile, 0, SEEK_END);
	fileSize = ftell(hFile);
	fseek(hFile, 0, SEEK_SET);

	char *name = mir_utf8encodeW(fileName);
	int fileNumber = tox_new_file_sender(tox, friendNumber, fileSize, (uint8_t*)name, strlen(name));
	if (fileNumber < 0)
	{
		debugLogA("CToxProto::SendFilesAsync: cannot send file");
		return NULL;
	}

	FileTransferParam *transfer = new FileTransferParam(friendNumber, fileNumber, fileName, fileSize);
	transfer->pfts.hContact = hContact;
	transfer->pfts.flags |= PFTS_SENDING;
	transfer->pfts.tszWorkingDir = fileDir;
	transfer->hFile = hFile;
	transfers->Add(transfer);

	return (HANDLE)transfer;
}

// start sending
void CToxProto::SendFileAsync(void *arg)
{
	FileTransferParam *transfer = (FileTransferParam*)arg;
	transfer->status = STARTED;

	debugLogA("CToxProto::SendFileAsync: start sending file (%d)", transfer->fileNumber);
	ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)transfer, 0);

	size_t fileSize = transfer->pfts.currentFileSize;
	size_t fileProgress = 0;
	TCHAR filePath[MAX_PATH];
	mir_sntprintf(filePath, SIZEOF(filePath), _T("%s%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);

	uint16_t chunkSize = min(fileSize, (size_t)tox_file_data_size(tox, transfer->friendNumber));
	uint8_t *data = (uint8_t*)mir_alloc(chunkSize);
	while (transfer->status < FAILED && !feof(transfer->hFile) && fileProgress < fileSize)
	{
		if (transfer->status == PAUSED)
		{
			Sleep(1000);
			continue;
		}

		//mir_cslock lock(toxLock);

		uint16_t size = min(chunkSize, fileSize - fileProgress);
		if (fread(data, sizeof(uint8_t), size, transfer->hFile) != size)
		{
			transfer->Fail(tox);
			debugLogA("CToxProto::SendFileAsync: failed to read from file (%d)", transfer->fileNumber);
			ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
			return;
		}

		if (tox_file_send_data(tox, transfer->friendNumber, transfer->fileNumber, data, size) == TOX_ERROR)
		{
			fseek(transfer->hFile, -size, SEEK_CUR);
			continue;
		}

		transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = fileProgress += size;
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
	}
	mir_free(data);

	if (transfer->status == STARTED)
	{
		//mir_cslock lock(toxLock);
		transfer->Finish(tox);
	}
}

/* GENERAL */

// file request is cancelled
int __cdecl CToxProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	transfer->Cancel(tox);
	transfers->Remove(transfer);

	return 0;
}

// file request is denied
int __cdecl CToxProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR*)
{
	return FileCancel(hContact, hTransfer);
}

void CToxProto::OnFileRequest(Tox *tox, int32_t friendNumber, uint8_t receive_send, uint8_t fileNumber, uint8_t type, const uint8_t *data, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	//mir_cslock lock(proto->toxLock);

	MCONTACT hContact = proto->FindContact(friendNumber);
	if (hContact)
	{
		
		FileTransferParam *transfer = proto->transfers->Get(fileNumber);
		if (transfer = NULL)
		{
			tox_file_send_control(tox, friendNumber, receive_send, fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
			return;
		}

		switch (type)
		{
		case TOX_FILECONTROL_ACCEPT:
			if (receive_send == 1)
			{
				// if friend allow file request
				if (transfer->status == NONE)
				{
					proto->ForkThread(&CToxProto::SendFileAsync, transfer);
				}
				else
				{
					// unpause file transfer
					transfer->status = STARTED;
				}
			}
			break;

		case TOX_FILECONTROL_PAUSE:
			transfer->status = PAUSED;
			break;

		case TOX_FILECONTROL_RESUME_BROKEN:
			// only for sending
			if (receive_send == 0)
			{
				uint64_t progress = *(uint64_t*)data;
				transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = progress;
				fseek(transfer->hFile, progress, SEEK_SET);
				transfer->Start(tox);
			}
			break;

		case TOX_FILECONTROL_KILL:
			transfer->status = CANCELED;
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
			proto->transfers->Remove(transfer);
			break;

		case TOX_FILECONTROL_FINISHED:
			if (receive_send == 0)
			{
				transfer->Finish(tox);
				proto->debugLogA("CToxProto::OnFileRequest: finished receiving file (%d)", fileNumber);
			}
			else
			{
				proto->debugLogA("CToxProto::OnFileRequest: finished sending file (%d)", fileNumber);
			}
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)transfer, 0);
			proto->transfers->Remove(transfer);
			break;
		}
	}
}