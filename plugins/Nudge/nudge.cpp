#include "headers.h"
#include "nudge.h"

void CNudge::Save(void)
{
	char SectionName[512];
	mir_snprintf(SectionName,512,"useByProtocol"); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->useByProtocol);
	mir_snprintf(SectionName,512,"RecvTimeSec");
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->recvTimeSec);
	mir_snprintf(SectionName,512,"SendTimeSec");
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->sendTimeSec);
	mir_snprintf(SectionName,512,"ResendDelaySec");
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->resendDelaySec);
}


void CNudge::Load(void)
{
	char SectionName[512];
	mir_snprintf(SectionName,512,"useByProtocol"); 
	this->useByProtocol = DBGetContactSettingByte(NULL, "Nudge", SectionName, FALSE) != 0;
	mir_snprintf(SectionName,512,"RecvTimeSec");
	this->recvTimeSec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 30);
	mir_snprintf(SectionName,512,"SendTimeSec");
	this->sendTimeSec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 30);
	mir_snprintf(SectionName,512,"ResendDelaySec");
	this->resendDelaySec = DBGetContactSettingDword(NULL, "Nudge", SectionName, 3);
}

int CNudgeElement::ShowContactMenu(bool show)
{

	CLISTMENUITEM mi;
	memset( &mi, 0, sizeof( mi ));
	mi.cbSize = sizeof( mi );
	mi.flags =  show? CMIM_FLAGS : CMIM_FLAGS | CMIF_HIDDEN ;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hContactMenu, (LPARAM)&mi);

	return 0;
}

void CNudgeElement::Save(void)
{
	char SectionName[512];
	mir_snprintf(SectionName,512,"%s-openMessageWindow", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->openMessageWindow);
	mir_snprintf(SectionName,512,"%s-openContactList", ProtocolName);
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->openContactList);
	mir_snprintf(SectionName,512,"%s-useIgnore", ProtocolName);
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->useIgnoreSettings);
	mir_snprintf(SectionName,512,"%s-showStatus", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->showStatus); 
	mir_snprintf(SectionName,512,"%s-showPopup", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->showPopup); 
	mir_snprintf(SectionName,512,"%s-shakeClist", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->shakeClist); 
	mir_snprintf(SectionName,512,"%s-shakeChat", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->shakeChat); 
	mir_snprintf(SectionName,512,"%s-enabled", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->enabled);
	mir_snprintf(SectionName,512,"%s-autoResend", ProtocolName); 
	DBWriteContactSettingByte(NULL, "Nudge", SectionName, this->autoResend);
	mir_snprintf(SectionName,512,"%s-statusFlags", ProtocolName);
	DBWriteContactSettingDword(NULL, "Nudge", SectionName, this->statusFlags);
	mir_snprintf(SectionName,512,"%s-recText", ProtocolName);
	DBWriteContactSettingTString(NULL, "Nudge", SectionName, this->recText);
	mir_snprintf(SectionName,512,"%s-senText", ProtocolName);
	DBWriteContactSettingTString(NULL, "Nudge", SectionName, this->senText);
}


void CNudgeElement::Load(void)
{
	DBVARIANT dbv;
	char SectionName[512];
	mir_snprintf(SectionName,512,"%s-openMessageWindow", ProtocolName); 
	this->openMessageWindow = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	mir_snprintf(SectionName,512,"%s-openContactList", ProtocolName); 
	this->openContactList = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	mir_snprintf(SectionName,512,"%s-useIgnore", ProtocolName); 
	this->useIgnoreSettings = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	mir_snprintf(SectionName,512,"%s-showStatus", ProtocolName); 
	this->showStatus = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	mir_snprintf(SectionName,512,"%s-showPopup", ProtocolName); 
	this->showPopup = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	mir_snprintf(SectionName,512,"%s-shakeClist", ProtocolName); 
	this->shakeClist = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;  
	mir_snprintf(SectionName,512,"%s-shakeChat", ProtocolName); 
	this->shakeChat = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0; 
	mir_snprintf(SectionName,512,"%s-enabled", ProtocolName); 
	this->enabled = DBGetContactSettingByte(NULL, "Nudge", SectionName, TRUE) != 0;
	mir_snprintf(SectionName,512,"%s-autoResend", ProtocolName); 
	this->autoResend = DBGetContactSettingByte(NULL, "Nudge", SectionName, FALSE) != 0;
	mir_snprintf(SectionName,512,"%s-statusFlags", ProtocolName);
	this->statusFlags = DBGetContactSettingDword(NULL, "Nudge", SectionName, 967);
	mir_snprintf(SectionName,512,"%s-recText", ProtocolName);
	if(!DBGetContactSettingTString(NULL,"Nudge",SectionName,&dbv)) 
	{
		_tcsncpy(this->recText,dbv.ptszVal,TEXT_LEN);
		if(_tcsclen(this->recText) < 1)
			_tcsncpy(this->recText,TranslateT("You received a nudge"),TEXT_LEN);
		DBFreeVariant(&dbv);
	}
	else
		_tcsncpy(this->recText,TranslateT("You received a nudge"),TEXT_LEN);
	mir_snprintf(SectionName,512,"%s-senText", ProtocolName);
	if(!DBGetContactSettingTString(NULL,"Nudge",SectionName,&dbv)) 
	{
		_tcsncpy(this->senText,dbv.ptszVal,TEXT_LEN);
		if(_tcsclen(this->senText) < 1)
			_tcsncpy(this->senText,TranslateT("You sent a nudge"),TEXT_LEN);
		DBFreeVariant(&dbv);
	}
	else
		_tcsncpy(this->senText,TranslateT("You sent a nudge"),TEXT_LEN);
}