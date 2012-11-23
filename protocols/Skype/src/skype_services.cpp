#include "skype_proto.h"

LIST<void> CSkypeProto::serviceList(0);

void CSkypeProto::InitServiceList()
{
	CSkypeProto::serviceList.insert(
		::CreateServiceFunction("Skype/MenuChoose", CSkypeProto::MenuChooseService));
}

int __cdecl CSkypeProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONW *pai = (PROTO_AVATAR_INFORMATIONW*)lParam;

	if (this->GetSettingWord(pai->hContact, "AvatarTS"))
	{
		return GAIR_NOAVATAR;
	}

	wchar_t *sid = this->GetSettingString(pai->hContact, "sid");
	if (sid)
	{
		wchar_t *path = this->GetContactAvatarFilePath(sid);
		if (path && !_waccess(path, 0)) 
		{
			::wcsncpy(pai->filename, path, SIZEOF(pai->filename));
			pai->format = PA_FORMAT_JPEG;
			return GAIR_SUCCESS;
		}
	}

	return GAIR_NOAVATAR;
}

int __cdecl CSkypeProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
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
		return PIP_SQUARE;

	case AF_FORMATSUPPORTED:
		if (lParam == PA_FORMAT_JPEG)
			return 1;

	case AF_ENABLED:
			return 1;
	
	case AF_DONTNEEDDELAYS:
		break;

	case AF_MAXFILESIZE:
		// server accepts images of 7168 bytees, not bigger
		return 7168;
	
	case AF_DELAYAFTERFAIL:
		// do not request avatar again if server gave an error
		return 1;// * 60 * 60 * 1000; // one hour
	
	case AF_FETCHALWAYS:
		// avatars can be fetched all the time (server only operation)
		return 1;
	}

	return 0;
}

int __cdecl CSkypeProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam) return -2;

	wchar_t *path = this->GetContactAvatarFilePath(NULL);
	if (path && !_waccess(path, 0)) 
	{
		::wcsncpy((wchar_t *)wParam, path, (int)lParam);
		delete path;
		return 0;
	}

	delete path;
	return -1;
}

int __cdecl CSkypeProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *path = (wchar_t *)lParam;
	int iRet = -1;

	if (path)
	{
		int dwPaFormat = CSkypeProto::DetectAvatarFormat(path);
		if (dwPaFormat != PA_FORMAT_XML)
		{ 
			HBITMAP avt = (HBITMAP)::CallService(MS_UTILS_LOADBITMAPT, 0, (WPARAM)path);
			if (!avt) return iRet;
			::DeleteObject(avt);
		}

		wchar_t *avatarPath = this->GetContactAvatarFilePath(NULL);
		if (::wcscmp(path, avatarPath) && !::CopyFile(path, avatarPath, FALSE))
		{
			this->Log("Failed to copy our avatar to local storage.");
			return iRet;
		}

		// todo: add avatar loading to skype server

		this->SetSettingDword("AvatarTS", time(NULL));
		iRet = 0;
	}
	else
	{
		// todo: avatar deletig
		this->DeleteSetting("AvatarTS");
		iRet = 0;
	}

	return iRet;
}