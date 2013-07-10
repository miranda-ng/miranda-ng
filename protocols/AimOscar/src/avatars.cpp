/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "aim.h"

void __cdecl CAimProto::avatar_request_thread(void* param)
{
	HANDLE hContact = (HANDLE)param;

	char *sn = getStringA(hContact, AIM_KEY_SN);
	LOG("Starting avatar request thread for %s)", sn);

	if (wait_conn(hAvatarConn, hAvatarEvent, 0x10))
	{
		char *hash_str = getStringA(hContact, AIM_KEY_AH);
		char type = getByte(hContact, AIM_KEY_AHT, 1);

		size_t len = (strlen(hash_str) + 1) / 2;
		char* hash = (char*)alloca(len);
		string_to_bytes(hash_str, hash);
		LOG("Requesting an Avatar: %s (Hash: %s)", sn, hash_str);
		aim_request_avatar(hAvatarConn, avatar_seqno, sn, type, hash, (unsigned short)len);

		mir_free(hash_str);
	}

	mir_free(sn);
}

void __cdecl CAimProto::avatar_upload_thread(void* param)
{
	avatar_up_req* req = (avatar_up_req*)param;

	if (wait_conn(hAvatarConn, hAvatarEvent, 0x10))
	{
		if (req->size2)
		{
			aim_upload_avatar(hAvatarConn, avatar_seqno, 1, req->data2, req->size2);
			aim_upload_avatar(hAvatarConn, avatar_seqno, 12, req->data1, req->size1);
		}
		else
			aim_upload_avatar(hAvatarConn, avatar_seqno, 1, req->data1, req->size1);
	}
	delete req;
}

void CAimProto::avatar_request_handler(HANDLE hContact, char* hash, unsigned char type)//checks to see if the avatar needs requested
{
	if (hContact == NULL)
	{
		hash = hash_lg ? hash_lg : hash_sm;
		type = hash_lg ? 12 : 1;
	}

	char* saved_hash = getStringA(hContact, AIM_KEY_AH);
	if (hash && _stricmp(hash, "0201d20472") && _stricmp(hash, "2b00003341")) //gaim default icon fix- we don't want their blank icon displaying.
	{
		if (_strcmps(saved_hash, hash))
		{
			setByte(hContact, AIM_KEY_AHT, type);
			setString(hContact, AIM_KEY_AH, hash);

			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
		}
	}
	else
	{
		if (saved_hash)
		{
			delSetting(hContact, AIM_KEY_AHT);
			delSetting(hContact, AIM_KEY_AH);

			ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
		}
	}
	mir_free(saved_hash);
}

void CAimProto::avatar_retrieval_handler(const char* sn, const char* hash, const char* data, int data_len)
{
	bool res = false;
	PROTO_AVATAR_INFORMATIONT AI = {0};
	AI.cbSize = sizeof(AI);

	AI.hContact = contact_from_sn(sn);

	if (data_len > 0)
	{
		const TCHAR *type;
		AI.format = detect_image_type(data, type);
		get_avatar_filename(AI.hContact, AI.filename, SIZEOF(AI.filename), type);

		int fileId = _topen(AI.filename, _O_CREAT | _O_TRUNC | _O_WRONLY | O_BINARY,  _S_IREAD | _S_IWRITE);
		if (fileId >= 0)
		{
			_write(fileId, data, data_len);
			_close(fileId);
			res = true;

			char *my_sn = getStringA(AIM_KEY_SN);
			if (!_strcmps(sn, my_sn))
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
			mir_free(my_sn);
		}
	}
	else LOG("AIM sent avatar of zero length for %s.(Usually caused by repeated request for the same icon)", sn);

	ProtoBroadcastAck(AI.hContact, ACKTYPE_AVATAR, res ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, &AI, 0);
}

int detect_image_type(const char* stream, const TCHAR* &type_ret)
{
	if (stream[0]=='G'&&stream[1]=='I'&&stream[2]=='F')
	{
		type_ret = _T(".gif");
		return PA_FORMAT_GIF;
	}
	else if (stream[1]=='P'&&stream[2]=='N'&&stream[3]=='G')
	{
		type_ret = _T(".png");
		return PA_FORMAT_PNG;
	}
	else if (stream[0]=='B'&&stream[1]=='M')
	{
		type_ret = _T(".bmp");
		return PA_FORMAT_BMP;
	}
	else//assume jpg
	{
		type_ret = _T(".jpg");
		return PA_FORMAT_JPEG;
	}
}

int detect_image_type(const TCHAR* file)
{
   const TCHAR *ext = _tcsrchr(file, '.');
   if (ext == NULL)
	   return PA_FORMAT_UNKNOWN;
   if (_tcsicmp(ext, _T(".gif")) == 0)
	   return PA_FORMAT_GIF;
   else if (_tcsicmp(ext, _T(".bmp")) == 0)
	   return PA_FORMAT_BMP;
   else if (_tcsicmp(ext, _T(".png")) == 0)
	   return PA_FORMAT_PNG;
   else
	   return PA_FORMAT_JPEG;
}

void CAimProto::init_custom_folders(void)
{
	if (init_cst_fld_ran) return;

	TCHAR AvatarsFolder[MAX_PATH];

	mir_sntprintf(AvatarsFolder, SIZEOF(AvatarsFolder), _T("%%miranda_avatarcache%%\\%S"), m_szModuleName);
	hAvatarsFolder = FoldersRegisterCustomPathT(LPGEN("Avatars"), m_szModuleName, AvatarsFolder, m_tszUserName);
	init_cst_fld_ran = true;
}

int CAimProto::get_avatar_filename(HANDLE hContact, TCHAR* pszDest, size_t cbLen, const TCHAR *ext)
{
	size_t tPathLen;
	bool found = false;

	init_custom_folders();

	TCHAR* path = (TCHAR*)alloca(cbLen * sizeof(TCHAR));
	if (hAvatarsFolder == NULL || FoldersGetCustomPathT(hAvatarsFolder, path, (int)cbLen, _T("")))
		tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), (TCHAR*)VARST( _T("%miranda_avatarcache%")), m_szModuleName);
	else {
		_tcscpy(pszDest, path);
		tPathLen = _tcslen(pszDest);
	}

	if (ext && _taccess(pszDest, 0))
		CreateDirectoryTreeT(pszDest);

	size_t tPathLen2 = tPathLen;

	DBVARIANT dbv;
	if (getTString(hContact, AIM_KEY_AH, &dbv)) return GAIR_NOAVATAR;
	tPathLen += mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T("\\%s"), dbv.ptszVal);
	db_free(&dbv);

	if (ext == NULL)
	{
		mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, _T(".*"));

		_tfinddata_t c_file;
		long hFile = _tfindfirst(pszDest, &c_file);
		if (hFile > -1L)
		{
			do {
				if (_tcsrchr(c_file.name, '.'))
				{
					mir_sntprintf(pszDest + tPathLen2, cbLen - tPathLen2, _T("\\%s"), c_file.name);
					found = true;
				}
			} while (_tfindnext(hFile, &c_file) == 0);
			_findclose( hFile );
		}

		if (!found) pszDest[0] = 0;
	}
	else
	{
		mir_sntprintf(pszDest + tPathLen, cbLen - tPathLen, ext);
		found = _taccess(pszDest, 0) == 0;
	}

	return found ? GAIR_SUCCESS : GAIR_WAITFOR;
}

bool get_avatar_hash(const TCHAR* file, char* hash, char** data, unsigned short &size)
{
	int fileId = _topen(file, _O_RDONLY | _O_BINARY, _S_IREAD);
	if (fileId == -1) return false;

	long  lAvatar = _filelength(fileId);
	if (lAvatar <= 0)
	{
		_close(fileId);
		return false;
	}

	char* pResult = (char*)mir_alloc(lAvatar);
	int res = _read(fileId, pResult, lAvatar);
	_close(fileId);

	if (res <= 0)
	{
		mir_free(pResult);
		return false;
	}

	mir_md5_state_t state;
	mir_md5_init(&state);
	mir_md5_append(&state, (unsigned char*)pResult, lAvatar);
	mir_md5_finish(&state, (unsigned char*)hash);

	if (data)
	{
		*data = pResult;
		size = (unsigned short)lAvatar;
	}
	else
		mir_free(pResult);

	return true;
}

void rescale_image(char *data, unsigned short size, char *&data1, unsigned short &size1)
{
	FI_INTERFACE *fei = NULL;
	CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM) &fei);
	if (fei == NULL) return;

    FIMEMORY *hmem = fei->FI_OpenMemory((BYTE *)data, size);
    FREE_IMAGE_FORMAT fif = fei->FI_GetFileTypeFromMemory(hmem, 0);
    FIBITMAP *dib = fei->FI_LoadFromMemory(fif, hmem, 0);
    fei->FI_CloseMemory(hmem);

	if (fei->FI_GetWidth(dib) > 64)
	{
		FIBITMAP *dib1 = fei->FI_Rescale(dib, 64, 64, FILTER_BSPLINE);

		FIMEMORY *hmem = fei->FI_OpenMemory(NULL, 0);
		fei->FI_SaveToMemory(fif, dib1, hmem, 0);

		BYTE *data2; DWORD size2;
		fei->FI_AcquireMemory(hmem, &data2, &size2);
		data1 = (char*)mir_alloc(size2);
		memcpy(data1, data2, size2);
		size1 = size2;

		fei->FI_CloseMemory(hmem);
		fei->FI_Unload(dib1);
	}
	fei->FI_Unload(dib);
}