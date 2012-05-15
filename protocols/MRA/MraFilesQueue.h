#if !defined(AFX_MRA_FILESQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_FILESQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


INT_PTR CALLBACK MraFilesQueueDlgProcOpts(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
DWORD	MraFilesQueueInitialize			(DWORD dwFilesTimeOutInterval,HANDLE *phFilesQueueHandle);
void	MraFilesQueueDestroy			(HANDLE hFilesQueueHandle);

HANDLE	MraFilesQueueItemProxyByID		(HANDLE hFilesQueueHandle,DWORD dwIDRequest);
DWORD	MraFilesQueueAddReceive			(HANDLE hFilesQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD dwIDRequest,LPWSTR lpwszFiles,SIZE_T dwFilesSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize);
DWORD	MraFilesQueueAddSend			(HANDLE hFilesQueueHandle,DWORD dwFlags,HANDLE hContact,LPWSTR *plpwszFiles,SIZE_T dwFilesCount,DWORD *pdwIDRequest);
DWORD	MraFilesQueueAccept				(HANDLE hFilesQueueHandle,DWORD dwIDRequest,LPWSTR lpwszPath,SIZE_T dwPathSize);
DWORD	MraFilesQueueCancel				(HANDLE hFilesQueueHandle,DWORD dwIDRequest,BOOL bSendDecline);
DWORD	MraFilesQueueStartMrimProxy		(HANDLE hFilesQueueHandle,DWORD dwIDRequest);
DWORD	MraFilesQueueFree				(HANDLE hFilesQueueHandle,DWORD dwCMDNum);
DWORD	MraFilesQueueSendMirror			(HANDLE hFilesQueueHandle,DWORD dwIDRequest,LPSTR lpszAddreses,SIZE_T dwAddresesSize);




#endif // !defined(AFX_MRA_FILESQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
