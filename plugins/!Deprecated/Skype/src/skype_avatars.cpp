#include "skype.h"

bool CSkypeProto::IsAvatarChanged(const SEBinary &avatar, MCONTACT hContact)
{
	bool result = false;

	BYTE digest[16];
	::mir_md5_hash((PBYTE)avatar.data(), (int)avatar.size(), digest);

	DBVARIANT dbv;
	::db_get(hContact, this->m_szModuleName, "AvatarHash", &dbv);
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

wchar_t * CSkypeProto::GetContactAvatarFilePath(MCONTACT hContact)
{
	TCHAR path[MAX_PATH];
	::mir_sntprintf(path, SIZEOF(path), _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), this->m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(path);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CallService(MS_UTILS_CREATEDIRTREET, 0, (LPARAM)path);

	ptrW sid(::db_get_wsa(hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));
	if (hContact != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s.jpg"), path, sid);
	else if (sid != NULL)
		::mir_sntprintf(path, MAX_PATH, _T("%s\\%s avatar.jpg"), path, sid);
	else
		return NULL;

	return mir_wstrdup(path);
}

INT_PTR __cdecl CSkypeProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONW *pai = (PROTO_AVATAR_INFORMATIONW *)lParam;

	if (this->getDword(pai->hContact, "AvatarTS", 0))
		return GAIR_NOAVATAR;

	ptrW sid( ::db_get_wsa(pai->hContact, this->m_szModuleName, SKYPE_SETTINGS_SID));
	if (sid)
	{
		ptrW path( this->GetContactAvatarFilePath(pai->hContact));
		if (path && !_waccess(path, 0))
		{
			::wcsncpy(pai->filename, path, SIZEOF(pai->filename));
			pai->format = PA_FORMAT_JPEG;
			return GAIR_SUCCESS;
		}
	}

	return GAIR_NOAVATAR;
}

INT_PTR __cdecl CSkypeProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case AF_MAXSIZE:
	{
		POINT *size = (POINT *)lParam;
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
	
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		// avatars can be fetched all the time (server only operation)
		return 1;
	}

	return 0;
}

INT_PTR __cdecl CSkypeProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
		return -2;

	mir_ptr<wchar_t> path( this->GetContactAvatarFilePath(NULL));
	if (path && CSkypeProto::FileExists(path)) 
	{
		::wcsncpy((wchar_t *)wParam, path, (int)lParam);
		return 0;
	}

	return -1;
}

INT_PTR __cdecl CSkypeProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *path = (wchar_t *)lParam;
	if (path)
	{
		ptrW avatarPath( this->GetContactAvatarFilePath(NULL));
		if ( !::wcscmp(path, avatarPath))
		{
			this->debugLogW(L"New avatar path are same with old.");
			return -1;
		}

		SEBinary avatar = this->GetAvatarBinary(path);
		if (avatar.size() == 0)
		{
			this->debugLogW(L"Failed to read avatar file.");
			return -1;
		}

		if (this->IsAvatarChanged(avatar))
		{
			this->debugLogW(L"New avatar are same with old.");
			return -1;
		}

		if ( !::CopyFile(path, avatarPath, FALSE))
		{
			this->debugLogW(L"Failed to copy new avatar to local storage.");
			return -1;
		}
		
		Skype::VALIDATERESULT result = Skype::NOT_VALIDATED;
		if (!this->account->SetAvatar(avatar, result))
		{
			this->debugLogW(CSkypeProto::ValidationReasons[result]);
			return -1;
		}

		uint newTS = this->account->GetUintProp(Account::P_AVATAR_IMAGE);
		this->setDword("AvatarTS", newTS);
		return 0;
	}

	this->account->SetBinProperty(Account::P_AVATAR_IMAGE, SEBinary());
	this->delSetting("AvatarTS");
	return 0;
}

SEBinary CSkypeProto::GetAvatarBinary(wchar_t *path)
{
	SEBinary avatar;

	if (CSkypeProto::FileExists(path))
	{
		int len;
		char *buffer;
		FILE* fp = ::_wfopen(path, L"rb");
		if (fp)
		{
			::fseek(fp, 0, SEEK_END);
			len = ::ftell(fp);
			::fseek(fp, 0, SEEK_SET);
			buffer = new char[len + 1];
			::fread(buffer, len, 1, fp);
			::fclose(fp);

			avatar.set(buffer, len);
		}
	}

	return avatar;
}