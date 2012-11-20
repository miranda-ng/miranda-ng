#include "stdafx.h"
#include "processbuddyinfo.h"

#include <sys\stat.h> 


/*
DBWriteContactSettingString(xsa->hContact, "ContactPhoto", "Backup", av.backup);
DBWriteContactSettingString(xsa->hContact, "ContactPhoto", "File", av.file);
DBWriteContactSettingString(xsa->hContact, "ContactPhoto", "RFile", av.rfile);
DBWriteContactSettingWord(xsa->hContact, "ContactPhoto", "Format", av.type);
*/

extern HANDLE	 XFireAvatarFolder;

//vom Yahoo plugin
/*int avt_hash(const char *key, DWORD ksize)
{
  const char *p = key;
  int h = *p;
  long l = 1;
  
  if (h)
	for (p += 1; l < ksize; p++, l++)
	  h = (h << 5) - h + *p;

  return h;
}*/

void ProcessBuddyInfo(xfirelib::BuddyInfoPacket *buddyinfo,HANDLE hcontact,char* username) {
	char temp[255]="";
	char filename[255];
	BOOL dl=FALSE;
	int type;

	//versuche doppeltes laden zuvermeiden
	if(hcontact) //avatar von freunden
	{
		if(DBGetContactSettingDword(hcontact, "ContactPhoto", "XFireAvatarId", 0)==buddyinfo->avatarid &&
			DBGetContactSettingByte(hcontact, "ContactPhoto", "XFireAvatarMode", 0)==buddyinfo->avatarmode)
			return;
	}
	else //eigeneder avatar
	{
		if(DBGetContactSettingDword(hcontact, protocolname, "XFireAvatarId", 0)==buddyinfo->avatarid &&
			DBGetContactSettingByte(hcontact, protocolname, "XFireAvatarMode", 0)==buddyinfo->avatarmode)
			return;

		//alten dateipfad des avatars löschen, wenn sichw as geändert hat
		DBDeleteContactSetting(NULL,protocolname, "MyAvatarFile");
	}

	FoldersGetCustomPath( XFireAvatarFolder, filename, 255, "" );
	strcat(filename,"\\");
	
	switch(buddyinfo->avatarmode) {
		case 1:
			strcat(filename,username);
			strcat(filename,".gif");
			type=PA_FORMAT_GIF;

			sprintf(temp,"/xfire/xf/images/avatars/gallery/default/%03d.gif",buddyinfo->avatarid);

			dl=GetWWWContent("media.xfire.com",temp,filename,FALSE);
			break;
		case 2:
			strcat(filename,username);
			strcat(filename,".jpg");
			type=PA_FORMAT_JPEG;

			sprintf(temp,"/avatar/100/%s.jpg?%d",username,buddyinfo->avatarid);

			dl=GetWWWContent("screenshot.xfire.com",temp,filename,FALSE);
			break;
		case 3:
			type=PA_FORMAT_GIF;
			strcat(filename,"xfire.gif");
	
			sprintf(temp,"/xfire/xf/images/avatars/gallery/default/xfire.gif",buddyinfo->avatarid);

			dl=GetWWWContent("media.xfire.com",temp,filename,TRUE);
			break;
		default:
			return;
	}


	if(dl!=FALSE)
	{
		if(hcontact) //buddyavatar setzen
		{
			DBWriteContactSettingDword(hcontact, "ContactPhoto", "XFireAvatarId", buddyinfo->avatarid);
			DBWriteContactSettingByte(hcontact, "ContactPhoto", "XFireAvatarMode", buddyinfo->avatarmode);
			PROTO_AVATAR_INFORMATION AI;
			AI.cbSize = sizeof(AI);
			AI.format = type;
			AI.hContact = hcontact;
			lstrcpy(AI.filename,filename);
			ProtoBroadcastAck(protocolname, hcontact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS,(HANDLE) &AI, 0);
		}
		else //eigenen avatar setzen
		{
			DBWriteContactSettingDword(NULL, protocolname, "XFireAvatarId", buddyinfo->avatarid);
			DBWriteContactSettingByte(NULL, protocolname, "XFireAvatarMode", buddyinfo->avatarmode);
			//neuen avatarfilepath eintragen
			DBWriteContactSettingString(NULL,protocolname, "MyAvatarFile",filename);
			//beshceid geben, avatar hat sich geändert
			CallService(MS_AV_REPORTMYAVATARCHANGED,(WPARAM)protocolname,0);
		}
	}
}