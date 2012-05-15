#if !defined(AFX_MRA_POPUP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_POPUP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define	MRA_POPUP_ALLOW_MSGBOX	1
#define	MRA_POPUP_ALLOW_ENTER	2

#define	MRA_POPUP_TYPE_NONE			0	// proto icon
#define	MRA_POPUP_TYPE_DEBUG		1	// IDI_APPLICATION
#define	MRA_POPUP_TYPE_INFORMATION	2	// IDI_INFORMATION
#define	MRA_POPUP_TYPE_QUESTION		3	// IDI_QUESTION
#define	MRA_POPUP_TYPE_WARNING		4	// IDI_WARNING
#define	MRA_POPUP_TYPE_ERROR		5	// IDI_ERROR
#define	MRA_POPUP_TYPE_EMAIL_STATUS	6	// 


int		MraPopupOptInit					(WPARAM wParam,LPARAM lParam);

void	MraPopupShowFromContactW		(HANDLE hContact,DWORD dwType,DWORD dwFlags,LPWSTR lpszMessage);
#define MraPopupShowFromAgentW(dwType,dwFlags,lpszMessage) MraPopupShowFromContactW(NULL,dwType,dwFlags,lpszMessage)

void	MraPopupShowW					(HANDLE hContact,DWORD dwType,DWORD dwFlags,LPWSTR lpszTitle,LPWSTR lpszMessage);





#endif // !defined(AFX_MRA_POPUP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
