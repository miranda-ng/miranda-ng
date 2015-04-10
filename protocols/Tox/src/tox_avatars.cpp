#include "common.h"

std::tstring CToxProto::GetAvatarFilePath(MCONTACT hContact)
{
	TCHAR path[MAX_PATH];
	mir_sntprintf(path, SIZEOF(path), _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);
	}

	ptrT id(getTStringA(hContact, TOX_SETTINGS_ID));
	if (hContact != NULL)
	{
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s.png"), path, id);
	}
	else if (id != NULL)
	{
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.png"), path, id);
	}
	else
	{
		return _T("");
	}

	return path;
}

void CToxProto::SetToxAvatar(std::tstring path, bool checkHash)
{
	FILE *hFile = _tfopen(path.c_str(), L"rb");
	if (!hFile)
	{
		debugLogA(__FUNCTION__": failed to open avatar file");
		return;
	}

	fseek(hFile, 0, SEEK_END);
	size_t length = ftell(hFile);
	rewind(hFile);
	if (length > 1024 * 1024)
	{
		fclose(hFile);
		debugLogA(__FUNCTION__": new avatar size is excessive");
		return;
	}

	uint8_t *data = (uint8_t*)mir_alloc(length);
	if (fread(data, sizeof(uint8_t), length, hFile) != length)
	{
		fclose(hFile);
		debugLogA(__FUNCTION__": failed to read avatar file");
		return;
	}
	fclose(hFile);

	DBVARIANT dbv;
	uint8_t hash[TOX_HASH_LENGTH];
	tox_hash(hash, data, TOX_HASH_LENGTH);
	if (checkHash && !db_get(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv))
	{
		if (memcmp(hash, dbv.pbVal, TOX_HASH_LENGTH) == 0)
		{
			db_free(&dbv);
			mir_free(data);
			debugLogA(__FUNCTION__": new avatar is same with old");
			return;
		}
		db_free(&dbv);
	}

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
	{
		if (GetContactStatus(hContact) == ID_STATUS_OFFLINE)
			continue;

		int32_t friendNumber = GetToxFriendNumber(hContact);
		if (friendNumber == UINT32_MAX)
		{
			mir_free(data);
			debugLogA(__FUNCTION__": failed to set new avatar");
			return;
		}

		TOX_ERR_FILE_SEND error;
		uint32_t fileNumber = tox_file_send(tox, friendNumber, TOX_FILE_KIND_AVATAR, length, NULL, hash, TOX_HASH_LENGTH, &error);
		if (error != TOX_ERR_FILE_SEND_OK)
		{
			mir_free(data);
			debugLogA(__FUNCTION__": failed to set new avatar");
			return;
		}

		FileTransferParam *transfer = new FileTransferParam(friendNumber, fileNumber, _T("avatar"), length);
		transfer->pfts.hContact = hContact;
		transfer->hFile = _tfopen(path.c_str(), L"rb");
		transfers.Add(transfer);
	}

	mir_free(data);

	if (checkHash)
	{
		db_set_blob(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, (void*)hash, TOX_HASH_LENGTH);
	}
}

INT_PTR CToxProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case AF_MAXSIZE:
	{
		POINT *size = (POINT *)lParam;
		if (size)
		{
			size->x = 300;
			size->y = 300;
		}
	}
	break;

	case AF_ENABLED:
		return 1;

	case AF_FORMATSUPPORTED:
		return lParam == PA_FORMAT_PNG;

	case AF_MAXFILESIZE:
		return 1024 * 1024;
	}

	return 0;
}

INT_PTR CToxProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONW *pai = (PROTO_AVATAR_INFORMATIONW *)lParam;

	ptrA id(getStringA(pai->hContact, TOX_SETTINGS_ID));
	if (id != NULL)
	{
		std::tstring path = GetAvatarFilePath(pai->hContact);
		if (IsFileExists(path))
		{
			_tcsncpy(pai->filename, path.c_str(), SIZEOF(pai->filename));
			pai->format = PA_FORMAT_PNG;

			return GAIR_SUCCESS;
		}
	}

	return GAIR_NOAVATAR;
}

INT_PTR CToxProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
	{
		return -2;
	}

	std::tstring path(GetAvatarFilePath());
	if (IsFileExists(path))
	{
		_tcsncpy((TCHAR*)wParam, path.c_str(), (int)lParam);

		return 0;
	}

	return -1;
}

INT_PTR CToxProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	TCHAR *path = (TCHAR*)lParam;
	std::tstring avatarPath = GetAvatarFilePath();
	if (path != NULL)
	{
		if (!CopyFile(path, avatarPath.c_str(), FALSE))
		{
			debugLogA("CToxProto::SetMyAvatar: failed to copy new avatar to avatar cache");
			return -1;
		}

		if (IsOnline())
			SetToxAvatar(avatarPath, true);
	}
	else
	{
		if (IsOnline())
		{
			for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName))
			{
				if (GetContactStatus(hContact) == ID_STATUS_OFFLINE)
					continue;

				int32_t friendNumber = GetToxFriendNumber(hContact);
				if (friendNumber == UINT32_MAX)
				{
					debugLogA(__FUNCTION__": failed to unset avatar");
					return -1;
				}

				TOX_ERR_FILE_SEND error;
				if (!tox_file_send(tox, NULL, TOX_FILE_KIND_AVATAR, 0, NULL, NULL, 0, &error))
				{
					debugLogA(__FUNCTION__": failed to unset avatar");
					return -1;
				}
			}
		}

		if (IsFileExists(avatarPath))
		{
			DeleteFile(avatarPath.c_str());
		}

		db_unset(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH);
	}

	return 0;
}

void CToxProto::OnGotFriendAvatarInfo(FileTransferParam *transfer, const uint8_t *hash)
{
	MCONTACT hContact = transfer->pfts.hContact;
	std::tstring path = GetAvatarFilePath();
	if (transfer->pfts.totalBytes == 0)
	{
		delSetting(hContact, TOX_SETTINGS_AVATAR_HASH);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, 0, 0);
		if (IsFileExists(path))
		{
			DeleteFile(path.c_str());
		}
		OnFileCancel(hContact, transfer);
	}
	else
	{
		DBVARIANT dbv;
		if (!db_get(transfer->pfts.hContact, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv))
		{
			if (memcmp(hash, dbv.pbVal, TOX_HASH_LENGTH) == 0)
			{
				db_free(&dbv);
				OnFileCancel(hContact, transfer);
				return;
			}
			db_free(&dbv);
		}
		OnFileAllow(hContact, transfer, path.c_str());
	}
}

/*void CToxProto::OnGotFriendAvatarData(Tox *, int32_t number, uint8_t, uint8_t *hash, uint8_t *data, uint32_t length, void *arg)
{
CToxProto *proto = (CToxProto*)arg;

MCONTACT hContact = proto->GetContact(number);
if (hContact)
{
db_set_blob(hContact, proto->m_szModuleName, TOX_SETTINGS_AVATAR_HASH, hash, TOX_HASH_LENGTH);

std::tstring path = proto->GetAvatarFilePath(hContact);
FILE *hFile = _tfopen(path.c_str(), L"wb");
if (hFile)
{
if (fwrite(data, sizeof(uint8_t), length, hFile) == length)
{
PROTO_AVATAR_INFORMATIONW pai = { sizeof(pai) };
pai.format = PA_FORMAT_PNG;
pai.hContact = hContact;
_tcscpy(pai.filename, path.c_str());

proto->ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&pai, 0);
}
fclose(hFile);
}
}
}*/