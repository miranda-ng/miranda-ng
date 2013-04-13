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

	wchar_t *sid = this->GetSettingString(pai->hContact, "sid");
	if (sid)
	{
		wchar_t *path = this->GetContactAvatarFilePath(pai->hContact);
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
	if (path && !_waccess(path, 0)) 
	{
		::wcsncpy((wchar_t *)wParam, path, (int)lParam);
		delete path;
		return 0;
	}

	delete path;
	return -1;
}

INT_PTR __cdecl CSkypeProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *path = (wchar_t *)lParam;
	int iRet = -1;

	if (path)
	{
		/*int dwPaFormat = CSkypeProto::DetectAvatarFormat(path);
		if (dwPaFormat != PA_FORMAT_XML)
		{ 
			HBITMAP avt = (HBITMAP)::CallService(MS_UTILS_LOADBITMAPT, 0, (WPARAM)path);
			if (!avt) return iRet;
			::DeleteObject(avt);
		}*/

		wchar_t *avatarPath = this->GetContactAvatarFilePath(NULL);
		if (::wcscmp(path, avatarPath) && !::CopyFile(path, avatarPath, FALSE))
		{
			this->Log(L"Failed to copy our avatar to local storage.");
			return iRet;
		}
		
		int len;
		char *buffer;
		FILE* fp = _wfopen(avatarPath, L"rb");
		if (!fp)
		{
			this->Log(L"Failed to read avatar in local storage.");
			return iRet;
		}
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		buffer = new char[len + 1];
		fread(buffer, len, 1, fp);
		fclose(fp);

		int fbl;
		SEBinary avatar(buffer, len);
		Skype::VALIDATERESULT result = Skype::NOT_VALIDATED;
		if (!this->skype->ValidateAvatar(avatar, result, fbl) || result != Skype::VALIDATED_OK)
		{
			this->Log(CSkypeProto::ValidationReasons[result]);
			return iRet;
		}
		if (!this->account->SetBinProperty(Account::P_AVATAR_IMAGE, avatar))
		{
			this->Log(L"Failed to send avatar on server.");
			return iRet;
		}

		delete [] buffer;

		this->SetSettingDword("AvatarTS", time(NULL));
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