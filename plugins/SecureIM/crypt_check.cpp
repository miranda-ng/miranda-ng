#include "commonheaders.h"


int getContactStatus(HANDLE hContact) {

	pSupPro ptr = getSupPro(hContact);
	if (ptr)
		return DBGetContactSettingWord(hContact, ptr->name, "Status", ID_STATUS_OFFLINE);

	return -1;
}


BOOL isSecureProtocol(HANDLE hContact) {

	pSupPro ptr = getSupPro(hContact);
	if(!ptr) return false;

	return ptr->inspecting;
}


BYTE isContactSecured(HANDLE hContact) {
	// нужна проверка на Offline и в этом случае другие статусы
	if (!clist_cnt) return 0;

	BYTE r=0;
	if( isProtoMetaContacts(hContact) )
		hContact = getMostOnline(hContact); // возьмем тот, через который пойдет сообщение

	for(int j=0;j<clist_cnt;j++) {
		if( clist[j].hContact == hContact ) {
			if( !clist[j].proto->inspecting ) break;
       			DBVARIANT dbv;
			r=clist[j].mode;
			switch(r) {
			case MODE_NATIVE:
				if(cpp_keyx(clist[j].cntx)!=0) r|=SECURED;
				break;
			case MODE_PGP:
				DBGetContactSetting(hContact,szModuleName,"pgp",&dbv);
				if( dbv.type!=0 ) r|=SECURED;
				DBFreeVariant(&dbv);
				break;
			case MODE_GPG:
				DBGetContactSetting(hContact,szModuleName,"gpg",&dbv);
				if( dbv.type!=0 ) r|=SECURED;
				DBFreeVariant(&dbv);
				break;
			case MODE_RSAAES:
				if(exp->rsa_get_state(clist[j].cntx)==7) r|=SECURED;
				break;
			case MODE_RSA:
				if(clist[j].cntx) r|=SECURED;
				break;
			}
			break;
		}
	}
	return r; // (mode&SECURED) - проверка на EST/DIS
}


BOOL isClientMiranda(pUinKey ptr, BOOL emptyMirverAsMiranda) {

	if( !bMCD ) return true;
	if( !ptr->proto->inspecting ) return false;

	BOOL isMiranda = true;
	LPSTR mirver = myDBGetString(ptr->hContact,ptr->proto->name,"MirVer");
	if( mirver ) {
		isMiranda = (emptyMirverAsMiranda && !*mirver) || (strstr(mirver,"Miranda")!=NULL);
		mir_free(mirver);
	}
	return isMiranda;
}


BOOL isClientMiranda(HANDLE hContact, BOOL emptyMirverAsMiranda) {

	if( !bMCD ) return true;
	if( !clist_cnt ) return false;

	for(int j=0;j<clist_cnt;j++) {
		if( clist[j].hContact == hContact ) {
			return isClientMiranda(&clist[j],emptyMirverAsMiranda);
		}
	}
	return false;
}


BOOL isProtoSmallPackets(HANDLE hContact) {

	if (!clist_cnt) return false;
	for(int j=0;j<clist_cnt;j++) {
		if( clist[j].hContact == hContact ) {
			if( !clist[j].proto->inspecting ) break;
				return  strstr(clist[j].proto->name,"IRC")!=NULL ||
					strstr(clist[j].proto->name,"WinPopup")!=NULL ||
					strstr(clist[j].proto->name,"VyChat")!=NULL;
		}
	}
	return false;
}


BOOL isContactInvisible(HANDLE hContact) {

	if( !DBGetContactSettingByte(hContact,"CList","Hidden",0) ) {
		if( !clist_cnt ) return false;
		for(int j=0;j<clist_cnt;j++) {
			if( clist[j].hContact == hContact ) {
				if( !clist[j].proto->inspecting ) return false;
				if( clist[j].waitForExchange ) return false;
				switch( (int)DBGetContactSettingWord(hContact,clist[j].proto->name,"ApparentMode",0) ) {
				case 0:
					return (CallProtoService(clist[j].proto->name,PS_GETSTATUS,0,0)==ID_STATUS_INVISIBLE);
				case ID_STATUS_ONLINE:
					return false;
				case ID_STATUS_OFFLINE:
					return true;
				} //switch
				break;
			}
		}// for
	}
	return true;
}


BOOL isNotOnList(HANDLE hContact) {
	return DBGetContactSettingByte(hContact, "CList", "NotOnList", 0);
}


BOOL isContactNewPG(HANDLE hContact) {

	if (!clist_cnt) return false;
	for(int j=0;j<clist_cnt;j++) {
		if (clist[j].hContact == hContact) {
			if( !clist[j].proto->inspecting ) break;
			if( !clist[j].cntx ) break;
			return (clist[j].features & CPP_FEATURES_NEWPG) != 0;
		}
	}
	return false;
}


BOOL isContactPGP(HANDLE hContact) {

	if(!bPGPloaded || (!bPGPkeyrings && !bPGPprivkey)) return false;
	if (!clist_cnt) return false;
	for(int j=0;j<clist_cnt;j++) {
	    if (clist[j].hContact == hContact) {
		if( !clist[j].proto->inspecting ) break;
	    	if( clist[j].mode!=MODE_PGP ) break;
        	DBVARIANT dbv;
        	DBGetContactSetting(hContact,szModuleName,"pgp",&dbv);
        	BOOL r=(dbv.type!=0);
        	DBFreeVariant(&dbv);
        	return r;
	    }
	}
	return false;
}


BOOL isContactGPG(HANDLE hContact) {

	if(!bGPGloaded || !bGPGkeyrings) return false;
	if (!clist_cnt) return false;
	for(int j=0;j<clist_cnt;j++) {
	    if (clist[j].hContact == hContact) {
		if( !clist[j].proto->inspecting ) break;
	    	if( clist[j].mode!=MODE_GPG ) break;
        	DBVARIANT dbv;
        	DBGetContactSetting(hContact,szModuleName,"gpg",&dbv);
        	BOOL r=(dbv.type!=0);
        	DBFreeVariant(&dbv);
        	return r;
	    }
	}
	return false;
}


BOOL isContactRSAAES(HANDLE hContact) {

	if (!clist_cnt) return false;
        for(int j=0;j<clist_cnt;j++) {
		if (clist[j].hContact == hContact) {
			if( !clist[j].proto->inspecting ) break;
			if( clist[j].mode!=MODE_RSAAES ) break;
        		return true;
		}
	}
	return false;
}


BOOL isContactRSA(HANDLE hContact) {

	if (!clist_cnt) return false;
        for(int j=0;j<clist_cnt;j++) {
		if (clist[j].hContact == hContact) {
			if( !clist[j].proto->inspecting ) break;
			if( clist[j].mode!=MODE_RSA ) break;
        	return true;
		}
	}
	return false;
}


BOOL isChatRoom(HANDLE hContact) {

	if (!clist_cnt) return false;
	for(int j=0;j<clist_cnt;j++) {
		if( clist[j].hContact == hContact ) {
			if( !clist[j].proto->inspecting ) break;
			return (DBGetContactSettingByte(hContact,clist[j].proto->name,"ChatRoom",0)!=0);
		}
	}
	return false;
}


BOOL isFileExist(LPCSTR filename) {
	return (GetFileAttributes(filename)!=(UINT)-1);
}


BOOL isSecureIM(pUinKey ptr, BOOL emptyMirverAsSecureIM) {

	if( !bAIP ) return false;
	if( !ptr->proto->inspecting ) return false;

	BOOL isSecureIM = false;
	if( bNOL && DBGetContactSettingByte(ptr->hContact,"CList","NotOnList",0) ) {
		return false;
	}
	LPSTR mirver = myDBGetString(ptr->hContact,ptr->proto->name,"MirVer");
	if( mirver ) {
		isSecureIM = (emptyMirverAsSecureIM && !*mirver) || (strstr(mirver,"SecureIM")!=NULL) || (strstr(mirver,"secureim")!=NULL);
		mir_free(mirver);
	}
	return isSecureIM;
}


BOOL isSecureIM(HANDLE hContact, BOOL emptyMirverAsSecureIM) {

	if( !bAIP ) return false;
	if( !clist_cnt ) return false;

	for(int j=0;j<clist_cnt;j++) {
		if (clist[j].hContact == hContact) {
			return isSecureIM(&clist[j],emptyMirverAsSecureIM);
		}
	}
	return false;
}


// EOF
