#include "nudge.h"

/*
*
****************************/
void Nudge_ShowPopup(CNudgeElement, HANDLE, TCHAR *);

/*
*
****************************/
void Nudge_ShowStatus(CNudgeElement, HANDLE, DWORD timestamp);

/*
*
****************************/
void Nudge_SentStatus(CNudgeElement, HANDLE);

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
