#include "skype_proto.h"

LIST<void> CSkypeProto::serviceList(0);

void CSkypeProto::InitServiceList()
{
	CSkypeProto::serviceList.insert(
		::CreateServiceFunction("Skype/MenuChoose", CSkypeProto::MenuChooseService));
}

int CSkypeProto::GetAvatarInfo(WPARAM, LPARAM lParam)
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

int CSkypeProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
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

int CSkypeProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam) return -2;

	wchar_t *path = this->GetOwnAvatarFilePath();
	if (path && !_waccess(path, 0)) 
	{
		::wcsncpy((wchar_t *)wParam, path, (int)lParam);
		delete path;
		return 0;
	}

	delete path;
	return -1;
}

int CSkypeProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *path = (wchar_t *)lParam;
	int iRet = -1;

	//if (path)
	//{ // set file for avatar
	//	int dwPaFormat = DetectAvatarFormat(tszFile);
	//	if (dwPaFormat != PA_FORMAT_XML)
	//	{ 
	//		// if it should be image, check if it is valid
	//		HBITMAP avt = (HBITMAP)CallService(MS_UTILS_LOADBITMAPT, 0, (WPARAM)tszFile);
	//		if (!avt) return iRet;
	//		DeleteObject(avt);
	//	}

	//	TCHAR tszMyFile[MAX_PATH+1];
	//	GetFullAvatarFileName(0, NULL, dwPaFormat, tszMyFile, MAX_PATH);
	//	// if not in our storage, copy
	//	if (lstrcmp(tszFile, tszMyFile) && !CopyFile(tszFile, tszMyFile, FALSE))
	//	{
	//		NetLog_Server("Failed to copy our avatar to local storage.");
	//		return iRet;
	//	}

	//	BYTE *hash = calcMD5HashOfFile(tszMyFile);
	//	if (hash)
	//	{
	//		BYTE* ihash = (BYTE*)_alloca(0x14);
	//		// upload hash to server
	//		ihash[0] = 0;    //unknown
	//		ihash[1] = dwPaFormat == PA_FORMAT_XML ? AVATAR_HASH_FLASH : AVATAR_HASH_STATIC; //hash type
	//		ihash[2] = 1;    //hash status
	//		ihash[3] = 0x10; //hash len
	//		memcpy(ihash+4, hash, 0x10);
	//		updateServAvatarHash(ihash, 0x14);

	//		if (setSettingBlob(NULL, "AvatarHash", ihash, 0x14))
	//		{
	//			NetLog_Server("Failed to save avatar hash.");
	//		}

	//		TCHAR tmp[MAX_PATH];
	//		CallService(MS_UTILS_PATHTORELATIVET, (WPARAM)tszMyFile, (LPARAM)tmp);
	//		setSettingStringT(NULL, "AvatarFile", tmp);

	//		iRet = 0;

	//		SAFE_FREE((void**)&hash);
	//	}
	//}
	//else
	//{ // delete user avatar
	//	deleteSetting(NULL, "AvatarFile");
	//	setSettingBlob(NULL, "AvatarHash", hashEmptyAvatar, 9);
	//	updateServAvatarHash(hashEmptyAvatar, 9); // set blank avatar
	//	iRet = 0;
	//}

	return iRet;
}