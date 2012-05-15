#if !defined(AFX_MRA_SENDQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_SENDQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



DWORD	MraSendQueueInitialize(DWORD dwSendTimeOutInterval,HANDLE *phSendQueueHandle);
void	MraSendQueueDestroy(HANDLE hSendQueueHandle);

DWORD	MraSendQueueAdd(HANDLE hSendQueueHandle,DWORD dwCMDNum,DWORD dwFlags,HANDLE hContact,DWORD dwAckType,LPBYTE lpbData,SIZE_T dwDataSize);
DWORD	MraSendQueueFree(HANDLE hSendQueueHandle,DWORD dwCMDNum);

DWORD	MraSendQueueFind(HANDLE hSendQueueHandle,DWORD dwCMDNum,DWORD *pdwFlags,HANDLE *phContact,DWORD *pdwAckType,LPBYTE *plpbData,SIZE_T *pdwDataSize);
DWORD	MraSendQueueFindOlderThan(HANDLE hSendQueueHandle,DWORD dwTime,DWORD *pdwCMDNum,DWORD *pdwFlags,HANDLE *phContact,DWORD *pdwAckType,LPBYTE *plpbData,SIZE_T *pdwDataSize);




#endif // !defined(AFX_MRA_SENDQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
