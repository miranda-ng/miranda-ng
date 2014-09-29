#include "common.h"

std::tstring CToxProto::GetAvatarFilePath(MCONTACT hContact)
{
	TCHAR path[MAX_PATH];
	mir_sntprintf(path, SIZEOF(path), _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	ptrT id(getTStringA(hContact, TOX_SETTINGS_ID));
	if (hContact != NULL)
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s.png"), path, id);
	else if (id != NULL)
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.png"), path, id);
	else
		return _T("");

	return path;
}

bool CToxProto::SetToxAvatar(std::tstring path, bool checkHash)
{
	int length;
	uint8_t *data;
	FILE *hFile = _tfopen(path.c_str(), L"rb");
	if (!hFile)
	{
		debugLogA("CToxProto::SetMyAvatar: failed to open avatar file");
		return false;
	}

	fseek(hFile, 0, SEEK_END);
	length = ftell(hFile);
	rewind(hFile);
	if (length > TOX_AVATAR_MAX_DATA_LENGTH)
	{
		fclose(hFile);
		debugLogA("CToxProto::SetMyAvatar: new avatar size is excessive");
		return false;
	}

	data = (uint8_t*)mir_alloc(length);
	size_t readed = fread(data, sizeof(uint8_t), length, hFile);
	if (readed != length)
	{
		fclose(hFile);
		debugLogA("CToxProto::SetMyAvatar: failed to read avatar file");
		return false;
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
			debugLogA("CToxProto::SetMyAvatar: new avatar is same with old");
			return false;
		}
		db_free(&dbv);
	}

	if (tox_set_avatar(tox, TOX_AVATAR_FORMAT_PNG, data, length) == TOX_ERROR)
	{
		mir_free(data);
		debugLogA("CToxProto::SetMyAvatar: failed to set new avatar");
		return false;
	}
	mir_free(data);

	if (checkHash)
	{
		db_set_blob(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, (void*)hash, TOX_HASH_LENGTH);
	}

	return true;
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
		return TOX_AVATAR_MAX_DATA_LENGTH;
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

INT_PTR CToxProto::SetMyAvatar(WPARAM wParam, LPARAM lParam)
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

		SetToxAvatar(avatarPath, true);
	}
	else
	{
		if (tox_unset_avatar(tox) == TOX_ERROR)
		{
			debugLogA("CToxProto::SetMyAvatar: failed to unset avatar");
			return -1;
		}

		if (IsFileExists(avatarPath))
		{
			DeleteFile(avatarPath.c_str());
		}

		db_unset(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH);
	}

	return 0;
}

void CToxProto::OnGotFriendAvatarInfo(Tox *tox, int32_t number, uint8_t format, uint8_t *hash, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		std::tstring path = proto->GetAvatarFilePath(hContact);
		if (format == TOX_AVATAR_FORMAT_NONE)
		{
			proto->delSetting(hContact, TOX_SETTINGS_AVATAR_HASH);
			proto->ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, 0, 0);
			if (IsFileExists(path))
			{
				DeleteFile(path.c_str());
			}
		}
		else
		{
			DBVARIANT dbv;
			if (!db_get(hContact, proto->m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv))
			{
				if (memcmp(hash, dbv.pbVal, TOX_HASH_LENGTH) != 0)
				{
					tox_request_avatar_data(proto->tox, number);
				}
				db_free(&dbv);
			}
			else
			{
				tox_request_avatar_data(proto->tox, number);
			}
		}
	}
}

void CToxProto::OnGotFriendAvatarData(Tox *tox, int32_t number, uint8_t format, uint8_t *hash, uint8_t *data, uint32_t length, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
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
}