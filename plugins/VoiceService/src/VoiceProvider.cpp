/*
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

static bool IsProtocol(const char *module)
{
	for (auto *pa : Accounts()) {
		if (pa->szModuleName == NULL || pa->szModuleName[0] == '\0')
			continue;

		if (strcmp(module, pa->szModuleName) == 0)
			return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// VoiceProvider class members

static int VoiceState(WPARAM wParam, LPARAM)
{
	VOICE_CALL *in = (VOICE_CALL *)wParam;
	if (in == NULL || in->cbSize < sizeof(VOICE_CALL) || in->moduleName == NULL || in->id == NULL)
		return 0;

	// Check if the call is aready in list
	VoiceCall *call = FindVoiceCall(in->moduleName, in->id, !IsFinalState(in->state));
	if (call == NULL)
		return 0;
	
	if(in->state == VOICE_STATE_SETSTATUS) {
		call->SetStatus(in->szStatus);
		return 0;
	}

	call->AppendCallerID(in->hContact,
		(in->flags & VOICE_UNICODE) ? in->szName.w : _A2T(in->szName.a),
		(in->flags & VOICE_UNICODE) ? in->szNumber.w : _A2T(in->szNumber.a));
	call->secure = (in->flags & VOICE_SECURE) != 0;

	if (in->state == VOICE_STATE_RINGING && call->hContact != NULL) {
		int aut = g_plugin.getWord(call->hContact, "AutoAccept", AUTO_NOTHING);
		if (aut == AUTO_ACCEPT || aut == AUTO_DROP) {
			call->state = VOICE_STATE_RINGING;
			call->Notify(false, false, false);

			if (aut == AUTO_ACCEPT)
				Answer(call);
			else
				call->Drop();

			return 0;
		}
	}

	if (in->state == VOICE_STATE_TALKING)
		HoldOtherCalls(call);

	call->SetState(in->state);

	return 0;
}

VoiceProvider::VoiceProvider(const char *name, const wchar_t *description, int flags, HANDLE pIcon) :
	hIcolib(pIcon)
{
	strncpy_s(this->name, name, _TRUNCATE);
	wcsncpy_s(this->description, description, _TRUNCATE);

	this->flags = flags;
	is_protocol = IsProtocol(name);
	canHold = (ProtoServiceExists(name, PS_VOICE_HOLDCALL) != 0);

	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, "%s%s", name, PE_VOICE_CALL_STATE);
	state_hook = HookEvent(str, VoiceState);
}

VoiceProvider::~VoiceProvider()
{
	UnhookEvent(state_hook);
	state_hook = NULL;
}

bool VoiceProvider::CanCall(MCONTACT hContact, BOOL now)
{
	if (hContact == NULL)
		return false;

	if ((flags & VOICE_CAPS_CALL_CONTACT) == 0)
		return false;

	if (ProtoServiceExists(name, PS_VOICE_CALL_CONTACT_VALID))
		return CallProtoService(name, PS_VOICE_CALL_CONTACT_VALID, (WPARAM)hContact, now) != FALSE;

	if (is_protocol) {
		if (now && CallProtoService(name, PS_GETSTATUS, 0, 0) <= ID_STATUS_OFFLINE)
			return false;

		if (!Proto_IsProtoOnContact(hContact, name))
			return false;

		return db_get_w(hContact, name, "Status", ID_STATUS_OFFLINE) > ID_STATUS_OFFLINE;
	}

	return true;
}

bool VoiceProvider::CanCall(const wchar_t *number)
{
	if (number == NULL || number[0] == 0)
		return false;

	if ((flags & VOICE_CAPS_CALL_STRING) == 0)
		return false;

	if (ProtoServiceExists(name, PS_VOICE_CALL_STRING_VALID))
		return CallProtoService(name, PS_VOICE_CALL_STRING_VALID, (WPARAM)number, 0) != FALSE;

	if (is_protocol)
		return CallProtoService(name, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;

	return true;
}

bool VoiceProvider::CanHold()
{
	return canHold;
}

bool VoiceProvider::CanSendDTMF()
{
	return ProtoServiceExists(name, PS_VOICE_SEND_DTMF) != FALSE;
}

void VoiceProvider::Call(MCONTACT hContact, const wchar_t *number)
{
	CallProtoService(name, PS_VOICE_CALL, (WPARAM)hContact, (LPARAM)number);
}

HICON VoiceProvider::GetIcon()
{
	if (hIcolib)
		return IcoLib_GetIconByHandle(hIcolib);

	if (is_protocol)
		return Skin_LoadProtoIcon(name, ID_STATUS_ONLINE);

	return NULL;
}

void VoiceProvider::ReleaseIcon(HICON hIcon)
{
	if (hIcolib)
		IcoLib_ReleaseIcon(hIcon);
}
