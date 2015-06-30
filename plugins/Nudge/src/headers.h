// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <commctrl.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clist.h>			
#include <m_langpack.h>	
#include <m_popup.h>	
#include <m_clui.h>
#include <m_message.h>
#include <m_ignore.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_icolib.h>	
#include <win2k.h>

#include <m_metacontacts.h>
#include <m_nudge.h>
#include <m_msg_buttonsbar.h>

#include "resource.h"
#include "shake.h"
#include "Version.h"
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
void Nudge_ShowPopup(CNudgeElement*, MCONTACT, TCHAR *);

/*
*
****************************/
void Nudge_ShowStatus(CNudgeElement*, MCONTACT, DWORD timestamp);

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

extern HINSTANCE hInst;
extern int nProtocol;
extern CShake shake;
extern CNudge GlobalNudge;

extern CNudgeElement DefaultNudge;
extern OBJLIST<CNudgeElement> arNudges;