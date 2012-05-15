#if !defined(AFX_MRA_ANTISPAM_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_ANTISPAM_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




SIZE_T MraAntiSpamLoadBadWordsW();
void MraAntiSpamFreeBadWords();

INT_PTR CALLBACK MraAntiSpamDlgProcOpts(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam);

void MraAntiSpamResetBadWordsList();

#define	MESSAGE_NOT_SPAM	1
#define	MESSAGE_SPAM		2
DWORD MraAntiSpamReceivedMessageW(LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwMessageFlags,LPWSTR lpwszMessage,SIZE_T dwMessageSize);


#endif // !defined(AFX_MRA_ANTISPAM_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
