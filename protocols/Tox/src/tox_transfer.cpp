#include "common.h"

/* FILE RECEIVING */

// incoming file flow
void CToxProto::OnFriendFile(Tox*, uint32_t friendNumber, uint32_t fileNumber, uint32_t kind, uint64_t fileSize, const uint8_t *fileName, size_t filenameLength, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		switch (kind)
		{
		case TOX_FILE_KIND_AVATAR:
		case TOX_FILE_KIND_DATA:
		{
			ptrA rawName((char*)mir_alloc(filenameLength + 1));
			memcpy(rawName, fileName, filenameLength);
			rawName[filenameLength] = 0;
			TCHAR *name = mir_utf8decodeT(rawName);

			FileTransferParam *transfer = new FileTransferParam(friendNumber, fileNumber, name, fileSize);
			transfer->pfts.hContact = hContact;
			proto->transfers.Add(transfer);

			if (kind == TOX_FILE_KIND_AVATAR)
			{
				proto->OnGotFriendAvatarInfo(transfer, fileName);
				return;
			}

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
		break;

		default:
			proto->debugLogA(__FUNCTION__": unknown file kind (%d)", kind);
			return;
		}
	}
}

// file request is allowed
HANDLE CToxProto::OnFileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR *tszPath)
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
			debugLogA("CToxProto::FileAllow: failed to open file (%d)", transfer->fileNumber);
			transfer->status = FAILED;
			tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
			transfers.Remove(transfer);
			return NULL;
		}

		debugLogA("CToxProto::FileAllow: start receiving file (%d)", transfer->fileNumber);
		transfer->status = STARTED;
		TOX_ERR_FILE_CONTROL error;
		if (!tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_RESUME, &error))
		{
			debugLogA("CToxProto::FileAllow: failed to start the transfer (%d)", error);
			tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
		}
	}

	return hTransfer;
}

// if file is exists
int CToxProto::OnFileResume(HANDLE hTransfer, int *action, const PROTOCHAR **szFilename)
{
	bool result = false;
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;

	switch (*action)
	{
	case FILERESUME_RENAME:
		transfer->ChangeName(*szFilename);
	case FILERESUME_OVERWRITE:
		result = transfer->OpenFile(_T("wb"));
		break;

	case FILERESUME_RESUME:
		result = transfer->OpenFile(_T("ab"));
		break;

	case FILERESUME_SKIP:
		result = false;
		break;
	}

	TOX_ERR_FILE_CONTROL error;
	if (result)
	{
		debugLogA("CToxProto::FileResume: start receiving file (%d)", transfer->fileNumber);
		transfer->status = STARTED;
		if (!tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_RESUME, &error))
		{
			debugLogA("CToxProto::FileResume: failed to start the transfer (%d)", error);
			tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, &error);
		}
	}
	else
	{
		transfer->status = CANCELED;
		tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, &error);
		transfers.Remove(transfer);
	}
	ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, result ? ACKRESULT_CONNECTED : ACKRESULT_DENIED, (HANDLE)transfer, 0);

	return 0;
}

// getting the file data
void CToxProto::OnFileReceiveData(Tox*, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, const uint8_t *data, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	FileTransferParam *transfer = proto->transfers.Get(friendNumber, fileNumber);
	if (transfer == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to fing transfer (%d)", fileNumber);
		return;
	}

	if (length == 0 || position == UINT64_MAX)
	{
		//receiving is finished
		proto->debugLogA(__FUNCTION__": finised the transfer of file (%d)", fileNumber);
		bool isFileFullyTransfered = transfer->pfts.currentFileProgress == transfer->pfts.currentFileSize;
		transfer->status = isFileFullyTransfered ? FINISHED : FAILED;
		if (!isFileFullyTransfered)
		{
			proto->debugLogA(__FUNCTION__": file (%d) is transferred not completely", fileNumber);
		}
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, isFileFullyTransfered ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)transfer, 0);
		proto->transfers.Remove(transfer);
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA("CToxProto::OnFileData: cannot find contact by number (%d)", friendNumber);
		tox_file_control(proto->tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
		return;
	}

	if (fwrite(data, sizeof(uint8_t), length, transfer->hFile) != length)
	{
		proto->debugLogA(__FUNCTION__": failed write to file (%d)", fileNumber);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
		transfer->status = FAILED;
		tox_file_control(proto->tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
		return;
	}

	transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += length;
	proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
}

/* FILE SENDING */

// outcoming file flow
HANDLE CToxProto::OnSendFile(MCONTACT hContact, const PROTOCHAR*, PROTOCHAR **ppszFiles)
{
	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber == UINT32_MAX)
		return NULL;


	FILE *hFile = _tfopen(ppszFiles[0], _T("rb"));
	if (hFile == NULL)
	{
		debugLogA(__FUNCTION__": cannot open file");
		return NULL;
	}

	TCHAR *fileName = _tcsrchr(ppszFiles[0], '\\') + 1;
	size_t fileDirLength = fileName - ppszFiles[0];
	TCHAR *fileDir = (TCHAR*)mir_alloc(sizeof(TCHAR)*(fileDirLength + 1));
	_tcsncpy(fileDir, ppszFiles[0], fileDirLength);
	fileDir[fileDirLength] = '\0';

	_fseeki64(hFile, 0, SEEK_END);
	uint64_t fileSize = _ftelli64(hFile);
	rewind(hFile);

	char *name = mir_utf8encodeW(fileName);
	TOX_ERR_FILE_SEND sendError;
	uint32_t fileNumber = tox_file_send(tox, friendNumber, TOX_FILE_KIND_DATA, fileSize, NULL, (uint8_t*)name, mir_strlen(name), &sendError);
	if (sendError != TOX_ERR_FILE_SEND_OK)
	{
		debugLogA(__FUNCTION__": failed to send file (%d)", sendError);
		mir_free(fileDir);
		mir_free(name);
		return NULL;
	}

	FileTransferParam *transfer = new FileTransferParam(friendNumber, fileNumber, fileName, fileSize);
	transfer->pfts.hContact = hContact;
	transfer->pfts.tszWorkingDir = fileDir;
	transfer->hFile = hFile;
	transfers.Add(transfer);
	
	mir_free(name);
	return (HANDLE)transfer;
}

void CToxProto::OnFileSendData(Tox*, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	FileTransferParam *transfer = proto->transfers.Get(friendNumber, fileNumber);
	if (transfer == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to fing transfer (%d)", fileNumber);
		return;
	}

	if (length == 0)
	{
		// file sending is finished
		proto->debugLogA(__FUNCTION__": finised the transfer of file (%d)", fileNumber);
		bool isFileFullyTransfered = transfer->pfts.currentFileProgress == transfer->pfts.currentFileSize;
		transfer->status = isFileFullyTransfered ? FINISHED : FAILED;
		if (!isFileFullyTransfered)
		{
			proto->debugLogA(__FUNCTION__": file (%d) is transferred not completely", fileNumber);
		}
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, isFileFullyTransfered ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)transfer, 0);
		proto->transfers.Remove(transfer);
		return;
	}

	uint64_t sentBytes = _ftelli64(transfer->hFile);
	if (sentBytes != position)
		_fseeki64(transfer->hFile, position, SEEK_SET);

	uint8_t *data = (uint8_t*)mir_alloc(length);
	if (fread(data, sizeof(uint8_t), length, transfer->hFile) != length)
	{
		proto->debugLogA(__FUNCTION__": failed to read from file (%d)", transfer->fileNumber);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
		transfer->status = FAILED;
		tox_file_control(proto->tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
		mir_free(data);
		return;
	}

	TOX_ERR_FILE_SEND_CHUNK error;
	if (!tox_file_send_chunk(proto->tox, friendNumber, fileNumber, position, data, length, &error))
	{
		proto->debugLogA(__FUNCTION__": failed to send file chunk (%d)", error);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
		transfer->status = FAILED;
		tox_file_control(proto->tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
		mir_free(data);
		return;
	}

	transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = length;

	mir_free(data);
}

/* COMMON */

// file request is cancelled
int CToxProto::OnFileCancel(MCONTACT, HANDLE hTransfer)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	transfer->status = CANCELED;
	tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, NULL);
	transfers.Remove(transfer);

	return 0;
}

void CToxProto::OnFileRequest(Tox*, uint32_t friendNumber, uint32_t fileNumber, TOX_FILE_CONTROL control, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers.Get(friendNumber, fileNumber);
		if (transfer == NULL)
		{
			proto->debugLogA(__FUNCTION__": failed to fing transfer (%d)", fileNumber);
			return;
		}

		switch (control)
		{
		case TOX_FILE_CONTROL_PAUSE:
			transfer->status = PAUSED;
			break;

		case TOX_FILE_CONTROL_RESUME:
			transfer->status = STARTED;
			proto->debugLogA("CToxProto::OnFileRequest: start/resume the transfer of file (%d)", transfer->fileNumber);
			break;

		case TOX_FILE_CONTROL_CANCEL:
			transfer->status = CANCELED;
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
			proto->transfers.Remove(transfer);
			break;
		}
	}
}