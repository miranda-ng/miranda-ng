#include "commonheaders.h"

//=======================================================
//DBGetContactSettingString
//=======================================================
int DBGetContactSettingString(HANDLE hContact, char* szModule, char* szSetting, char* value)
{
	DBVARIANT dbv;
	if (!DBGetContactSetting(hContact, szModule, szSetting, &dbv))
	{
		strcpy(value, dbv.pszVal);
		DBFreeVariant(&dbv);
		return 1;
	}
	else 
	{
		DBFreeVariant(&dbv);
		return 0;
	}
	
	return 0;
}

//=======================================================
//GetCaps
//=======================================================

int GetLCCaps(WPARAM wParam,LPARAM lParam)
{
	if(wParam==PFLAGNUM_1)
		return 0;
	if(wParam==PFLAGNUM_2)
		return PF2_ONLINE|PF2_LONGAWAY|PF2_SHORTAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT|PF2_INVISIBLE|PF2_OUTTOLUNCH|PF2_ONTHEPHONE; // add the possible statuses here.
	if(wParam==PFLAGNUM_3)
		return 0;
	return 0;
}

//=======================================================
//GetName
//=======================================================
int GetLCName(WPARAM wParam,LPARAM lParam)
{
	lstrcpyn((char*)lParam,modname,wParam);
	return 0;
}

//=======================================================
//BPLoadIcon
//=======================================================
int LoadLCIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;

	switch(wParam&0xFFFF) {
		case PLI_PROTOCOL: id=IDI_MAIN; break; // IDI_MAIN is the main icon for the protocol
		default: return (int)(HICON)NULL;	
	}
	return (int)LoadImage(hInst,MAKEINTRESOURCE(id),IMAGE_ICON,GetSystemMetrics(wParam&PLIF_SMALL?SM_CXSMICON:SM_CXICON),GetSystemMetrics(wParam&PLIF_SMALL?SM_CYSMICON:SM_CYICON),0);
}

//=======================================================
//SetFStatus			
//=======================================================
int SetLCStatus(WPARAM wParam,LPARAM lParam) 
{ 
    int oldStatus;
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	const char* szProto = (const char*)lParam;
	if (szProto && strcmp(szProto, modname)) return 0; // not nimc so ignore anyway...
	else if (!szProto && DBGetContactSettingByte(NULL, modname, "IgnoreGlobalStatusChange",0)) return 0; // global change and being ignored
	else
	{
		oldStatus = LCStatus;
		LCStatus = wParam;
		DBWriteContactSettingWord(NULL, modname, "Status", (WORD)wParam);
		DBWriteContactSettingWord(NULL, modname, "timerCount",0);
		if (LCStatus == ID_STATUS_OFFLINE || (LCStatus == ID_STATUS_AWAY && !DBGetContactSettingByte(NULL, modname, "AwayAsStatus", 0)) || !DBGetContactSettingWord(NULL, modname, "Timer",1)) killTimer();
		else if (DBGetContactSettingWord(NULL, modname, "Timer",1))
			startTimer(TIMER); 
		while (hContact)
		{
			const char* proto = (const char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0); 
			if (proto && !strcmp(proto, modname)) 
			{
				if (LCStatus != ID_STATUS_OFFLINE) replaceAllStrings(hContact);
				switch (LCStatus)
				{
					case ID_STATUS_OFFLINE:
						if (DBGetContactSettingByte(hContact, modname, "AlwaysVisible",0) && !DBGetContactSettingByte(hContact, modname, "VisibleUnlessOffline",1))
							DBWriteContactSettingWord(hContact,modname, "Status",(WORD)DBGetContactSettingWord(hContact,modname, "Icon",ID_STATUS_ONLINE));	
						else DBWriteContactSettingWord(hContact,modname, "Status", ID_STATUS_OFFLINE);
					break;
					case ID_STATUS_ONLINE:
						DBWriteContactSettingWord(hContact,modname, "Status",(WORD)DBGetContactSettingWord(hContact,modname, "Icon",ID_STATUS_ONLINE));
					break;
					case ID_STATUS_AWAY:
						if (DBGetContactSettingByte(NULL, modname, "AwayAsStatus", 0) && (DBGetContactSettingByte(hContact, modname, "AlwaysVisible",0) || (DBGetContactSettingWord(hContact,modname, "Icon",ID_STATUS_ONLINE)==ID_STATUS_AWAY)) )
							DBWriteContactSettingWord(hContact,modname, "Status",(WORD)(WORD)DBGetContactSettingWord(hContact,modname, "Icon",ID_STATUS_ONLINE));	
						else if (!DBGetContactSettingByte(NULL, modname, "AwayAsStatus", 0))
							DBWriteContactSettingWord(hContact,modname, "Status",(WORD)DBGetContactSettingWord(hContact,modname, "Icon",ID_STATUS_ONLINE));
						else DBWriteContactSettingWord(hContact,modname, "Status", ID_STATUS_OFFLINE);
					break;
					default:
						if (DBGetContactSettingByte(hContact, modname, "AlwaysVisible",0) || LCStatus == DBGetContactSettingWord(hContact,modname, "Icon",ID_STATUS_ONLINE))
							DBWriteContactSettingWord(hContact,modname, "Status",(WORD)DBGetContactSettingWord(hContact,modname, "Icon",ID_STATUS_ONLINE));
					break;
				}
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		ProtoBroadcastAck(modname,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)oldStatus,wParam);
	}
	return 0; 
} 



//=======================================================
//GetStatus
//=======================================================
int GetLCStatus(WPARAM wParam,LPARAM lParam)
{
	if ((LCStatus >= ID_STATUS_ONLINE) && (LCStatus <= ID_STATUS_OUTTOLUNCH))
		return LCStatus;
	else
		return ID_STATUS_OFFLINE;
}

