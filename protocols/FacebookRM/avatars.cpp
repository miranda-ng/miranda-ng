/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File name      : $HeadURL: http://eternityplugins.googlecode.com/svn/trunk/facebook/avatars.cpp $
Revision       : $Revision: 91 $
Last change by : $Author: n3weRm0re.ewer $
Last change on : $Date: 2011-01-08 11:10:34 +0100 (so, 08 1 2011) $

*/

#include "common.h"

bool FacebookProto::GetDbAvatarInfo(PROTO_AVATAR_INFORMATION &ai, std::string *url)
{
	DBVARIANT dbv;
	if (!DBGetContactSettingString(ai.hContact, m_szModuleName, FACEBOOK_KEY_AV_URL, &dbv))
	{
		std::string new_url = dbv.pszVal;
		DBFreeVariant(&dbv);

		if (new_url.empty())
			return false;
	
		if (url)
			*url = new_url;

		if (!DBGetContactSettingString(ai.hContact, m_szModuleName, FACEBOOK_KEY_ID, &dbv))
		{
			std::string ext = new_url.substr(new_url.rfind('.'));
			std::string filename = GetAvatarFolder() + '\\' + dbv.pszVal + ext;			
			DBFreeVariant(&dbv);			

			ai.hContact = ai.hContact;
			ai.format = ext_to_format(ext);
			strncpy(ai.filename, filename.c_str(), sizeof(ai.filename));
			ai.filename[sizeof(ai.filename)-1] = 0;

			return true;
		}
	}
	return false;
}

void FacebookProto::CheckAvatarChange(HANDLE hContact, std::string image_url)
{
	// Facebook contacts always have some avatar - keep avatar in database even if we have loaded empty one (e.g. for 'On Mobile' contacts)
	if (image_url.empty())
		return;
	
	if (DBGetContactSettingByte(NULL, m_szModuleName, FACEBOOK_KEY_BIG_AVATARS, DEFAULT_BIG_AVATARS)) 
	{
		std::string::size_type pos = image_url.rfind( "_q." );
		if (pos != std::string::npos)
			image_url = image_url.replace( pos, 3, "_s." );
	}
	
	DBVARIANT dbv;
	bool update_required = true;
	if (!DBGetContactSettingString(hContact, m_szModuleName, FACEBOOK_KEY_AV_URL, &dbv))
	{
		update_required = image_url != dbv.pszVal;
		DBFreeVariant(&dbv);
	}
	if (update_required || !hContact)
	{
		DBWriteContactSettingString(hContact, m_szModuleName, FACEBOOK_KEY_AV_URL, image_url.c_str());
		if (hContact)
			ProtoBroadcastAck(m_szModuleName, hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
		else
		{
			PROTO_AVATAR_INFORMATION ai = {sizeof(ai)};
			if (GetAvatarInfo(update_required ? GAIF_FORCE : 0, (LPARAM)&ai) != GAIR_WAITFOR)
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
		}
	}
}

void FacebookProto::UpdateAvatarWorker(void *)
{
	HANDLE nlc = NULL;

	LOG("***** UpdateAvatarWorker");

	for (;;)
	{
		std::string url;
		PROTO_AVATAR_INFORMATION ai = {sizeof(ai)};
		ai.hContact = avatar_queue[0];

		if (Miranda_Terminated())
		{
			LOG("***** Terminating avatar update early: %s", url.c_str());
			break;
		} 

		if (GetDbAvatarInfo(ai, &url))
		{
			LOG("***** Updating avatar: %s", url.c_str());
			bool success = facy.save_url(url, std::string(ai.filename), nlc);

			if (ai.hContact)
				ProtoBroadcastAck(m_szModuleName, ai.hContact, ACKTYPE_AVATAR, success ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)&ai, 0);
			else if (success)
				CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
		}

		ScopedLock s(avatar_lock_);
		avatar_queue.erase(avatar_queue.begin());
		if (avatar_queue.empty())
			break;
	}
	Netlib_CloseHandle(nlc);
}

std::string FacebookProto::GetAvatarFolder()
{
	char path[MAX_PATH];
	if ( hAvatarFolder_ && FoldersGetCustomPath(hAvatarFolder_,path,sizeof(path), "") == 0 )
		return path;
	else
		return def_avatar_folder_;
}

int FacebookProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam)
	{
	case AF_MAXSIZE:
		((POINT*)lParam)->x = -1;
		((POINT*)lParam)->y = -1;
		break;

	case AF_MAXFILESIZE:
		res = 0;
		break;

	case AF_PROPORTION:
		res = PIP_NONE;
		break;

	case AF_FORMATSUPPORTED:
		res = (lParam == PA_FORMAT_JPEG || lParam == PA_FORMAT_GIF);
		break;

	case AF_DELAYAFTERFAIL:
		res = 60 * 1000;
		break;

	case AF_ENABLED:
	case AF_DONTNEEDDELAYS:
	case AF_FETCHALWAYS:
		res = 1;
		break;
	}

	return res;
}

int FacebookProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATION* AI = (PROTO_AVATAR_INFORMATION*)lParam;

	if (GetDbAvatarInfo(*AI, NULL))
	{
		bool fileExist = _access(AI->filename, 0) == 0;
		
		bool needLoad;
		if (AI->hContact)
			needLoad = (wParam & GAIF_FORCE) && (!fileExist || DBGetContactSettingByte(AI->hContact, "ContactPhoto", "NeedUpdate", 0));
		else
			needLoad = (wParam & GAIF_FORCE) || !fileExist;

		if (needLoad)
		{												
			LOG("***** Starting avatar request thread for %s", AI->filename);
			ScopedLock s( avatar_lock_ );

			if (std::find(avatar_queue.begin(), avatar_queue.end(), AI->hContact) == avatar_queue.end())
			{
				bool is_empty = avatar_queue.empty();
				avatar_queue.push_back(AI->hContact);
				if (is_empty)
					ForkThread(&FacebookProto::UpdateAvatarWorker, this, NULL);
			}
			return GAIR_WAITFOR;
		}
		else if (fileExist)
			return GAIR_SUCCESS;

	}
	return GAIR_NOAVATAR;
}

int FacebookProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	LOG("***** GetMyAvatar");

	if (!wParam || !lParam)
		return -3;

	char* buf = ( char* )wParam;
	int  size = ( int )lParam;

	PROTO_AVATAR_INFORMATION ai = {sizeof(ai)};
	switch (GetAvatarInfo(0, (LPARAM)&ai))
	{
	case GAIR_SUCCESS:
		strncpy(buf, ai.filename, size);
		buf[size-1] = 0;
		return 0;

	case GAIR_WAITFOR:
		return -1;

	default:
		return -2;
	}
}
