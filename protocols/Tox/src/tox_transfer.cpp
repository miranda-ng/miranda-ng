#include "common.h"

void CToxProto::AddToTransferList(FileTransferParam *transfer)
{
	if (transfers.find(transfer->number) != transfers.end())
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
		FileTransferParam *transfer = new FileTransferParam(fileNumber, ptrT(mir_utf8decodeT((const char*)fileName)), fileSize);
		transfer->pfts.hContact = hContact;
		transfer->pfts.flags |= PFTS_RECEIVING;
		proto->AddToTransferList(transfer);

		PROTORECVFILET pre = { 0 };
		pre.flags = PREF_TCHAR;
		pre.fileCount = 1;
		pre.timestamp = time(NULL);
		pre.tszDescription = _T("");
		pre.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)* 2);
		pre.ptszFiles[0] = mir_utf8decodeT((char*)fileName);
		pre.ptszFiles[1] = NULL;
		pre.lParam = (LPARAM)transfer;
		ProtoChainRecvFile(hContact, &pre);
	}
}

// file request is allowed
HANDLE __cdecl CToxProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* tszPath)
{
	std::string toxId(getStringA(hContact, TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	uint32_t number = tox_get_friend_number(tox, clientId.data());

	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	transfer->pfts.tszWorkingDir = mir_tstrdup(tszPath);

	// stupid fix
	//TCHAR fullPath[MAX_PATH];
	//mir_sntprintf(fullPath, SIZEOF(fullPath), _T("%s\\%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);
	//replaceStrT(transfer->pfts.tszCurrentFile, fullPath);

	//if (!ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)transfer, (LPARAM)&transfer->pfts))
	{
		tox_file_send_control(tox, number, 1, transfer->number, TOX_FILECONTROL_ACCEPT, NULL, 0);
	}

	return hTransfer;
}

// file request is denied
int __cdecl CToxProto::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	std::string toxId(getStringA(hContact, TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	uint32_t number = tox_get_friend_number(tox, clientId.data());

	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	RemoveFromTransferList(transfer);

	tox_file_send_control(tox, number, 1, transfer->number, TOX_FILECONTROL_KILL, NULL, 0);

	return 0;
}

// file request is denied
int __cdecl CToxProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR*)
{
	return FileCancel(hContact, hTransfer);
}

// some file request stuff
int __cdecl CToxProto::FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename)
{
	uint8_t fileNumber = (uint8_t)hTransfer;
	FileTransferParam *transfer = transfers.at(fileNumber);

	switch (*action)
	{
	case FILERESUME_RESUME:
		//replaceStrT(ft->std.tszCurrentFile, *szFilename);
		break;

	case FILERESUME_RENAME:
		replaceStrT(transfer->pfts.tszCurrentFile, *szFilename);
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
		FileTransferParam *transfer = proto->transfers.at(fileNumber);

		TCHAR filePath[MAX_PATH];
		mir_sntprintf(filePath, SIZEOF(filePath), _T("%s%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);

		FILE *hFile = NULL;
		if (transfer->pfts.currentFileProgress == 0)
		{
			hFile = _tfopen(filePath, _T("wb+"));
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)transfer, 0);
		}
		else
		{
			hFile = _tfopen(filePath, _T("ab"));
		}
		if (hFile != NULL)
		{
			if (fwrite(data, sizeof(uint8_t), size, hFile) == size)
			{
				transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += size;
				proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
			}
			fclose(hFile);
		}
	}
}

/* FILE SENDING */

// outcoming file flow
// send request through tox
HANDLE __cdecl CToxProto::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
{
	std::string toxId(getStringA(hContact, TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	uint32_t number = tox_get_friend_number(tox, clientId.data());

	TCHAR *fileName = _tcsrchr(ppszFiles[0], '\\') + 1;

	size_t fileDirLength = fileName - ppszFiles[0];
	TCHAR *fileDir = (TCHAR*)mir_alloc(sizeof(TCHAR)*fileDirLength);
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

	int fileNumber = tox_new_file_sender(tox, number, fileSize, (uint8_t*)(char*)ptrA(mir_utf8encodeT(fileName)), _tcslen(fileName));
	if (fileNumber < 0)
	{
		debugLogA("CToxProto::SendFilesAsync: cannot send file");
		return NULL;
	}

	FileTransferParam *transfer = new FileTransferParam(fileNumber, fileName, fileSize);
	transfer->pfts.hContact = hContact;
	transfer->pfts.flags |= PFTS_RECEIVING;
	transfer->pfts.tszWorkingDir = fileDir;
	AddToTransferList(transfer);

	return (HANDLE)transfer;
}

// start sending
void CToxProto::SendFileAsync(void* arg)
{
	FileTransferParam *transfer = (FileTransferParam*)arg;

	std::string toxId(getStringA(transfer->pfts.hContact, TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	if (uint32_t number = tox_get_friend_number(tox, clientId.data()) > TOX_ERROR)
	{
		size_t fileSize = transfer->pfts.currentFileSize;
		size_t fileProgress = 0;
		TCHAR filePath[MAX_PATH];
		mir_sntprintf(filePath, SIZEOF(filePath), _T("%s%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);

		FILE *hFile = _wfopen(filePath, _T("rb"));
		if (!hFile)
		{
			ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer, 0);
			RemoveFromTransferList(transfer);
			return;
		}

		size_t chunkSize = min(tox_file_data_size(tox, number), fileSize);
		uint8_t *data = (uint8_t*)mir_alloc(chunkSize);
		while (!feof(hFile) && fileProgress < fileSize)
		{
			size_t size = min(chunkSize, fileSize - fileProgress);
			if (fread(data, sizeof(uint8_t), size, hFile) == size)
			{
				tox_file_send_data(tox, number, transfer->number, data, size);
				transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = fileProgress += size;

				ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
			}
		}
		mir_free(data);
		tox_file_send_control(tox, number, 0, transfer->number, TOX_FILECONTROL_FINISHED, NULL, 0);
	}
}

/* GENERAL */

void CToxProto::OnFileRequest(Tox *tox, int32_t number, uint8_t receive_send, uint8_t fileNumber, uint8_t type, const uint8_t *data, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers.at(fileNumber);

		switch (type)
		{
			// if friend allow file request
		case TOX_FILECONTROL_ACCEPT:
			proto->ForkThread(&CToxProto::SendFileAsync, transfer);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)transfer, 0);
			break;

			// if file transfer is paused
		case TOX_FILECONTROL_PAUSE:
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)transfer, 0);
			break;

			// if file transfer is cancelled
		case TOX_FILECONTROL_KILL:
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer, 0);
			break;

			// if file transfer is finished
		case TOX_FILECONTROL_FINISHED:
			tox_file_send_control(proto->tox, number, 1, fileNumber, TOX_FILECONTROL_FINISHED, NULL, 0);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)transfer, 0);
			proto->RemoveFromTransferList(transfer);
			break;
		}
	}
}