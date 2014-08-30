#include "common.h"

void CToxProto::SendFileAsync(void* arg)
{
	FileTransferParam *transfer = (FileTransferParam*)arg;

	std::string toxId(getStringA(transfer->pfts.hContact, TOX_SETTINGS_ID));
	std::vector<uint8_t> clientId = HexStringToData(toxId);

	uint32_t number = tox_get_friend_number(tox, clientId.data());
	if (number < 0)
	{
		return;
	}

	size_t fileSize = transfer->pfts.currentFileSize;
	size_t fileProgress = 0;
	TCHAR filePath[MAX_PATH];
	mir_sntprintf(filePath, SIZEOF(filePath), _T("%s%s"), transfer->pfts.tszWorkingDir, transfer->pfts.tszCurrentFile);

	FILE *hFile = _wfopen(filePath, _T("rb"));
	if (hFile != NULL)
	{
		size_t chunkSize = min(tox_file_data_size(tox, number), fileSize);
		uint8_t *data = (uint8_t*)mir_alloc(chunkSize);
		while (!feof(hFile) && fileProgress < fileSize)
		{
			size_t size = min(chunkSize, fileSize - fileProgress);
			if (fread(data, sizeof(uint8_t), size, hFile) == size)
			{
				tox_file_send_data(tox, number, transfer->number, data, size);
				transfer->pfts.totalProgress = transfer->pfts.currentFileProgress = fileProgress += size;

				ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer->number, (LPARAM)&transfer->pfts);
			}
		}
		mir_free(data);
		tox_file_send_control(tox, number, 0, transfer->number, TOX_FILECONTROL_FINISHED, NULL, 0);
	}
	else
	{
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer->number, 0);
	}
}

void CToxProto::OnFriendFile(Tox *tox, int32_t number, uint8_t fileNumber, uint64_t fileSize, const uint8_t *fileName, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		FileTransferParam *transfer = new FileTransferParam(fileNumber, ptrT(mir_utf8decodeT((const char*)fileName)), fileSize);
		transfer->pfts.hContact = hContact;
		transfer->pfts.flags |= PFTS_RECEIVING;
		proto->transfers[fileNumber] = transfer;

		PROTORECVFILET pre = { 0 };
		pre.flags = PREF_TCHAR;
		pre.fileCount = 1;
		pre.timestamp = time(NULL);
		pre.tszDescription = _T("");
		pre.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*)* 2);
		pre.ptszFiles[0] = mir_utf8decodeT((char*)fileName);
		pre.ptszFiles[1] = NULL;
		pre.lParam = (LPARAM)fileNumber;
		ProtoChainRecvFile(hContact, &pre);
	}
}

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
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)fileNumber, 0);
		}
		else
		{
			hFile = _tfopen(filePath, _T("ab"));
		}
		if (hFile != NULL)
		{
			fseek(hFile, transfer->pfts.currentFileProgress + 1, SEEK_SET);
			if (fwrite(data, sizeof(uint8_t), size, hFile) == size)
			{
				transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += size;
				proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)fileNumber, (LPARAM)&transfer->pfts);
			}
			fclose(hFile);
		}
	}
}

void CToxProto::OnFileRequest(Tox *tox, int32_t number, uint8_t isSend, uint8_t fileNumber, uint8_t type, const uint8_t *data, uint16_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		FileTransferParam *transfer = proto->transfers.at(fileNumber);

		switch (type)
		{
		case TOX_FILECONTROL_ACCEPT:
			proto->ForkThread(&CToxProto::SendFileAsync, transfer);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, (HANDLE)fileNumber, 0);
			break;

		case TOX_FILECONTROL_FINISHED:
			tox_file_send_control(proto->tox, number, 1, fileNumber, TOX_FILECONTROL_FINISHED, NULL, 0);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, (HANDLE)fileNumber, 0);
			break;
		}
	}
}