#include "common.h"

TCHAR* CToxProto::GetContactAvatarFilePath(MCONTACT hContact)
{
	TCHAR path[MAX_PATH];
	mir_sntprintf(path, SIZEOF(path), _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	ptrA id(getStringA(hContact, TOX_SETTINGS_ID));
	if (hContact != NULL)
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s.jpg"), path, id);
	else if (id != NULL)
		mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.jpg"), path, id);
	else
		return NULL;

	return mir_tstrdup(path);
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
			size->x = -1;
			size->y = -1;
		}
	}
		break;

	case AF_PROPORTION:
		return PIP_NONE;

	case AF_FORMATSUPPORTED:
		return lParam == PA_FORMAT_PNG;

	case AF_ENABLED:
		return 1;

	case AF_DONTNEEDDELAYS:
		return 1;

	case AF_MAXFILESIZE:
		return TOX_MAX_AVATAR_DATA_LENGTH;

	case AF_DELAYAFTERFAIL:
		// do not request avatar again if server gave an error
		return 1;// * 60 * 60 * 1000; // one hour

	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		// avatars can be fetched all the time (server only operation)
		return 1;
	}

	return 0;
}

INT_PTR CToxProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONW *pai = (PROTO_AVATAR_INFORMATIONW *)lParam;

	ptrA id(getStringA(pai->hContact, TOX_SETTINGS_ID));
	if (id != NULL)
	{
		ptrT path(GetContactAvatarFilePath(pai->hContact));
		if (path && !_waccess(path, 0))
		{
			_tcsncpy(pai->filename, path, SIZEOF(pai->filename));
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

	std::tstring path(GetContactAvatarFilePath(NULL));
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
	if (path != NULL)
	{
		ptrT avatarPath(GetContactAvatarFilePath(NULL));
		if (!_tcscmp(path, avatarPath))
		{
			debugLogA("CToxProto::SetMyAvatar: new avatar path are same with old");
			return -1;
		}

		if (!CopyFile(path, avatarPath, FALSE))
		{
			debugLogA("CToxProto::SetMyAvatar: failed to copy new avatar to local storage");
			return -1;
		}

		int length;
		uint8_t *data;
		FILE *hFile = _tfopen(avatarPath, L"rb");
		if (!hFile)
		{
			debugLogA("CToxProto::SetMyAvatar: failed to open avatar file");
			return -1;
		}

		fseek(hFile, 0, SEEK_END);
		length = ftell(hFile);
		fseek(hFile, 0, SEEK_SET);
		if (length > TOX_MAX_AVATAR_DATA_LENGTH)
		{
			fclose(hFile);
			debugLogA("CToxProto::SetMyAvatar: new avatar size is excessive");
			return -1;
		}

		data = new uint8_t[length + 1];
		if (fread(data, length, 1, hFile) != length)
		{
			fclose(hFile);
			debugLogA("CToxProto::SetMyAvatar: failed to read avatar file");
			return -1;
		}
		fclose(hFile);

		DBVARIANT dbv;
		uint8_t hash[TOX_AVATAR_HASH_LENGTH];
		tox_avatar_hash(tox, &hash[0], data, length);		
		if (!db_get(NULL, m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv))
		{
			if (memcmp(hash, dbv.pbVal, TOX_AVATAR_HASH_LENGTH) == 0)
			{
				db_free(&dbv);
				delete data;
				debugLogW(L"CToxProto::SetMyAvatar: new avatar is same with old");
				return -1;
			}
			db_free(&dbv);
		}

		if (tox_set_avatar(tox, TOX_AVATARFORMAT_PNG, data, length) == TOX_ERROR)
		{
			delete data;
			debugLogA("CToxProto::SetMyAvatar: failed to set new avatar");
			return -1;
		}

		delete data;
	}

	return 0;
}

void CToxProto::OnGotFriendAvatarInfo(Tox *tox, int32_t number, uint8_t format, uint8_t *hash, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	MCONTACT hContact = proto->FindContact(number);
	if (hContact)
	{
		TCHAR *path = proto->GetContactAvatarFilePath(hContact);
		if (format == TOX_AVATARFORMAT_NONE)
		{
			proto->delSetting(hContact, TOX_SETTINGS_AVATAR_HASH);
			DeleteFile(path);
		}
		else
		{
			DBVARIANT dbv;
			if (!db_get(hContact, proto->m_szModuleName, TOX_SETTINGS_AVATAR_HASH, &dbv))
			{
				if (memcmp(hash, dbv.pbVal, TOX_AVATAR_HASH_LENGTH) != 0)
				{
					tox_request_avatar_data(proto->tox, number);
				}
				db_free(&dbv);
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
		db_set_blob(hContact, proto->m_szModuleName, TOX_SETTINGS_AVATAR_HASH, hash, TOX_AVATAR_HASH_LENGTH);

		TCHAR *path = proto->GetContactAvatarFilePath(hContact);
		FILE *hFile = _tfopen(path, L"wb");
		if (hFile)
		{
			fwrite(data, sizeof(uint8_t), length, hFile);
			fclose(hFile);
		}
		mir_free(path);
	}
}