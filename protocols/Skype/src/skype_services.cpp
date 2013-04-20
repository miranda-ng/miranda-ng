#include "skype_proto.h"

LIST<void> CSkypeProto::serviceList(0);

void CSkypeProto::InitServiceList()
{
	CSkypeProto::serviceList.insert(
		::CreateServiceFunction("Skype/MenuChoose", CSkypeProto::MenuChooseService));
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