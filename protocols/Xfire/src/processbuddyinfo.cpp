#include "stdafx.h"
#include "processbuddyinfo.h"
#include "variables.h"
#include <sys\stat.h> 


/*
db_set_s(xsa->hContact, "ContactPhoto", "Backup", av.backup);
db_set_s(xsa->hContact, "ContactPhoto", "File", av.file);
db_set_s(xsa->hContact, "ContactPhoto", "RFile", av.rfile);
db_set_w(xsa->hContact, "ContactPhoto", "Format", av.type);
*/

extern HANDLE	 XFireAvatarFolder;

void ProcessBuddyInfo(xfirelib::BuddyInfoPacket *buddyinfo, MCONTACT hcontact, char* username)
{
	char temp[255] = "";
	char filename[1024] = "";
	BOOL dl = FALSE;
	int type;

	//versuche doppeltes laden zuvermeiden
	if (hcontact) //avatar von freunden
	{
		if (db_get_dw(hcontact, "ContactPhoto", "XFireAvatarId", 0) == buddyinfo->avatarid &&
			db_get_b(hcontact, "ContactPhoto", "XFireAvatarMode", 0) == buddyinfo->avatarmode)
			return;
	}
	else //eigeneder avatar
	{
		if (db_get_dw(hcontact, protocolname, "XFireAvatarId", 0) == buddyinfo->avatarid &&
			db_get_b(hcontact, protocolname, "XFireAvatarMode", 0) == buddyinfo->avatarmode)
			return;

		//alten dateipfad des avatars löschen, wenn sichw as geändert hat
		db_unset(NULL, protocolname, "MyAvatarFile");
	}

	mir_strcpy(filename, XFireGetFoldersPath("Avatar"));

	switch (buddyinfo->avatarmode) {
	case 1:
		mir_strcat(filename, username);
		mir_strcat(filename, ".gif");
		type = PA_FORMAT_GIF;

		mir_snprintf(temp, "/xfire/xf/images/avatars/gallery/default/%03d.gif", buddyinfo->avatarid);

		dl = GetWWWContent("media.xfire.com", temp, filename, FALSE);
		break;
	case 2:
		mir_strcat(filename, username);
		mir_strcat(filename, ".jpg");
		type = PA_FORMAT_JPEG;

		mir_snprintf(temp, "/avatar/100/%s.jpg?%d", username, buddyinfo->avatarid);

		dl = GetWWWContent("screenshot.xfire.com", temp, filename, FALSE);
		break;
	case 3:
		type = PA_FORMAT_GIF;
		mir_strcat(filename, "xfire.gif");

		mir_snprintf(temp, "/xfire/xf/images/avatars/gallery/default/xfire.gif", buddyinfo->avatarid);

		dl = GetWWWContent("media.xfire.com", temp, filename, TRUE);
		break;
	default:
		return;
	}


	if (dl != FALSE) {
		if (hcontact) //buddyavatar setzen
		{
			db_set_dw(hcontact, "ContactPhoto", "XFireAvatarId", buddyinfo->avatarid);
			db_set_b(hcontact, "ContactPhoto", "XFireAvatarMode", buddyinfo->avatarmode);
			PROTO_AVATAR_INFORMATION ai;
			ai.format = type;
			ai.hContact = hcontact;
			_tcsncpy_s(ai.filename, _A2T(filename), _TRUNCATE);
			ProtoBroadcastAck(protocolname, hcontact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, 0);
		}
		else //eigenen avatar setzen
		{
			db_set_dw(NULL, protocolname, "XFireAvatarId", buddyinfo->avatarid);
			db_set_b(NULL, protocolname, "XFireAvatarMode", buddyinfo->avatarmode);
			//neuen avatarfilepath eintragen
			db_set_s(NULL, protocolname, "MyAvatarFile", filename);
			//beshceid geben, avatar hat sich geändert
			CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)protocolname, 0);
		}
	}
}
