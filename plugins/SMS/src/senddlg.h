#if !defined(AFX_SMS_SENDDLG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_SMS_SENDDLG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

DWORD  SendSMSWindowInitialize();
void   SendSMSWindowDestroy();

//Decleration of SMS send window list
INT_PTR CALLBACK	SMSAcceptedDlgProc(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK	SMSTimedOutDlgProc(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam);

HTREEITEM SendSMSWindowHItemSendGet(HWND hWndDlg);
HTREEITEM SendSMSWindowNextHItemGet(HWND hWndDlg,HTREEITEM hItem);

HWND   SendSMSWindowAdd(HANDLE hContact);
void   SendSMSWindowRemove(HWND hWndDlg);
HANDLE SendSMSWindowHContactGet(HWND hWndDlg);
void   SendSMSWindowHContactSet(HWND hWndDlg,HANDLE hContact);
HWND   SendSMSWindowHwndByHProcessGet(HANDLE hProcess);
void   SendSMSWindowHProcessSet(HWND hWndDlg,HANDLE hProcess);
BOOL   SendSMSWindowMultipleGet(HWND hWndDlg);
void   SendSMSWindowMultipleSet(HWND hWndDlg,BOOL bMultiple);
void   SendSMSWindowNumberSet(HWND hWndDlg,LPWSTR lpwszPhone,SIZE_T dwPhoneSize);
void   SendSMSWindowAsSentSet(HWND hWndDlg);
void   SendSMSWindowDbeiSet(HWND hWndDlg,DBEVENTINFO *pdbei);
void   SendSMSWindowDBAdd(HWND hWndDlg);
void   SendSMSWindowHItemSendSet(HWND hWndDlg,HTREEITEM hItemSend);
HWND   SendSMSWindowIsOtherInstanceHContact(HANDLE hContact);
void   SendSMSWindowNext(HWND hWndDlg);

void   SendSMSWindowSMSContactAdd(HWND hWndDlg,HANDLE hContact);
HANDLE SendSMSWindowSMSContactGet(HWND hWndDlg,SIZE_T iNum);
void   SendSMSWindowSMSContactsRemove(HWND hWndDlg);
void   SendSMSWindowUpdateAccountList(HWND hWndDlg);
void   SendSMSWindowsUpdateAllAccountLists();

#endif // !defined(AFX_SMS_SENDDLG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
