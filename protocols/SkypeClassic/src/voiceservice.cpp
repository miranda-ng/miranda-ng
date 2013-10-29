#include "skype.h"
#include "skypeapi.h"
#include "skypesvc.h"
#include "voiceservice.h"
#include <m_voiceservice.h>

#pragma warning (push)
#pragma warning (disable: 4100) // unreferenced formal parameter
#include <m_utils.h>
#pragma warning (pop)

HANDLE hVoiceNotify = NULL;
BOOL has_voice_service = FALSE;

extern char g_szProtoName[];


BOOL HasVoiceService()
{
	return has_voice_service;
}

void NofifyVoiceService(HANDLE hContact, char *callId, int state) 
{
	VOICE_CALL vc = {0};
	vc.cbSize = sizeof(vc);
	vc.szModule = SKYPE_PROTONAME;
	vc.id = callId;
	vc.flags = VOICE_CALL_CONTACT;
	vc.state = state;
	vc.hContact = hContact;
	NotifyEventHooks(hVoiceNotify, (WPARAM) &vc, 0);
}

static INT_PTR VoiceGetInfo(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	return VOICE_SUPPORTED | VOICE_CALL_CONTACT | VOICE_CAN_HOLD;
}

static HANDLE FindContactByCallId(char *callId)
{
	HANDLE hContact;
	int iCmpRes;
	for (hContact = db_find_first();
			hContact != NULL;
			hContact = db_find_next(hContact))
	{
		char *szProto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		DBVARIANT dbv;
		if (szProto != NULL 
			&& !strcmp(szProto, SKYPE_PROTONAME) 
			&& db_get_b(hContact, SKYPE_PROTONAME, "ChatRoom", 0) == 0 
			&& !db_get_s(hContact, SKYPE_PROTONAME, "CallId", &dbv)) 
		{
			iCmpRes = strcmp(callId, dbv.pszVal);
			db_free(&dbv);
			if (iCmpRes == 0) return hContact;
		}
	}

	return NULL;
}

static INT_PTR VoiceCall(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;

	UNREFERENCED_PARAMETER(lParam);

	if (!wParam) return -1;

	if (db_get_s((HANDLE)wParam, SKYPE_PROTONAME, SKYPE_NAME, &dbv)) 
		return -1;

	SkypeSend("CALL %s", dbv.pszVal);
	db_free (&dbv);

	return 0;
}

static INT_PTR VoiceAnswer(WPARAM wParam, LPARAM lParam)
{
	char *callId = (char *) wParam;

	UNREFERENCED_PARAMETER(lParam);

	if (!wParam) return -1;
	
	if (FindContactByCallId(callId) == NULL)
		return -1;

	SkypeSend("SET %s STATUS INPROGRESS", callId);
	testfor("ERROR", 200);

	return 0;
}

static INT_PTR VoiceDrop(WPARAM wParam, LPARAM lParam)
{
	char *callId = (char *) wParam;

	UNREFERENCED_PARAMETER(lParam);

	if (!wParam) return -1;

	if (FindContactByCallId(callId) == NULL)
		return -1;

	SkypeSend("SET %s STATUS FINISHED", callId);

	return 0;
}

static INT_PTR VoiceHold(WPARAM wParam, LPARAM lParam)
{
	char *callId = (char *) wParam;

	UNREFERENCED_PARAMETER(lParam);

	if (!wParam) return -1;

	if (FindContactByCallId(callId) == NULL)
		return -1;

	SkypeSend("SET %s STATUS ONHOLD", callId);

	return 0;
}

void VoiceServiceInit() 
{
	// leecher, 26.03.2011: Did this ever work in the old versions?? 
	char szEvent[MAXMODULELABELLENGTH];

	_snprintf (szEvent, sizeof(szEvent), "%s%s", SKYPE_PROTONAME, PE_VOICE_CALL_STATE);
	hVoiceNotify = CreateHookableEvent( szEvent );
	CreateProtoService( PS_VOICE_GETINFO, VoiceGetInfo );
	CreateProtoService( PS_VOICE_CALL, VoiceCall );
	CreateProtoService( PS_VOICE_ANSWERCALL, VoiceAnswer );
	CreateProtoService( PS_VOICE_DROPCALL, VoiceDrop );
	CreateProtoService( PS_VOICE_HOLDCALL, VoiceHold );
}

void VoiceServiceExit()
{
	DestroyHookableEvent(hVoiceNotify);
}

void VoiceServiceModulesLoaded() 
{
	has_voice_service = ServiceExists(MS_VOICESERVICE_REGISTER);
}