#include "stdafx.h"

wchar_t* CToxProto::GetAvatarFilePath(MCONTACT hContact)
{
	wchar_t *path = (wchar_t*)mir_calloc(MAX_PATH * sizeof(wchar_t) + 1);
	mir_snwprintf(path, MAX_PATH, L"%s\\%S", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
	CreateDirectoryTreeW(path);

	ptrW address(getWStringA(hContact, TOX_SETTINGS_ID));
	if (address == NULL) {
		mir_free(path);
		return mir_wstrdup(L"");
	}

	if (hContact && mir_wstrlen(address) > TOX_PUBLIC_KEY_SIZE * 2)
		address[TOX_PUBLIC_KEY_SIZE * 2] = 0;

	mir_snwprintf(path, MAX_PATH, L"%s\\%s.png", path, address.get());
	return path;
}

void CToxProto::SetToxAvatar(const wchar_t* path)
{
	FILE *hFile = _wfopen(path, L"rb");
	if (!hFile) {
		debugLogA(__FUNCTION__": failed to open avatar file");
		return;
	}

	fseek(hFile, 0, SEEK_END);
	size_t length = ftell(hFile);
	rewind(hFile);
	if (length > TOX_MAX_AVATAR_SIZE) {
		fclose(hFile);
		debugLogA(__FUNCTION__": new avatar size is excessive");
		return;
	}

	uint8_t *data = (uint8_t*)mir_alloc(length);
	if (fread(data, sizeof(uint8_t), length, hFile) != length) {
		fclose(hFile);
		debugLogA(__FUNCTION__": failed to read avatar file");
		mir_free(data);
		return;
	}
	fclose(hFile);

	DBVARIANT dbv;
	uint8_t hash[TOX_HASH_LENGTH];
	tox_hash(hash, data, length);
	if (!db_get(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv)) {
		if (memcmp(hash, dbv.pbVal, TOX_HASH_LENGTH) == 0) {
			db_free(&dbv);
			mir_free(data);
			debugLogA(__FUNCTION__": new avatar is same with old");
			return;
		}
		db_free(&dbv);
	}

	db_set_blob(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, (void*)hash, TOX_HASH_LENGTH);

	if (IsOnline()) {
		for (auto &hContact : AccContacts()) {
			if (GetContactStatus(hContact) == ID_STATUS_OFFLINE)
				continue;

			int32_t friendNumber = GetToxFriendNumber(hContact);
			if (friendNumber == UINT32_MAX) {
				mir_free(data);
				return;
			}

			debugLogA(__FUNCTION__": send avatar to friend (%d)", friendNumber);

			TOX_ERR_FILE_SEND error;
			uint32_t fileNumber = tox_file_send(m_tox, friendNumber, TOX_FILE_KIND_AVATAR, length, hash, nullptr, 0, &error);
			if (error != TOX_ERR_FILE_SEND_OK) {
				mir_free(data);
				debugLogA(__FUNCTION__": failed to set new avatar (%d)", error);
				return;
			}

			AvatarTransferParam *transfer = new AvatarTransferParam(friendNumber, fileNumber, nullptr, length);
			transfer->pfts.flags |= PFTS_SENDING;
			memcpy(transfer->hash, hash, TOX_HASH_LENGTH);
			transfer->pfts.hContact = hContact;
			transfer->hFile = _wfopen(path, L"rb");
			transfers.Add(transfer);
		}
	}

	mir_free(data);
}

INT_PTR CToxProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_ENABLED:
		return 1;

	case AF_FORMATSUPPORTED:
		return lParam == PA_FORMAT_PNG;

	case AF_MAXSIZE:
		((POINT*)lParam)->x = 300;
		((POINT*)lParam)->y = 300;
		return 0;

	case AF_MAXFILESIZE:
		return TOX_MAX_AVATAR_SIZE;
	}

	return 0;
}

INT_PTR CToxProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	ptrA address(getStringA(pai->hContact, TOX_SETTINGS_ID));
	if (address != NULL) {
		ptrW path(GetAvatarFilePath(pai->hContact));
		if (IsFileExists(path)) {
			mir_wstrncpy(pai->filename, path, _countof(pai->filename));
			pai->format = PA_FORMAT_PNG;

			return GAIR_SUCCESS;
		}
	}

	return GAIR_NOAVATAR;
}

INT_PTR CToxProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	ptrW path(GetAvatarFilePath());
	if (IsFileExists(path))
		mir_wstrncpy((wchar_t*)wParam, path, (int)lParam);

	return 0;
}

INT_PTR CToxProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	debugLogA(__FUNCTION__": setting avatar");
	wchar_t *path = (wchar_t*)lParam;
	ptrW avatarPath(GetAvatarFilePath());
	if (path != nullptr) {
		debugLogA(__FUNCTION__": copy new avatar");
		if (!CopyFile(path, avatarPath, FALSE)) {
			debugLogA(__FUNCTION__": failed to copy new avatar to avatar cache");
			return 0;
		}

		SetToxAvatar(avatarPath);

		return 0;
	}

	if (IsOnline()) {
		for (auto &hContact : AccContacts()) {
			if (GetContactStatus(hContact) == ID_STATUS_OFFLINE)
				continue;

			int32_t friendNumber = GetToxFriendNumber(hContact);
			if (friendNumber == UINT32_MAX)
				continue;

			debugLogA(__FUNCTION__": unset avatar for friend (%d)", friendNumber);

			TOX_ERR_FILE_SEND error;
			tox_file_send(m_tox, friendNumber, TOX_FILE_KIND_AVATAR, 0, nullptr, nullptr, 0, &error);
			if (error != TOX_ERR_FILE_SEND_OK) {
				debugLogA(__FUNCTION__": failed to unset avatar (%d)", error);
				return 0;
			}
		}
	}

	if (IsFileExists(avatarPath))
		DeleteFile(avatarPath);

	delSetting(TOX_SETTINGS_AVATAR_HASH);

	return 0;
}

void CToxProto::OnGotFriendAvatarInfo(Tox *tox, AvatarTransferParam *transfer)
{
	if (transfer->pfts.totalBytes == 0) {
		MCONTACT hConact = transfer->pfts.hContact;
		ptrW path(GetAvatarFilePath(hConact));
		if (IsFileExists(path))
			DeleteFile(path);

		transfers.Remove(transfer);
		delSetting(hConact, TOX_SETTINGS_AVATAR_HASH);
		ProtoBroadcastAck(hConact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr);
		return;
	}

	DBVARIANT dbv;
	if (!db_get(transfer->pfts.hContact, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv)) {
		if (memcmp(transfer->hash, dbv.pbVal, TOX_HASH_LENGTH) == 0) {
			db_free(&dbv);
			CancelTransfer(transfer->pfts.hContact, transfer);
			return;
		}
		db_free(&dbv);
	}

	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s\\%S", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
	OnFileAllow(tox, transfer->pfts.hContact, transfer, path);
}

void CToxProto::OnGotFriendAvatarData(AvatarTransferParam *transfer)
{
	db_set_blob(transfer->pfts.hContact, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, transfer->hash, TOX_HASH_LENGTH);

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.format = PA_FORMAT_PNG;
	ai.hContact = transfer->pfts.hContact;
	mir_wstrcpy(ai.filename, transfer->pfts.szCurrentFile.w);

	fclose(transfer->hFile);
	transfer->hFile = nullptr;

	ProtoBroadcastAck(transfer->pfts.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
	transfers.Remove(transfer);
}
