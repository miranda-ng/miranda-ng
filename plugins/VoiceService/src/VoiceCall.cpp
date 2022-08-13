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

static wchar_t *popupTitles[] = {
	LPGENW("Voice call started"),
	LPGENW("Voice call ringing"),
	LPGENW("Voice call"),
	LPGENW("Voice call on hold"),
	LPGENW("Voice call ended"),
	LPGENW("Voice call busy"),
};

static wchar_t *stateTexts[] = {
	LPGENW("Call from %s has started"),
	LPGENW("Call from %s is ringing"),
	LPGENW("Calling %s"),
	LPGENW("Call from %s is on hold"),
	LPGENW("Call from %s has ended"),
	LPGENW("%s is busy"),
};

/////////////////////////////////////////////////////////////////////////////////////////
// VoiceCall class members

VoiceCall::VoiceCall(VoiceProvider *module, const char *id)
	: module(module), id(mir_strdup(id))
{
	hContact = NULL;
	name[0] = 0;
	number[0] = 0;
	displayName[0] = 0;
	state = -1;
	end_time = 0;
	clistBlinking = false;
	incoming = false;
	secure = false;
	hwnd = NULL;
	CreateDisplayName();
}

VoiceCall::~VoiceCall()
{
	RemoveNotifications();
	mir_free(id);
	id = NULL;
}

void VoiceCall::AppendCallerID(MCONTACT aHContact, const wchar_t *aName, const wchar_t *aNumber)
{
	bool changed = false;

	if (aHContact != NULL) {
		hContact = aHContact;
		changed = true;
	}

	if (!IsEmptyW(aName)) {
		lstrcpyn(name, aName, _countof(name));
		changed = true;
	}

	if (!IsEmptyW(aNumber)) {
		lstrcpyn(number, aNumber, _countof(number));
		changed = true;
	}

	if (changed)
		CreateDisplayName();
}

void VoiceCall::CreateDisplayName()
{
	wchar_t *contact = NULL;
	if (hContact != NULL)
		contact = Clist_GetContactDisplayName(hContact, 0);

	wchar_t *nameTmp = NULL;
	if (lstrcmp(name, number) != 0)
		nameTmp = name;

	if (contact != NULL) {
		if (!IsEmptyW(number))
			mir_snwprintf(displayName, _countof(displayName), _T("%s <%s>"), contact, number);
		else
			lstrcpyn(displayName, contact, _countof(displayName));
	}
	else if (!IsEmptyW(nameTmp) && !IsEmptyW(number)) {
		mir_snwprintf(displayName, _countof(displayName), _T("%s <%s>"), name, number);
	}
	else if (!IsEmptyW(nameTmp)) {
		lstrcpyn(displayName, name, _countof(displayName));
	}
	else if (!IsEmptyW(number)) {
		lstrcpyn(displayName, number, _countof(displayName));
	}
	else {
		lstrcpyn(displayName, TranslateT("Unknown number"), _countof(displayName));
	}
}

void VoiceCall::RemoveNotifications()
{
	if (hwnd != NULL) {
		DestroyWindow(hwnd);
		hwnd = NULL;
	}

	if (clistBlinking) {
		g_clistApi.pfnRemoveEvent(hContact, (LPARAM)this);
		clistBlinking = false;
	}
}

void VoiceCall::SetState(int aState)
{
	if (state == aState)
		return;

	if (aState == VOICE_STATE_RINGING)
		incoming = true;
	else if (aState == VOICE_STATE_CALLING)
		incoming = false;

	RemoveNotifications();

	state = aState;

	if (IsFinished()) {
		if (end_time == 0)
			end_time = GetTickCount();

		// Remove id because providers can re-use them
		mir_free(id);
		id = NULL;
	}

	Notify();
}

void VoiceCall::Notify(bool popup, bool sound, bool clist)
{
	if (popup) {
		wchar_t text[512];
		mir_snwprintf(text, _countof(text), TranslateW(stateTexts[state]), displayName);

		ShowPopup(NULL, TranslateW(popupTitles[state]), text);
	}

	if (sound)
		Skin_PlaySound(g_sounds[state].szName);

	if (clist && state == VOICE_STATE_RINGING) {
		CLISTEVENT ce = {};
		ce.hContact = hContact;
		ce.hIcon = g_plugin.getIcon(IDI_RINGING);
		ce.hDbEvent = 1001;
		ce.pszService = MS_VOICESERVICE_CLIST_DBLCLK;
		ce.lParam = (LPARAM)this;
		g_clistApi.pfnAddEvent(&ce);

		IcoLib_ReleaseIcon(ce.hIcon);

		clistBlinking = true;
	}

	RefreshFrame();
}

bool VoiceCall::IsFinished()
{
	return IsFinalState(state);
}

bool VoiceCall::CanDrop()
{
	return !IsFinished();
}

void VoiceCall::Drop()
{
	if (!CanDrop())
		return;

	RemoveNotifications();

	CallProtoService(module->name, PS_VOICE_DROPCALL, (WPARAM)id, 0);
}

bool VoiceCall::CanAnswer()
{
	return state == -1 || state == VOICE_STATE_RINGING || state == VOICE_STATE_ON_HOLD;
}

void VoiceCall::Answer()
{
	if (!CanAnswer())
		return;

	RemoveNotifications();

	CallProtoService(module->name, PS_VOICE_ANSWERCALL, (WPARAM)id, 0);
}

bool VoiceCall::CanHold()
{
	return module->CanHold() && (state == -1 || state == VOICE_STATE_TALKING);
}

void VoiceCall::Hold()
{
	if (!CanHold())
		return;

	RemoveNotifications();

	CallProtoService(module->name, PS_VOICE_HOLDCALL, (WPARAM)id, 0);
}

bool VoiceCall::CanSendDTMF()
{
	return module->CanSendDTMF() && state == VOICE_STATE_TALKING;
}

void VoiceCall::SendDTMF(wchar_t c)
{
	if (!CanSendDTMF())
		return;

	CallProtoService(module->name, PS_VOICE_SEND_DTMF, (WPARAM)id, (LPARAM)c);
}

void VoiceCall::SetNewCallHWND(HWND _h)
{
	if (_h != NULL)
		RemoveNotifications();

	hwnd = _h;
}
