#if !defined(AFX_MRA_FILESQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_FILESQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_

#pragma once

INT_PTR CALLBACK MraFilesQueueDlgProcOpts(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

DWORD	 MraFilesQueueInitialize(DWORD dwFilesTimeOutInterval, HANDLE *phFilesQueueHandle);
void	 MraFilesQueueDestroy(HANDLE hFilesQueueHandle);

void   MraFilesQueueConnectionReceived(HANDLE hNewConnection, DWORD dwRemoteIP, void *pExtra);

HANDLE MraFilesQueueItemProxyByID(HANDLE hFilesQueueHandle, DWORD dwIDRequest);
DWORD  MraFilesQueueStartMrimProxy(HANDLE hFilesQueueHandle, DWORD dwIDRequest);
DWORD  MraFilesQueueFree(HANDLE hFilesQueueHandle, DWORD dwCMDNum);

#endif // !defined(AFX_MRA_FILESQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
