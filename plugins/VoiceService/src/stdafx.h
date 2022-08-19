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


#ifndef __COMMONS_H__
# define __COMMONS_H__


#include <windows.h>
#include <tchar.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <commctrl.h>


// Disable "...truncated to '255' characters in the debug information" warnings
#pragma warning(disable: 4786)

#include <vector>
using namespace std;

// Miranda headers
#include <newpluginapi.h>
#include <m_system.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_clistint.h>
#include <m_clc.h>
#include <m_contacts.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_utils.h>
#include <m_button.h>
#include <m_popup.h>
#include <m_cluiframes.h>
#include <m_icolib.h>
#include <m_metacontacts.h>
#include <m_fontservice.h>
#include <m_skin.h>
#include <m_avatars.h>

#include <m_voice.h>
#include <m_voiceservice.h>

#include "resource.h"
#include "version.h"
#include "options.h"
#include "popup.h"

/////////////////////////////////////////////////////////////////////////////////////////

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

#define MODULE_NAME "VoiceService"

#define ACTION_CALL 0
#define ACTION_ANSWER 1
#define ACTION_HOLD 2
#define ACTION_DROP 3

#define NUM_STATES 6

#define AUTO_NOTHING 0
#define AUTO_ACCEPT 1
#define AUTO_DROP 2

class VoiceProvider
{
public:
	wchar_t description[256];
	char    name[256];
	HANDLE  hIcolib;
	int     flags;
	bool    is_protocol;

	VoiceProvider(const char *name, const wchar_t *description, int flags, HANDLE hIcolib);
	~VoiceProvider();

	bool  CanCall(const wchar_t *number);
	bool  CanCall(MCONTACT hContact, BOOL now = TRUE);
	void  Call(MCONTACT hContact, const wchar_t *number);

	bool  CanHold();

	bool  CanSendDTMF();

	HICON GetIcon();
	void  ReleaseIcon(HICON hIcon);

private:
	bool canHold;
	HANDLE state_hook;
};

class VoiceCall : CDlgBase
{
public:
	VoiceProvider *module;
	char *id;					// Protocol especific ID for this call
	MCONTACT hContact;
	wchar_t name[256];
	wchar_t number[256];
	wchar_t displayName[256];
	int state;
	DWORD end_time;
	bool incoming;
	bool secure;

	VoiceCall(VoiceProvider *module, const char *id);
	~VoiceCall();

	void AppendCallerID(MCONTACT hContact, const wchar_t *name, const wchar_t *number);

	void SetState(int state);

	void Drop();
	void Answer();
	void Hold();

	bool CanDrop();
	bool CanAnswer();
	bool CanHold();

	bool CanSendDTMF();
	void SendDTMF(wchar_t c);

	bool IsFinished();

	void Notify(bool popup = true, bool sound = true, bool clist = true);
	void SetNewCallHWND(HWND hwnd);

private:
	HWND hwnd;
	bool clistBlinking;

	void RemoveNotifications();
	void CreateDisplayName();
	void OnCommand_Answer(CCtrlButton*);
	void OnCommand_Drop(CCtrlButton*);
	void OnCallTimer(CTimer*);
	bool OnInitDialog() override;
	bool OnClose() override;
	
	CCtrlButton m_btnAnswer;
	CCtrlButton m_btnDrop;
	CCtrlLabel m_lblStatus, m_lblContactName, m_lblAddress;
	int m_nsec;
	CTimer m_calltimer;
	HFONT hContactNameFont;
};

extern OBJLIST<VoiceProvider> modules;
extern OBJLIST<VoiceCall> calls;

void Answer(VoiceCall *call);
bool CanCall(MCONTACT hContact, BOOL now = TRUE);
bool CanCall(const wchar_t *number);
bool CanCallNumber();
void HoldOtherCalls(VoiceCall *call);
VoiceCall * GetTalkingCall();
bool IsFinalState(int state);

INT_PTR VoiceRegister(WPARAM wParam, LPARAM lParam);
INT_PTR VoiceUnregister(WPARAM wParam, LPARAM lParam);

VoiceProvider *FindModule(const char *szModule);
VoiceCall *FindVoiceCall(const char *szModule, const char *id, bool add);
VoiceCall *FindVoiceCall(MCONTACT hContact);

__inline BOOL IsEmptyA(const char *str)
{
	return str == NULL || str[0] == 0;
}

__inline BOOL IsEmptyW(const WCHAR *str)
{
	return str == NULL || str[0] == 0;
}

#define ICON_SIZE 16

#define TIME_TO_SHOW_ENDED_CALL		5000 // ms

#define MS_VOICESERVICE_CLIST_DBLCLK "VoiceService/CList/RingingDblClk"

#define MS_VOICESERVICE_CM_CALL "VoiceService/ContactMenu/Call"
#define MS_VOICESERVICE_CM_ANSWER "VoiceService/ContactMenu/Answer"
#define MS_VOICESERVICE_CM_HOLD "VoiceService/ContactMenu/Hold"
#define MS_VOICESERVICE_CM_DROP "VoiceService/ContactMenu/Drop"

struct SoundDescr
{
	const char *szName;
	const wchar_t *wszDescr;
};

extern SoundDescr g_sounds[NUM_STATES];

#endif // __COMMONS_H__
