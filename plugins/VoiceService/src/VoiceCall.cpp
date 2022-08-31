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

VoiceCall::VoiceCall(VoiceProvider *module, const char *id)	: 
	CDlgBase(g_plugin, IDD_VOICECALL),
	module(module),
	id(mir_strdup(id)), 
	m_btnAnswer(this, IDC_ANSWERBTN),
	m_btnDrop(this, IDC_DROPBTN),
	m_lblStatus(this, IDC_STATUS),
	m_calltimer(this, 1),
	m_lblContactName(this, IDC_DESCR),
	m_lblAddress(this, IDC_ADDRESS)
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
	m_nsec = 0;

	m_btnAnswer.OnClick = Callback(this, &VoiceCall::OnCommand_Answer);
	m_btnDrop.OnClick = Callback(this, &VoiceCall::OnCommand_Drop);
	m_calltimer.OnEvent = Callback(this, &VoiceCall::OnCallTimer);

	CreateDisplayName();
	Show(SW_SHOWNOACTIVATE);
}

VoiceCall::~VoiceCall()
{
	RemoveNotifications();
	mir_free(id);
	id = NULL;
	DeleteObject(hContactNameFont);
}

bool VoiceCall::OnInitDialog()
{
	Button_SetIcon_IcoLib(m_hwnd, IDC_DROPBTN, g_plugin.getIconHandle(IDI_ACTION_DROP));
	Button_SetIcon_IcoLib(m_hwnd, IDC_ANSWERBTN, g_plugin.getIconHandle(IDI_ACTION_ANSWER));

	HFONT hf = (HFONT) SendMessage(m_lblContactName.GetHwnd(), WM_GETFONT, 0,0);
	LOGFONT lf;
	GetObject(hf, sizeof(LOGFONT), &lf);
	lf.lfWeight = FW_BOLD;
	hContactNameFont = CreateFontIndirect(&lf);
	SendMessage(m_lblContactName.GetHwnd(), WM_SETFONT, (WPARAM)hContactNameFont, 0);

	return true;
}

bool VoiceCall::OnClose()
{
	Button_FreeIcon_IcoLib(m_hwnd, IDC_DROPBTN);
	Button_FreeIcon_IcoLib(m_hwnd, IDC_ANSWERBTN);

	calls.remove(this);
	return false;
}

void VoiceCall::OnCommand_Answer(CCtrlButton *)
{
	Answer();
}

void VoiceCall::OnCommand_Drop(CCtrlButton *)
{
	if (state == VOICE_STATE_ENDED || state == VOICE_STATE_BUSY)
		Close();
	else
		Drop();
}

void VoiceCall::OnCallTimer(CTimer *)
{
	m_nsec++;
	wchar_t text[256], hrs[11];
	hrs[0] = 0;
	int mins = m_nsec / 60;
	int hours = m_nsec / 3600;
	if (hours)
		mir_snwprintf(hrs, _countof(hrs), L"%d:", hours);
	mir_snwprintf(text, _countof(text), L"%s%d:%02d", hrs, (mins) % 60, m_nsec % 60);
	m_lblStatus.SetText(text);
}

void VoiceCall::AppendCallerID(MCONTACT aHContact, const wchar_t *aName, const wchar_t *aNumber)
{
	bool changed = false;

	if (aHContact != NULL) {
		hContact = aHContact;
		changed = true;

		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
		if (ace && (INT_PTR)ace != CALLSERVICE_NOTFOUND && (ace->dwFlags & AVS_BITMAP_VALID))
			SendMessage(GetDlgItem(m_hwnd, IDC_AVATAR), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)ace->hbmPic);
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
			mir_snwprintf(displayName, _T("%s <%s>"), contact, number);
		else
			wcsncpy_s(displayName, contact, _TRUNCATE);
	}
	else if (!IsEmptyW(nameTmp) && !IsEmptyW(number)) {
		mir_snwprintf(displayName, _T("%s <%s>"), name, number);
	}
	else if (!IsEmptyW(nameTmp)) {
		wcsncpy_s(displayName, name, _TRUNCATE);
	}
	else if (!IsEmptyW(number)) {
		wcsncpy_s(displayName, number, _TRUNCATE);
	}
	else {
		wcsncpy_s(displayName, TranslateT("Unknown number"), _TRUNCATE);
	}

	if (contact)
		m_lblContactName.SetText(contact);
	if (number[0])
		m_lblAddress.SetText(number);
}

void VoiceCall::RemoveNotifications()
{
	if (hwnd != NULL) {
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
/*
	if (clistBlinking) {
		g_clistApi.pfnRemoveEvent(hContact, MEVENT(0xBABABEDA));
		clistBlinking = false;
	}*/
}

void VoiceCall::SetState(int aState)
{
	if (state == aState)
		return;
	state = aState;

	RemoveNotifications();

	switch (state) {
	case VOICE_STATE_TALKING:
		m_calltimer.Start(1000);
		m_lblStatus.SetText(L"0:00");
		m_btnAnswer.Enable(false);
		break;
	case VOICE_STATE_RINGING:
		incoming = true;
		SetCaption(TranslateT("Incoming call"));
		m_btnAnswer.Enable(true);
		m_lblStatus.SetText(TranslateT("Ringing"));
		SetWindowPos(GetHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetWindowPos(GetHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		break;
	case VOICE_STATE_CALLING:
		incoming = false;
		SetCaption(TranslateT("Outgoing call"));
		m_lblStatus.SetText(TranslateT("Calling"));
		m_btnAnswer.Enable(false);
		break;
	case VOICE_STATE_ON_HOLD:
		m_lblStatus.SetText(TranslateT("Holded"));
		m_btnAnswer.Enable(true);
		m_btnAnswer.SetText(TranslateT("Unhold"));
		break;
	case VOICE_STATE_ENDED:
		m_calltimer.Stop();
		m_lblStatus.SetText(TranslateT("Call ended"));
		m_btnAnswer.Enable(false);
		m_btnDrop.SetText(TranslateT("Close"));
		break;
	case VOICE_STATE_BUSY:
		m_lblStatus.SetText(TranslateT("Busy"));
		m_btnAnswer.Enable(false);
		m_btnDrop.SetText(TranslateT("Close"));
		break;
	default:
		m_lblStatus.SetText(TranslateT("Unknown state"));
		break;
	}

	if (state != VOICE_STATE_ON_HOLD)
		m_nsec = 0;

	if (IsFinished()) {
		if (end_time == 0)
			end_time = GetTickCount();

		// Remove id because providers can re-use them
		mir_free(id);
		id = NULL;
	}

	Notify();
}

void VoiceCall::Notify(bool popup, bool sound, bool /*clist*/)
{
	if (popup) {
		wchar_t text[512];
		mir_snwprintf(text, _countof(text), TranslateW(stateTexts[state]), displayName);

		ShowPopup(NULL, TranslateW(popupTitles[state]), text);
	}

	if (sound)
		Skin_PlaySound(g_sounds[state].szName);
/*
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
	}*/
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
