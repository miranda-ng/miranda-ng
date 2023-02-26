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

static wchar_t *popupTexts[] = {
	LPGENW("Voice call started"),
	LPGENW("Voice call ringing"),
	LPGENW("Voice call"),
	LPGENW("Voice call on hold"),
	LPGENW("Voice call ended"),
	LPGENW("Voice call busy"),
	LPGENW("Voice call ready"),
};

struct {
	wchar_t *status;
	wchar_t *btn1text;
	wchar_t *btn2text;
}
static stateTexts[NUM_STATES] = {
	{ L"0:00",           LPGENW("Hold"),   LPGENW("Drop")   },
	{ LPGENW("Ringing"),        LPGENW("Answer"), LPGENW("Drop")   },
	{ LPGENW("Calling"),        nullptr,   LPGENW("Drop")   },
	{ LPGENW("Holded"),         LPGENW("Resume"), LPGENW("Drop")   },
	{ nullptr,           nullptr,   LPGENW("Close")  },
	{ LPGENW("Callee is busy"), nullptr,   LPGENW("Close")  },
	{ LPGENW("Ready"),          LPGENW("Call"),   LPGENW("Cancel") },
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
	name[0] = 0;
	number[0] = 0;
	displayName[0] = 0;

	m_btnAnswer.OnClick = Callback(this, &VoiceCall::OnCommand_Answer);
	m_btnDrop.OnClick = Callback(this, &VoiceCall::OnCommand_Drop);
	m_calltimer.OnEvent = Callback(this, &VoiceCall::OnCallTimer);

	CreateDisplayName();
	Create();
}

VoiceCall::~VoiceCall()
{
	RemoveNotifications();
	mir_free(id);
	id = 0;
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
		mir_snwprintf(hrs, L"%d:", hours);
	mir_snwprintf(text, L"%s%d:%02d", hrs, (mins) % 60, m_nsec % 60);
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
		wcsncpy_s(name, aName, _TRUNCATE);
		changed = true;
	}

	if (!IsEmptyW(aNumber)) {
		wcsncpy_s(number, aNumber, _TRUNCATE);
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
	if (clistBlinking) {
		g_clistApi.pfnRemoveEvent(hContact, MEVENT(1001));
		clistBlinking = false;
	}

	if (soundActive) {
		Skin_PlaySoundFile(nullptr);
		soundActive = false;
	}
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
		break;
	case VOICE_STATE_RINGING:
		incoming = true;
		if(opts.opt_bImmDialog) {
			SetWindowPos(GetHwnd(), HWND_TOPMOST,   0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			SetWindowPos(GetHwnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			Show(opts.opt_bImmDialogFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE);
		}
		break;
	case VOICE_STATE_CALLING:
		incoming = false;
		break;
	case VOICE_STATE_ENDED:
		m_calltimer.Stop();
		wchar_t text[100];
		mir_snwprintf(text, TranslateT("Call is over %s"), m_nsec ? m_lblStatus.GetText() : L"");
		m_lblStatus.SetText(text);
		break;
	case VOICE_STATE_READY:
		Show();
		break;
	}
	
	if(stateTexts[state].status)
		m_lblStatus.SetText(stateTexts[state].status);
	if(stateTexts[state].btn1text)
		m_btnAnswer.SetText(stateTexts[state].btn1text);
	m_btnDrop.SetText(stateTexts[state].btn2text);

	m_btnAnswer.Enable(state == VOICE_STATE_TALKING ? CanHold() : (bool)stateTexts[state].btn1text);

	if (state != VOICE_STATE_ON_HOLD)
		m_nsec = 0;

	if (IsFinished()) {
		// Remove id because providers can re-use them
		mir_free(id);
		id = NULL;
	}

	Notify();
}

void VoiceCall::SetStatus(const wchar_t *text)
{
	m_lblStatus.SetText(text);
}

void VoiceCall::Notify(bool popup, bool sound, bool clist)
{
	if (sound) {
		soundActive = true;
		Skin_PlaySound(g_sounds[state].szName, SPS_LOOP);
	}

	if (IsWindowVisible(GetHwnd()))
		return;

	if (popup)
		ShowPopup(hContact, displayName, TranslateW(popupTexts[state]));

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
	return state == -1 || state == VOICE_STATE_RINGING || state == VOICE_STATE_ON_HOLD || state == VOICE_STATE_READY;
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
