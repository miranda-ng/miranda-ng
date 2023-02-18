#include "stdafx.h"

/* FILE RECEIVING */

// incoming transfer flow
void CToxProto::OnFriendFile(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint32_t kind, uint64_t fileSize, const uint8_t *fileName, size_t fileNameLength, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxHexAddress pubKey = proto->GetContactPublicKey(tox, friendNumber);

	MCONTACT hContact = proto->GetContact(tox, friendNumber);
	if (hContact == NULL) {
		Netlib_Logf(proto->m_hNetlibUser, __FUNCTION__": cannot find contact %s (%d)", (const char*)pubKey, friendNumber);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		return;
	}

	switch (kind) {
	case TOX_FILE_KIND_AVATAR:
		{
			proto->debugLogA(__FUNCTION__": incoming avatar (%d) from %s (%d)", fileNumber, (const char*)pubKey, friendNumber);

			ptrW address(proto->getWStringA(hContact, TOX_SETTINGS_ID));
			wchar_t avatarName[MAX_PATH];
			mir_snwprintf(avatarName, MAX_PATH, L"%s.png", address.get());

			AvatarTransferParam *transfer = new AvatarTransferParam(friendNumber, fileNumber, avatarName, fileSize);
			transfer->pfts.flags |= PFTS_RECEIVING;
			transfer->pfts.hContact = hContact;
			proto->transfers.Add(transfer);

			TOX_ERR_FILE_GET error;
			tox_file_get_file_id(tox, friendNumber, fileNumber, transfer->hash, &error);
			if (error != TOX_ERR_FILE_GET_OK) {
				Netlib_Logf(proto->m_hNetlibUser, __FUNCTION__": unable to get avatar hash (%d) from %s (%d) cause (%d)", fileNumber, (const char*)pubKey, friendNumber, error);
				memset(transfer->hash, 0, TOX_HASH_LENGTH);
			}
			proto->OnGotFriendAvatarInfo(tox, transfer);
		}
		break;

	case TOX_FILE_KIND_DATA:
		{
			proto->debugLogA(__FUNCTION__": incoming file (%d) from %s (%d)", fileNumber, (const char*)pubKey, friendNumber);

			CMStringA rawName((char*)fileName, (int)fileNameLength);
			const wchar_t *name = mir_utf8decodeW(rawName);

			FileTransferParam *transfer = new FileTransferParam(friendNumber, fileNumber, name, fileSize);
			transfer->pfts.flags |= PFTS_RECEIVING;
			transfer->pfts.hContact = hContact;
			proto->transfers.Add(transfer);

			PROTORECVFILE pre = {};
			pre.dwFlags = PRFF_UNICODE;
			pre.fileCount = 1;
			pre.timestamp = now();
			pre.descr.w = L"";
			pre.files.w = &name;
			pre.lParam = (LPARAM)transfer;
			ProtoChainRecvFile(hContact, &pre);
		}
		break;

	default:
		proto->debugLogA(__FUNCTION__": unsupported transfer (%d) from %s (%d) with type (%d)", fileNumber, (const char*)pubKey, friendNumber, kind);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		break;
	}
}

// file request is allowed
HANDLE CToxProto::OnFileAllow(Tox *tox, MCONTACT hContact, HANDLE hTransfer, const wchar_t *tszPath)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	transfer->pfts.szWorkingDir.w = mir_wstrdup(tszPath);

	// stupid fix
	wchar_t fullPath[MAX_PATH];
	mir_snwprintf(fullPath, L"%s\\%s", transfer->pfts.szWorkingDir.w, transfer->pfts.szCurrentFile.w);
	transfer->ChangeName(fullPath);

	if (!ProtoBroadcastAck(hContact, ACKTYPE_FILE, ACKRESULT_FILERESUME, (HANDLE)transfer, (LPARAM)&transfer->pfts))
		OnFileResume(tox, hTransfer, FILERESUME_OVERWRITE, fullPath);

	return hTransfer;
}

// if file is exists
int CToxProto::OnFileResume(Tox *tox, HANDLE hTransfer, int action, const wchar_t *szFilename)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;

	if (action == FILERESUME_SKIP) {
		tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer);
		transfers.Remove(transfer);
		return 0;
	}

	if (action == FILERESUME_RENAME)
		transfer->ChangeName(szFilename);

	ToxHexAddress pubKey = GetContactPublicKey(tox, transfer->friendNumber);

	if (!transfer->Resume()) {
		debugLogA(__FUNCTION__": failed to open file (%d) from %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
		tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		transfers.Remove(transfer);
		return 0;
	}

	TOX_ERR_FILE_CONTROL error;
	debugLogA(__FUNCTION__": start receiving file (%d) from %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
	if (!tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_RESUME, &error)) {
		debugLogA(__FUNCTION__": failed to start receiving of file (%d) from %s (%d) cause (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber, error);
		tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		transfers.Remove(transfer);
	}

	return 0;
}

// getting the file data
void CToxProto::OnDataReceiving(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, const uint8_t *data, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxHexAddress pubKey = proto->GetContactPublicKey(tox, friendNumber);

	FileTransferParam *transfer = proto->transfers.Get(friendNumber, fileNumber);
	if (transfer == nullptr) {
		proto->debugLogA(__FUNCTION__": failed to find transfer (%d) from %s (%d)", fileNumber, (const char*)pubKey, friendNumber);
		return;
	}

	//receiving is finished
	if (length == 0 || position == UINT64_MAX) {
		proto->OnTransferCompleted(tox, transfer);
		return;
	}

	MCONTACT hContact = proto->GetContact(tox, friendNumber);
	if (hContact == NULL) {
		proto->debugLogA(__FUNCTION__": cannot find contact %s (%d)", (const char*)pubKey, friendNumber);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		return;
	}

	uint64_t filePos = _ftelli64(transfer->hFile);
	if (filePos != position && !_fseeki64(transfer->hFile, position, SEEK_SET)) {
		proto->debugLogA(__FUNCTION__": failed seek into file (%d)", fileNumber);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		return;
	}

	if (fwrite(data, sizeof(uint8_t), length, transfer->hFile) != length) {
		proto->debugLogA(__FUNCTION__": failed write to file (%d)", fileNumber);
		tox_file_control(proto->m_tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		return;
	}

	transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += length;
	proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
}

void CToxProto::OnTransferCompleted(Tox *tox, FileTransferParam *transfer)
{
	ToxHexAddress pubKey = GetContactPublicKey(tox, transfer->friendNumber);

	debugLogA(__FUNCTION__": finised the transfer of file (%d) from %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
	bool isFullyTransfered = transfer->pfts.currentFileProgress == transfer->pfts.currentFileSize;
	if (!isFullyTransfered)
		debugLogA(__FUNCTION__": file (%d) from %s (%d) is transferred not completely", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);

	if (transfer->transferType == TOX_FILE_KIND_AVATAR) {
		OnGotFriendAvatarData((AvatarTransferParam*)transfer);
		return;
	}

	ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, isFullyTransfered ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)transfer);
	transfers.Remove(transfer);
}

void CToxProto::OnFileRequest(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, TOX_FILE_CONTROL control, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	FileTransferParam *transfer = proto->transfers.Get(friendNumber, fileNumber);
	if (transfer == nullptr) {
		proto->debugLogA(__FUNCTION__": failed to find transfer (%d)", fileNumber);
		return;
	}

	ToxHexAddress pubKey = proto->GetContactPublicKey(tox, friendNumber);

	MCONTACT hContact = proto->GetContact(tox, friendNumber);
	if (hContact == NULL) {
		proto->debugLogA(__FUNCTION__": cannot find contact %s (%d)", (const char*)pubKey, friendNumber);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		return;
	}

	switch (control) {
	case TOX_FILE_CONTROL_PAUSE:
		proto->debugLogA(__FUNCTION__": received ask to pause the transfer of file (%d) from %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
		transfer->Pause();
		break;

	case TOX_FILE_CONTROL_RESUME:
		proto->debugLogA(__FUNCTION__": received ask to resume the transfer of file (%d) from %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
		if (!transfer->Resume()) {
			proto->debugLogA(__FUNCTION__": failed to resume file (%d) from %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
			tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
			proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
			proto->transfers.Remove(transfer);
		}
		break;

	case TOX_FILE_CONTROL_CANCEL:
		proto->debugLogA(__FUNCTION__": received ask to cancel the transfer of file (%d) from %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		break;
	}
}

/* FILE SENDING */

// outcoming file flow
HANDLE CToxProto::OnSendFile(Tox *tox, MCONTACT hContact, const wchar_t*, wchar_t **ppszFiles)
{
	int32_t friendNumber = GetToxFriendNumber(hContact);
	if (friendNumber == UINT32_MAX)
		return nullptr;

	FILE *hFile = _wfopen(ppszFiles[0], L"rb");
	if (hFile == nullptr) {
		debugLogA(__FUNCTION__": cannot open file %s", ppszFiles[0]);
		return nullptr;
	}

	wchar_t *fileName = wcsrchr(ppszFiles[0], '\\') + 1;
	size_t fileDirLength = fileName - ppszFiles[0];
	wchar_t *fileDir = (wchar_t*)mir_alloc(sizeof(wchar_t)*(fileDirLength + 1));
	wcsncpy(fileDir, ppszFiles[0], fileDirLength);
	fileDir[fileDirLength] = '\0';

	_fseeki64(hFile, 0, SEEK_END);
	uint64_t fileSize = _ftelli64(hFile);
	rewind(hFile);

	ToxHexAddress pubKey = GetContactPublicKey(tox, friendNumber);

	T2Utf rawName(fileName);
	TOX_ERR_FILE_SEND sendError;
	uint32_t fileNumber = tox_file_send(tox, friendNumber, TOX_FILE_KIND_DATA, fileSize, nullptr, (uint8_t*)(char*)rawName, mir_strlen(rawName), &sendError);
	if (sendError != TOX_ERR_FILE_SEND_OK) {
		debugLogA(__FUNCTION__": failed to send file (%d) to %s(%d) cause (%d)", fileNumber, (const char*)pubKey, friendNumber, sendError);
		mir_free(fileDir);
		return nullptr;
	}
	debugLogA(__FUNCTION__": start sending file (%d) to %s (%d)", fileNumber, (const char*)pubKey, friendNumber);

	FileTransferParam *transfer = new FileTransferParam(friendNumber, fileNumber, fileName, fileSize);
	transfer->pfts.flags |= PFTS_SENDING;
	transfer->pfts.hContact = hContact;
	transfer->pfts.szWorkingDir.w = fileDir;
	transfer->hFile = hFile;
	transfers.Add(transfer);

	return (HANDLE)transfer;
}

void CToxProto::OnFileSendData(Tox *tox, uint32_t friendNumber, uint32_t fileNumber, uint64_t position, size_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxHexAddress pubKey = proto->GetContactPublicKey(tox, friendNumber);

	FileTransferParam *transfer = proto->transfers.Get(friendNumber, fileNumber);
	if (!transfer) {
		proto->debugLogA(__FUNCTION__": failed to find transfer (%d) to %s (%d)", fileNumber, (const char*)pubKey, friendNumber);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		return;
	}

	if (length == 0) {
		// file sending is finished
		proto->debugLogA(__FUNCTION__": finised the transfer of file (%d) to %s (%d)", fileNumber, (const char*)pubKey, friendNumber);
		bool isFullyTransfered = transfer->pfts.currentFileProgress == transfer->pfts.currentFileSize;
		if (!isFullyTransfered)
			proto->debugLogA(__FUNCTION__": file (%d) is not completely transferred to %s (%d)", fileNumber, (const char*)pubKey, friendNumber);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, isFullyTransfered ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		return;
	}

	uint64_t sentBytes = _ftelli64(transfer->hFile);
	if (sentBytes != position && !_fseeki64(transfer->hFile, position, SEEK_SET)) {
		proto->debugLogA(__FUNCTION__": failed seek into file (%d)", fileNumber);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		return;
	}

	mir_ptr<uint8_t> data((uint8_t*)mir_alloc(length));
	if (fread(data, sizeof(uint8_t), length, transfer->hFile) != length) {
		proto->debugLogA(__FUNCTION__": failed to read from file (%d) to %s (%d)", fileNumber, (const char*)pubKey, friendNumber);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		return;
	}

	TOX_ERR_FILE_SEND_CHUNK error;
	if (!tox_file_send_chunk(proto->m_tox, friendNumber, fileNumber, position, data, length, &error)) {
		if (error == TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_CONNECTED)
			return;
		proto->debugLogA(__FUNCTION__": failed to send file chunk (%d) to %s (%d) cause (%d)", fileNumber, (const char*)pubKey, friendNumber, error);
		tox_file_control(tox, friendNumber, fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, (HANDLE)transfer);
		proto->transfers.Remove(transfer);
		return;
	}

	transfer->pfts.totalProgress = transfer->pfts.currentFileProgress += length;
	proto->ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DATA, (HANDLE)transfer, (LPARAM)&transfer->pfts);
}

/* COMMON */

int CToxProto::CancelTransfer(MCONTACT, HANDLE hTransfer)
{
	FileTransferParam *transfer = (FileTransferParam*)hTransfer;
	debugLogA(__FUNCTION__": transfer (%d) is canceled", transfer->fileNumber);

	if (IsOnline())
		tox_file_control(m_tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);

	transfers.Remove(transfer);
	return 0;
}

void CToxProto::PauseOutgoingTransfers(uint32_t friendNumber)
{
	for (size_t i = 0; i < transfers.Count(); i++) {
		// only for sending
		FileTransferParam *transfer = transfers.GetAt(i);
		if (transfer->friendNumber == friendNumber && transfer->GetDirection() == 0) {
			ToxHexAddress pubKey = GetContactPublicKey(m_tox, friendNumber);

			debugLogA(__FUNCTION__": sending ask to pause the transfer of file (%d) to %s (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
			TOX_ERR_FILE_CONTROL error;
			if (!tox_file_control(m_tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_PAUSE, &error)) {
				debugLogA(__FUNCTION__": failed to pause the transfer (%d) to %s (%d) cause(%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber, error);
				tox_file_control(m_tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
			}
		}
	}
}

void CToxProto::ResumeIncomingTransfers(uint32_t friendNumber)
{
	for (size_t i = 0; i < transfers.Count(); i++) {
		// only for receiving
		FileTransferParam *transfer = transfers.GetAt(i);
		if (transfer->friendNumber == friendNumber && transfer->GetDirection() == 1) {
			ToxHexAddress pubKey = GetContactPublicKey(m_tox, friendNumber);

			debugLogA(__FUNCTION__": sending ask to resume the transfer of file (%d) from %s (%d) cause (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber);
			TOX_ERR_FILE_CONTROL error;
			if (!tox_file_control(m_tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_RESUME, &error)) {
				debugLogA(__FUNCTION__": failed to resume the transfer (%d) from %s (%d) cause (%d)", transfer->fileNumber, (const char*)pubKey, transfer->friendNumber, error);
				CancelTransfer(NULL, transfer);
			}
		}
	}
}

void CToxProto::CancelAllTransfers(Tox *tox)
{
	debugLogA(__FUNCTION__": canceling all transfers");
	for (size_t i = 0; i < transfers.Count(); i++) {
		FileTransferParam *transfer = transfers.GetAt(i);
		tox_file_control(tox, transfer->friendNumber, transfer->fileNumber, TOX_FILE_CONTROL_CANCEL, nullptr);
		ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_FILE, ACKRESULT_DENIED, (HANDLE)transfer);
		transfers.Remove(transfer);
	}
}