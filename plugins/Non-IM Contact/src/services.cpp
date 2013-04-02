#include "commonheaders.h"

//=======================================================
//db_get_static
//=======================================================
int db_get_static(HANDLE hContact, const char *szModule, const char *szSetting, char *value)
{
	DBVARIANT dbv;
	if (!DBGetContactSetting(hContact, szModule, szSetting, &dbv))
	{
		strcpy(value, dbv.pszVal);
		db_free(&dbv);
		return 1;
	}
	else 
	{
		db_free(&dbv);
		return 0;
	}
	
	return 0;
}

//=======================================================
//GetCaps
//=======================================================

INT_PTR GetLCCaps(WPARAM wParam,LPARAM lParam)
{
	if (wParam==PFLAGNUM_1)
		return 0;
	if (wParam==PFLAGNUM_2)
		return PF2_ONLINE|PF2_LONGAWAY|PF2_SHORTAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT|PF2_INVISIBLE|PF2_OUTTOLUNCH|PF2_ONTHEPHONE; // add the possible statuses here.
	if (wParam==PFLAGNUM_3)
		return 0;
	return 0;
}

//=======================================================
//GetName
//=======================================================
INT_PTR GetLCName(WPARAM wParam,LPARAM lParam)
{
	lstrcpynA((char*)lParam, MODNAME, wParam);
	return 0;
}

//=======================================================
//BPLoadIcon
//=======================================================
INT_PTR LoadLCIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;

	switch(wParam & 0xFFFF) {
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
	if (szProto && strcmp(szProto, MODNAME)) return 0; // not nimc so ignore anyway...
	else if (!szProto && db_get_b(NULL, MODNAME, "IgnoreGlobalStatusChange",0)) return 0; // global change and being ignored
	else {
		oldStatus = LCStatus;
		LCStatus = wParam;
		db_set_w(NULL, MODNAME, "Status", (WORD)wParam);
		db_set_w(NULL, MODNAME, "timerCount",0);
		if (LCStatus == ID_STATUS_OFFLINE || (LCStatus == ID_STATUS_AWAY && !db_get_b(NULL, MODNAME, "AwayAsStatus", 0)) || !db_get_w(NULL, MODNAME, "Timer",1)) killTimer();
		else if (db_get_w(NULL, MODNAME, "Timer",1))
			startTimer(TIMER); 
		while (hContact) {
			char* proto = GetContactProto(hContact); 
			if (proto && !strcmp(proto, MODNAME)) {
				if (LCStatus != ID_STATUS_OFFLINE) replaceAllStrings(hContact);
				switch (LCStatus) {
				case ID_STATUS_OFFLINE:
					if (db_get_b(hContact, MODNAME, "AlwaysVisible",0) && !db_get_b(hContact, MODNAME, "VisibleUnlessOffline",1))
						db_set_w(hContact,MODNAME, "Status",(WORD)db_get_w(hContact,MODNAME, "Icon",ID_STATUS_ONLINE));	
					else
						db_set_w(hContact,MODNAME, "Status", ID_STATUS_OFFLINE);
					break;
				case ID_STATUS_ONLINE:
					db_set_w(hContact,MODNAME, "Status",(WORD)db_get_w(hContact,MODNAME, "Icon",ID_STATUS_ONLINE));
					break;
				case ID_STATUS_AWAY:
					if (db_get_b(NULL, MODNAME, "AwayAsStatus", 0) && (db_get_b(hContact, MODNAME, "AlwaysVisible",0) || (db_get_w(hContact,MODNAME, "Icon",ID_STATUS_ONLINE)==ID_STATUS_AWAY)) )
						db_set_w(hContact,MODNAME, "Status",(WORD)(WORD)db_get_w(hContact,MODNAME, "Icon",ID_STATUS_ONLINE));	
					else if (!db_get_b(NULL, MODNAME, "AwayAsStatus", 0))
						db_set_w(hContact,MODNAME, "Status",(WORD)db_get_w(hContact,MODNAME, "Icon",ID_STATUS_ONLINE));
					else
						db_set_w(hContact,MODNAME, "Status", ID_STATUS_OFFLINE);
					break;
				default:
					if (db_get_b(hContact, MODNAME, "AlwaysVisible",0) || LCStatus == db_get_w(hContact,MODNAME, "Icon",ID_STATUS_ONLINE))
						db_set_w(hContact,MODNAME, "Status",(WORD)db_get_w(hContact,MODNAME, "Icon",ID_STATUS_ONLINE));
					break;
				}
			}
			hContact = db_find_next(hContact);
		}
		ProtoBroadcastAck(MODNAME,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)oldStatus,wParam);
	}
	return 0; 
} 



//=======================================================
//GetStatus
//=======================================================
INT_PTR GetLCStatus(WPARAM wParam,LPARAM lParam)
{
	if ((LCStatus >= ID_STATUS_ONLINE) && (LCStatus <= ID_STATUS_OUTTOLUNCH))
		return LCStatus;
	else
		return ID_STATUS_OFFLINE;
}

