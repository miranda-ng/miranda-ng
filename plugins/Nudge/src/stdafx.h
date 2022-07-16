// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <windows.h>
#include <commctrl.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_langpack.h>	
#include <m_popup.h>	
#include <m_message.h>
#include <m_ignore.h>
#include <m_hotkeys.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_icolib.h>	

#include <m_metacontacts.h>
#include <m_nudge.h>

#include "resource.h"
#include "shake.h"
#include "version.h"
#include "nudge.h"

/*
*
****************************/
int NudgeOptInit(WPARAM wParam,LPARAM lParam);

/*
*
****************************/
int Preview();

/*
*
****************************/
void Nudge_ShowPopup(CNudgeElement*, MCONTACT, wchar_t *);

/*
*
****************************/
void Nudge_ShowStatus(CNudgeElement*, MCONTACT, uint32_t timestamp);

/*
*
****************************/
void Nudge_SentStatus(CNudgeElement*, MCONTACT);

/*
*
****************************/
void Nudge_AddAccount(PROTOACCOUNT *proto);

/*
*
****************************/
void LoadPopupClass();

/*
*
****************************/
void AutoResendNudge(void *wParam) ;

///////////////////////////////////////////////////////////////////////////////
// external variables

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern int nProtocol;
extern CShake shake;
extern CNudge GlobalNudge;

extern CNudgeElement DefaultNudge;
extern OBJLIST<CNudgeElement> arNudges;