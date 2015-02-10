#include "common.h"

/* FILE RECEIVING */

// incoming file flow
void CToxProto::OnFriendFile(Tox *tox, int32_t friendNumber, uint8_t fileNumber, uint64_t fileSize, const uint8_t *fileName, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
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
	transfer->ChangeName(fullPath);

	if (!ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)transfer, (LPARAM)&transfer->pfts))
	{
		if (!transfer->OpenFile(_T("wb")))
		{
			debugLogA("CToxProto::FileAllow: cannot to open file (%d)", transfer->fileNumber);
			transfer->status = FAILED;
			tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
			transfers->Remove(transfer);
			return NULL;
		}

		debugLogA("CToxProto::FileAllow: start receiving file (%d)", transfer->fileNumber);
		transfer->status = STARTED;
		if (tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_ACCEPT, NULL, 0) == TOX_ERROR)
		{
			debugLogA("CToxProto::FileAllow: failed to start the transfer of file (%d)", transfer->fileNumber);
			tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		}
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
		transfer->ChangeName(*szFilename);
		result = transfer->OpenFile(_T("wb"));
		break;

	case FILERESUME_OVERWRITE:
		result = transfer->OpenFile(_T("wb"));
		break;

	case FILERESUME_SKIP:
		result = false;
		break;

	case FILERESUME_RESUME:
		result = transfer->OpenFile(_T("ab"));
		break;
	}

	if (result)
	{
		debugLogA("CToxProto::FileResume: start receiving file (%d)", transfer->fileNumber);
		transfer->status = STARTED;
		if (tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_ACCEPT, NULL, 0) == TOX_ERROR)
		{
			debugLogA("CToxProto::FileResume: failed to start the transfer of file (%d)", transfer->fileNumber);
			tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		}
	}
	else
	{
		transfer->status = CANCELED;
		tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		transfers->Remove(transfer);
	}
	ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, result ? ACKRESULT_CONNECTED : ACKRESULT_DENIED, (HANDLE)transfer, 0);

	return 0;
}

// getting the file data
void CToxProto::OnFileData(Tox *tox, int32_t friendNumber, uint8_t fileNumber, const uint8_t *data, uint16_t size, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA("CToxProto::OnFileData: cannot find contact by number (%d)", friendNumber);
		tox_file_send_control(tox, friendNumber, 1, fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		return;
	}

	FileTransferParam *transfer = proto->transfers->Get(fileNumber);
	if (transfer == NULL)
	{
		proto->debugLogA("CToxProto::OnFileData: cannot find transfer by number (%d)", fileNumber);
		transfer->status = FAILED;
		tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		return;
	}

	if (fwrite(data, sizeof(uint8_t), size, transfer->hFile) != size)
	{
		proto->debugLogA("CToxProto::OnFileData: cannot write to file (%d)", fileNumber);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
		transfer->status = FAILED;
		tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		return;
	}

	transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += size;
	proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
}

/* FILE SENDING */

// outcoming file flow
HANDLE __cdecl CToxProto::SendFile(MCONTACT hContact, const PROTOCHAR*, PROTOCHAR **ppszFiles)
{
	ToxBinAddress pubKey = ptrA(getStringA(hContact, TOX_SETTINGS_ID));
	int32_t friendNumber = tox_get_friend_number(tox, pubKey);
	if (friendNumber == TOX_ERROR)
	{
		debugLogA("CToxProto::SendMsg: failed to get friend number");
		return NULL;
	}

	TCHAR *fileName = _tcsrchr(ppszFiles[0], '\\') + 1;

	size_t fileDirLength = fileName - ppszFiles[0];
	TCHAR *fileDir = (TCHAR*)mir_alloc(sizeof(TCHAR)*(fileDirLength + 1));
	_tcsncpy(fileDir, ppszFiles[0], fileDirLength);
	fileDir[fileDirLength] = '\0';

	FILE *hFile = _tfopen(ppszFiles[0], _T("rb"));
	if (hFile == NULL)
	{
		debugLogA("CToxProto::SendFilesAsync: cannot open file");
		return NULL;
	}

	_fseeki64(hFile, 0, SEEK_END);
	uint64_t fileSize = _ftelli64(hFile);
	rewind(hFile);

	char *name = mir_utf8encodeW(fileName);
	int fileNumber = tox_new_file_sender(tox, friendNumber, fileSize, (uint8_t*)name, mir_strlen(name));
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

	int dataSize = 0;
	uint64_t fileProgress = transfer->pfts.currentFileProgress;
	uint64_t fileSize = transfer->pfts.currentFileSize;
	int chunkSize = min(tox_file_data_size(tox, transfer->friendNumber), fileSize);
	uint8_t *data = (uint8_t*)mir_alloc(chunkSize);

	while (transfer->status == STARTED && transfer->hFile != NULL && fileProgress < fileSize)
	{
		if (dataSize == 0)
		{
			dataSize = min(chunkSize, fileSize - fileProgress);
			int read = fread(data, sizeof(uint8_t), dataSize, transfer->hFile);
			if (read != dataSize)
			{
				debugLogA("CToxProto::SendFileAsync: failed to read from file (%d)", transfer->fileNumber);
				debugLogA("CToxProto::SendFileAsync: read %d of %d (%d)", read, dataSize, transfer->fileNumber);
				debugLogA("CToxProto::SendFileAsync: sent %llu of %llu of file (%d)", transfer->pfts.currentFileProgress, transfer->pfts.currentFileSize, transfer->fileNumber);
				ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
				transfer->status = FAILED;
				tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
				return;
			}
		}

		int sendResult = TOX_ERROR;
		{
			mir_cslock lock(toxLock);
			sendResult = tox_file_send_data(tox, transfer->friendNumber, transfer->fileNumber, data, dataSize);
		}
		if (sendResult == TOX_ERROR)
		{
			Sleep(100);
			continue;
		}

		transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = fileProgress += dataSize;
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
		dataSize = 0;
	}

	mir_free(data);

	if (transfer->status == STARTED && fileProgress == fileSize)
	{
		transfer->status = FINISHED;
		if (tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_FINISHED, NULL, 0) == TOX_ERROR)
		{
			debugLogA("CToxProto::SendFileAsync: failed to finish the transfer of file (%d)", transfer->fileNumber);
			tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
		}
	}
}

/* COMMON */

// file request is cancelled
int __cdecl CToxProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	transfer->status = CANCELED;
	tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
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

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers->Get(fileNumber);
		if (transfer == NULL)
		{
			tox_file_send_control(tox, friendNumber, receive_send, fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
			return;
		}

		switch (type)
		{
		case TOX_FILECONTROL_ACCEPT:
			// receiver allowed the transfer
			if (receive_send == 1)
			{
				proto->debugLogA("CToxProto::OnFileRequest: start the transfer of file (%d)", transfer->fileNumber);
				// start file sending
				proto->ForkThread(&CToxProto::SendFileAsync, transfer);
			}
			break;

		case TOX_FILECONTROL_PAUSE:
			transfer->status = PAUSED;
			break;

		case TOX_FILECONTROL_RESUME_BROKEN:
			// receiver asked to resume transfer
			if (receive_send == 1)
			{
				uint64_t progress = *(uint64_t*)data;
				if (progress >= transfer->pfts.currentFileSize || length != sizeof(uint64_t))
				{
					transfer->status = FAILED;
					tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
				}
				if (tox_file_send_control(tox, friendNumber, transfer->GetDirection(), fileNumber, TOX_FILECONTROL_ACCEPT, NULL, 0) == TOX_ERROR)
				{
					proto->debugLogA("CToxProto::OnFileRequest: failed to resume the transfer of file (%d)", transfer->fileNumber);
					proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
					transfer->status = FAILED;
					tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
					break;
				}
				if (transfer->pfts.currentFileProgress != progress)
				{
					transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = progress;
					if (_fseeki64(transfer->hFile, progress, SEEK_SET))
					{
						proto->debugLogA("CToxProto::OnFileRequest: failed to change file position from %llu to %llu of file (%d)",
							transfer->pfts.currentFileProgress, progress, transfer->fileNumber);
						transfer->status = FAILED;
						tox_file_send_control(tox, transfer->friendNumber, transfer->GetDirection(), transfer->fileNumber, TOX_FILECONTROL_KILL, NULL, 0);
					}
				}
				proto->debugLogA("CToxProto::SendFileAsync: resume the transfer of file (%d)", transfer->fileNumber);
				// resume file sending
				proto->ForkThread(&CToxProto::SendFileAsync, transfer);
			}
			break;

		case TOX_FILECONTROL_KILL:
			transfer->status = CANCELED;
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
			proto->transfers->Remove(transfer);
			break;

		case TOX_FILECONTROL_FINISHED:
			{
				proto->debugLogA("CToxProto::SendFileAsync: finish the transfer of file (%d)", transfer->fileNumber);
				bool isFileFullyTransfered = transfer->pfts.currentFileProgress == transfer->pfts.currentFileSize;
				transfer->status = isFileFullyTransfered ? FINISHED : FAILED;
				if (!isFileFullyTransfered)
				{
					proto->debugLogA("CToxProto::SendFileAsync: file (%d) is transferred not completely", transfer->fileNumber);
				}
				tox_file_send_control(tox, friendNumber, transfer->GetDirection(), fileNumber, isFileFullyTransfered ? TOX_FILECONTROL_FINISHED : TOX_FILECONTROL_KILL, NULL, 0);
				proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, isFileFullyTransfered ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)transfer, 0);
				proto->transfers->Remove(transfer);
			}
			break;
		}
	}
}