/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-13 Robert Pösel

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

*/

#include "common.h"

bool FacebookProto::GetDbAvatarInfo(PROTO_AVATAR_INFORMATIONT &ai, std::string *url)
{
	DBVARIANT dbv;
	if (!getString(ai.hContact, FACEBOOK_KEY_AV_URL, &dbv)) {
		std::string new_url = dbv.pszVal;
		db_free(&dbv);

		if (new_url.empty())
			return false;
	
		if (url)
			*url = new_url;

		if (!getTString(ai.hContact, FACEBOOK_KEY_ID, &dbv)) {
			std::string ext = new_url.substr(new_url.rfind('.'), 4);
			std::tstring filename = GetAvatarFolder() + L'\\' + dbv.ptszVal + (TCHAR*)_A2T(ext.c_str());
			db_free(&dbv);			

			_tcsncpy_s(ai.filename, SIZEOF(ai.filename), filename.c_str(), _TRUNCATE);
			ai.format = ProtoGetAvatarFormat(ai.filename);
			return true;
		}
	}
	return false;
}

void FacebookProto::CheckAvatarChange(MCONTACT hContact, std::string image_url)
{
	// Facebook contacts always have some avatar - keep avatar in database even if we have loaded empty one (e.g. for 'On Mobile' contacts)
	if (image_url.empty())
		return;
	
	// We've got url to avatar of default size 32x32px, let's change it to bigger one
	if (image_url.find("oh=") == std::tstring::npos) {
		// We can change avatar size only when there are no parameters (hashes) in URL
		if (getBool(FACEBOOK_KEY_BIG_AVATARS, DEFAULT_BIG_AVATARS)) {
			// Remove cropping and use bigger size
			std::tstring::size_type pos = image_url.find("/t1.0-1/");
			if (pos != std::tstring::npos) {
				pos += 8;

				std::tstring::size_type pos2 = image_url.find("/", pos);
				if (pos2 != std::tstring::npos && image_url.find("/", pos2 + 1) != std::tstring::npos)
					pos2 = image_url.find("/", pos2 + 1);

				// TODO: crop it somehow to square image

				if (pos2 != std::tstring::npos)
					image_url.replace(pos, pos2 - pos, "p180x180");

				// Allow big images
				if ((pos = image_url.rfind("_s.")) != std::tstring::npos || (pos = image_url.rfind("_t.")) != std::tstring::npos) {
					image_url = image_url.replace(pos, 3, "_q.");
				}
			}
		} else {
			// Try to get slighly bigger (but still square) image
			utils::text::replace_first(&image_url, ".32.32/", ".50.50/");
			utils::text::replace_first(&image_url, "32x32/", "50x50/");
		}
	}

	// Check for avatar change
	ptrA old_url(getStringA(hContact, FACEBOOK_KEY_AV_URL));
	bool update_required = (old_url == NULL || image_url.compare(old_url) != 0);

	if (update_required)
		setString(hContact, FACEBOOK_KEY_AV_URL, image_url.c_str());

	if (!hContact) {
		PROTO_AVATAR_INFORMATIONT ai = { sizeof(ai) };
		if (GetAvatarInfo(update_required ? GAIF_FORCE : 0, (LPARAM)&ai) != GAIR_WAITFOR)
			CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName, 0);
	} else if (update_required) {
		db_set_b(hContact, "ContactPhoto", "NeedUpdate", 1);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);
	}
}

void FacebookProto::UpdateAvatarWorker(void *)
{
	HANDLE nlc = NULL;

	debugLogA("***** UpdateAvatarWorker");

	for (;;)
	{
		std::string url;
		PROTO_AVATAR_INFORMATIONT ai = {sizeof(ai)};
		ai.hContact = avatar_queue[0];

		if (Miranda_Terminated())
		{
			debugLogA("***** Terminating avatar update early: %s", url.c_str());
			break;
		} 

		if (GetDbAvatarInfo(ai, &url))
		{
			debugLogA("***** Updating avatar: %s", url.c_str());
			bool success = facy.save_url(url, ai.filename, nlc);

			if (ai.hContact)
				ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, success ? ACKRESULT_SUCCESS : ACKRESULT_FAILED, (HANDLE)&ai, 0);
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

std::tstring FacebookProto::GetAvatarFolder()
{
	TCHAR path[MAX_PATH];
	mir_sntprintf(path, SIZEOF(path), _T("%s\\%s"), VARST(_T("%miranda_avatarcache%")), m_tszUserName);
	return path;
}

INT_PTR FacebookProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
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
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		res = 1;
		break;
	}

	return res;
}

INT_PTR FacebookProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	if (!lParam)
		return GAIR_NOAVATAR;

	PROTO_AVATAR_INFORMATIONT* AI = (PROTO_AVATAR_INFORMATIONT*)lParam;
	if (GetDbAvatarInfo(*AI, NULL))
	{
		bool fileExist = _taccess(AI->filename, 0) == 0;
		
		bool needLoad;
		if (AI->hContact)
			needLoad = (wParam & GAIF_FORCE) && (!fileExist || db_get_b(AI->hContact, "ContactPhoto", "NeedUpdate", 0));
		else
			needLoad = (wParam & GAIF_FORCE) || !fileExist;

		if (needLoad)
		{												
			debugLogA("***** Starting avatar request thread for %s", _T2A(AI->filename));
			ScopedLock s(avatar_lock_);

			if (std::find(avatar_queue.begin(), avatar_queue.end(), AI->hContact) == avatar_queue.end())
			{
				bool is_empty = avatar_queue.empty();
				avatar_queue.push_back(AI->hContact);
				if (is_empty)
					ForkThread(&FacebookProto::UpdateAvatarWorker, NULL);
			}
			return GAIR_WAITFOR;
		}
		else if (fileExist)
			return GAIR_SUCCESS;

	}
	return GAIR_NOAVATAR;
}

INT_PTR FacebookProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	debugLogA("***** GetMyAvatar");

	if (!wParam || !lParam)
		return -3;

	TCHAR* buf = (TCHAR*)wParam;
	int  size = (int)lParam;

	PROTO_AVATAR_INFORMATIONT ai = {sizeof(ai)};
	switch (GetAvatarInfo(0, (LPARAM)&ai)) {
	case GAIR_SUCCESS:
		_tcsncpy(buf, ai.filename, size);
		buf[size-1] = 0;
		return 0;

	case GAIR_WAITFOR:
		return -1;

	default:
		return -2;
	}
}
