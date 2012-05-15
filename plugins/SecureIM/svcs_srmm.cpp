#include "commonheaders.h"


int __cdecl onWindowEvent(WPARAM wParam, LPARAM lParam) {

	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;
	if(mwd->uType == MSG_WINDOW_EVT_OPEN || mwd->uType == MSG_WINDOW_EVT_OPENING) {
		ShowStatusIcon(mwd->hContact);
	}
	return 0;
}


int __cdecl onIconPressed(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	if( isProtoMetaContacts(hContact) )
		hContact = getMostOnline(hContact); // возьмем тот, через который пойдет сообщение

	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if( strcmp(sicd->szModule, szModuleName) != 0 ||
		!isSecureProtocol(hContact) ) return 0; // not our event

	BOOL isPGP = isContactPGP(hContact);
	BOOL isGPG = isContactGPG(hContact);
	BOOL isSecured = isContactSecured(hContact)&SECURED;
	BOOL isChat = isChatRoom(hContact);

	if( !isPGP && !isGPG && !isChat ) {
		if(isSecured)	Service_DisableIM(wParam,0);
		else		Service_CreateIM(wParam,0);
	}

	return 0;
}


// EOF
