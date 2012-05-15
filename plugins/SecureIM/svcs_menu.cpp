#include "commonheaders.h"


INT_PTR __cdecl Service_IsContactSecured(WPARAM wParam, LPARAM lParam) {

	return (isContactSecured((HANDLE)wParam)&SECURED) || isContactPGP((HANDLE)wParam) || isContactGPG((HANDLE)wParam);
}


INT_PTR __cdecl Service_CreateIM(WPARAM wParam,LPARAM lParam){
	if (!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)wParam, (LPARAM)szModuleName))
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)wParam, (LPARAM)szModuleName);
//	WPARAM flags = 0;
//	HANDLE hMetaContact = getMetaContact((HANDLE)wParam);
//	if( hMetaContact ) {
//		wParam = (WPARAM)hMetaContact;
//		flags = PREF_METANODB;
//	}
	CallContactService((HANDLE)wParam,PSS_MESSAGE,(WPARAM)PREF_METANODB,(LPARAM)SIG_INIT);
	return 1;
}


INT_PTR __cdecl Service_DisableIM(WPARAM wParam,LPARAM lParam) {
//	WPARAM flags = 0;
//	HANDLE hMetaContact = getMetaContact((HANDLE)wParam);
//	if( hMetaContact ) {
//		wParam = (WPARAM)hMetaContact;
//		flags = PREF_METANODB;
//	}
	CallContactService((HANDLE)wParam,PSS_MESSAGE,(WPARAM)PREF_METANODB,(LPARAM)SIG_DEIN);
	return 1;
}


INT_PTR __cdecl Service_Status(WPARAM wParam, LPARAM lParam) {

    switch(--lParam) {
    case STATUS_DISABLED:
    case STATUS_ENABLED:
    case STATUS_ALWAYSTRY:
		pUinKey ptr = getUinKey((HANDLE)wParam);
		if(ptr) {
			ptr->status=ptr->tstatus=(BYTE)lParam;
			if(ptr->status==STATUS_ENABLED)	DBDeleteContactSetting(ptr->hContact, szModuleName, "StatusID");
			else 				DBWriteContactSettingByte(ptr->hContact, szModuleName, "StatusID", ptr->status);
		}
		break;
    }

	return 1;
}


INT_PTR __cdecl Service_StatusDis(WPARAM wParam, LPARAM lParam) {

	return Service_Status(wParam,STATUS_DISABLED+1);
}


INT_PTR __cdecl Service_StatusEna(WPARAM wParam, LPARAM lParam) {

	return Service_Status(wParam,STATUS_ENABLED+1);
}


INT_PTR __cdecl Service_StatusTry(WPARAM wParam, LPARAM lParam) {

	return Service_Status(wParam,STATUS_ALWAYSTRY+1);
}


INT_PTR __cdecl Service_PGPdelKey(WPARAM wParam, LPARAM lParam) {

	if(bPGPloaded) {
    	    DBDeleteContactSetting((HANDLE)wParam, szModuleName, "pgp");
    	    DBDeleteContactSetting((HANDLE)wParam, szModuleName, "pgp_mode");
    	    DBDeleteContactSetting((HANDLE)wParam, szModuleName, "pgp_abbr");
	}
	{
	    pUinKey ptr = getUinKey((HANDLE)wParam);
	    cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((HANDLE)wParam);
	return 1;
}


INT_PTR __cdecl Service_PGPsetKey(WPARAM wParam, LPARAM lParam) {

	BOOL del = true;
	if(bPGPloaded) {
    	if(bPGPkeyrings) {
	    char szKeyID[128]; szKeyID[0]='\0';
    	    PVOID KeyID = pgp_select_keyid(GetForegroundWindow(),szKeyID);
	    if(szKeyID[0]) {
    		DBDeleteContactSetting((HANDLE)wParam,szModuleName,"pgp");
    		DBCONTACTWRITESETTING cws;
    		memset(&cws,0,sizeof(cws));
    		cws.szModule = szModuleName;
    		cws.szSetting = "pgp";
    		cws.value.type = DBVT_BLOB;
    		cws.value.pbVal = (LPBYTE)KeyID;
    		cws.value.cpbVal = pgp_size_keyid();
    		CallService(MS_DB_CONTACT_WRITESETTING,wParam,(LPARAM)&cws);
    		DBWriteContactSettingByte((HANDLE)wParam,szModuleName,"pgp_mode",0);
    		DBWriteContactSettingString((HANDLE)wParam,szModuleName,"pgp_abbr",szKeyID);
    	  	del = false;
	    }
    	}
    	else
    	if(bPGPprivkey) {
    		char KeyPath[MAX_PATH]; KeyPath[0]='\0';
    	  	if(ShowSelectKeyDlg(0,KeyPath)){
    	  		char *publ = LoadKeys(KeyPath,false);
    	  		if(publ) {
    				DBDeleteContactSetting((HANDLE)wParam,szModuleName,"pgp");
    		  		myDBWriteStringEncode((HANDLE)wParam,szModuleName,"pgp",publ);
    				DBWriteContactSettingByte((HANDLE)wParam,szModuleName,"pgp_mode",1);
    				DBWriteContactSettingString((HANDLE)wParam,szModuleName,"pgp_abbr","(binary)");
    		  		mir_free(publ);
    		  		del = false;
    	  		}
    		}
    	}
	}

	if(del) Service_PGPdelKey(wParam,lParam);
	else {
		pUinKey ptr = getUinKey((HANDLE)wParam);
		cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((HANDLE)wParam);
	return 1;
}


INT_PTR __cdecl Service_GPGdelKey(WPARAM wParam, LPARAM lParam) {

	if(bGPGloaded) {
    	    DBDeleteContactSetting((HANDLE)wParam, szModuleName, "gpg");
	}
	{
	    pUinKey ptr = getUinKey((HANDLE)wParam);
	    cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((HANDLE)wParam);
	return 1;
}


INT_PTR __cdecl Service_GPGsetKey(WPARAM wParam, LPARAM lParam) {

	BOOL del = true;
	if(bGPGloaded && bGPGkeyrings) {
   		char szKeyID[128]; szKeyID[0]='\0';
		gpg_select_keyid(GetForegroundWindow(),szKeyID);
   		if(szKeyID[0]) {
   		    DBWriteContactSettingString((HANDLE)wParam,szModuleName,"gpg",szKeyID);
   	  		del = false;
   		}
	}

	if(del) Service_GPGdelKey(wParam,lParam);
	else {
		pUinKey ptr = getUinKey((HANDLE)wParam);
		cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((HANDLE)wParam);
	return 1;
}


INT_PTR __cdecl Service_Mode(WPARAM wParam, LPARAM lParam) {

	pUinKey ptr = getUinKey((HANDLE)wParam);

    switch(--lParam) {
    case MODE_NATIVE:
    case MODE_RSAAES:
    		if( isContactSecured((HANDLE)wParam)&SECURED ) {
    			msgbox(NULL, sim111, szModuleName, MB_OK);
    			return 0;
    		}
		if( lParam!=MODE_NATIVE && ptr->status>STATUS_ENABLED ) {
			Service_Status(wParam,STATUS_ENABLED+1);
		}
    case MODE_PGP:
    case MODE_GPG:
    		// нужно много проверок и отключение активного контекста если необходимо
		if(ptr) {
			if( ptr->cntx ) {
		    		cpp_delete_context(ptr->cntx);
				ptr->cntx = 0;
				ptr->keyLoaded = 0;
			}
			ptr->mode=(BYTE)lParam;
			DBWriteContactSettingByte((HANDLE)wParam, szModuleName, "mode", (BYTE)lParam);
		}
		ShowStatusIcon((HANDLE)wParam);
		break;
    }

    return 1;
}


INT_PTR __cdecl Service_ModeNative(WPARAM wParam, LPARAM lParam) {

	return Service_Mode(wParam,MODE_NATIVE+1);
}


INT_PTR __cdecl Service_ModePGP(WPARAM wParam, LPARAM lParam) {

	return Service_Mode(wParam,MODE_PGP+1);
}


INT_PTR __cdecl Service_ModeGPG(WPARAM wParam, LPARAM lParam) {

	return Service_Mode(wParam,MODE_GPG+1);
}


INT_PTR __cdecl Service_ModeRSAAES(WPARAM wParam, LPARAM lParam) {

	return Service_Mode(wParam,MODE_RSAAES+1);
}


// EOF
