#include "skype_proto.h"

bool CSkypeProto::IsAvatarChanged(const SEBinary &avatar)
{
	bool result = false;

	::mir_md5_byte_t digest[16];
	::mir_md5_hash((PBYTE)avatar.data(), (int)avatar.size(), digest);

	DBVARIANT dbv;
	::db_get(NULL, this->m_szModuleName, "AvatarHash", &dbv);
	if (dbv.type == DBVT_BLOB && dbv.pbVal && dbv.cpbVal == 16)
	{
		if (::memcmp(digest, dbv.pbVal, 16) == 0)
		{
			result = true;
		}
	}
	::db_free(&dbv);

	return result;
}

int CSkypeProto::DetectAvatarFormatBuffer(const char *pBuffer)
{
	if (!strncmp(pBuffer, "%PNG", 4))
		return PA_FORMAT_PNG;

	if (!strncmp(pBuffer, "GIF8", 4))
		return PA_FORMAT_GIF;

	if (!_strnicmp(pBuffer, "<?xml", 5))
		return PA_FORMAT_XML;

	if ((((DWORD*)pBuffer)[0] == 0xE0FFD8FFul) || (((DWORD*)pBuffer)[0] == 0xE1FFD8FFul))
		return PA_FORMAT_JPEG;

	if (!strncmp(pBuffer, "BM", 2))
		return PA_FORMAT_BMP;

	return PA_FORMAT_UNKNOWN;
}

int CSkypeProto::DetectAvatarFormat(const wchar_t *path)
{
	int src = _wopen(path, _O_BINARY | _O_RDONLY, 0);
	if (src == -1)
		return PA_FORMAT_UNKNOWN;

	char pBuf[32];
	_read(src, pBuf, 32);
	_close(src);

	return CSkypeProto::DetectAvatarFormatBuffer(pBuf);
}

wchar_t* CSkypeProto::GetContactAvatarFilePath(HANDLE hContact)
{
	wchar_t* path = new wchar_t[MAX_PATH];

	this->InitCustomFolders();

	if (m_hAvatarsFolder == NULL || FoldersGetCustomPathT(m_hAvatarsFolder, path, MAX_PATH, _T("")))
	{
		wchar_t *tmpPath = ::Utils_ReplaceVarsT(L"%miranda_avatarcache%");
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%S"), tmpPath, this->m_szModuleName);
		::mir_free(tmpPath);
	}

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	::mir_ptr<wchar_t> sid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN));
	if (hContact != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s.jpg"), path, sid);
	else if (sid != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.jpg"), path, sid);
	else
	{
		delete [] path;
		return NULL;
	}

	return path;
}

INT_PTR __cdecl CSkypeProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONW *pai = (PROTO_AVATAR_INFORMATIONW*)lParam;

	if (this->GetSettingWord(pai->hContact, "AvatarTS"))
	{
		return GAIR_NOAVATAR;
	}

	wchar_t *sid = ::db_get_wsa(pai->hContact, this->m_szModuleName, SKYPE_SETTINGS_LOGIN);
	if (sid)
	{
		wchar_t *path = this->GetContactAvatarFilePath(pai->hContact);
		if (path && !_waccess(path, 0))
		{
			::wcsncpy(pai->filename, path, SIZEOF(pai->filename));
			pai->format = PA_FORMAT_JPEG;
			return GAIR_SUCCESS;
		}

		::mir_free(sid);
	}

	return GAIR_NOAVATAR;
}

INT_PTR __cdecl CSkypeProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case AF_MAXSIZE:
	{
		POINT *size = (POINT*)lParam;
		if (size)
		{
			size->x = 96;
			size->y = 96;
		}
	}
	break;
	
	case AF_PROPORTION:
		return PIP_NONE;

	case AF_FORMATSUPPORTED:
		return lParam == PA_FORMAT_JPEG;

	case AF_ENABLED:
			return 1;
	
	case AF_DONTNEEDDELAYS:
		return 1;

	case AF_MAXFILESIZE:
		// server accepts images of 32000 bytees, not bigger
		return 32000;
	
	case AF_DELAYAFTERFAIL:
		// do not request avatar again if server gave an error
		return 1;// * 60 * 60 * 1000; // one hour
	
	case AF_FETCHALWAYS:
		// avatars can be fetched all the time (server only operation)
		return 1;
	}

	return 0;
}

INT_PTR __cdecl CSkypeProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam) return -2;

	wchar_t *path = this->GetContactAvatarFilePath(NULL);
	if (path && ::PathFileExists(path)) 
	{
		::wcsncpy((wchar_t *)wParam, path, (int)lParam);
		delete path;
		return 0;
	}

	return -1;
}

INT_PTR __cdecl CSkypeProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *path = (wchar_t *)lParam;
	int iRet = -1;

	if (path)
	{
		wchar_t *avatarPath = this->GetContactAvatarFilePath(NULL);
		if (::wcscmp(path, avatarPath) && !::CopyFile(path, avatarPath, FALSE))
		{
			this->Log(L"Failed to copy our avatar to local storage.");
			return iRet;
		}
		
		SEBinary avatar = this->GetAvatarBinary(avatarPath);
		if (avatar.size() == 0)
		{
			this->Log(L"Failed to read avatar file.");
			return iRet;
		}

		if (!this->IsAvatarChanged(avatar))
		{
			this->Log(L"New avatar are same with old.");
			return iRet;
		}

		Skype::VALIDATERESULT result = Skype::NOT_VALIDATED;
		if (!this->account->SetAvatar(avatar, result))
		{
			this->Log(CSkypeProto::ValidationReasons[result]);
			return iRet;
		}

		uint newTS = this->account->GetUintProp(/* *::P_AVATAR_TIMESTAMP */ 182);
		this->SetSettingDword("AvatarTS", newTS);
		iRet = 0;
	}
	else
	{
		this->account->SetBinProperty(Account::P_AVATAR_IMAGE, SEBinary());
		this->DeleteSetting("AvatarTS");
		iRet = 0;
	}

	return iRet;
}