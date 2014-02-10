#if !defined(AFX_MRA_SENDQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_SENDQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#pragma once

DWORD	MraSendQueueInitialize(DWORD dwSendTimeOutInterval, HANDLE *phSendQueueHandle);
void	MraSendQueueDestroy(HANDLE hSendQueueHandle);

DWORD	MraSendQueueAdd(HANDLE hSendQueueHandle, DWORD dwCMDNum, DWORD dwFlags, MCONTACT hContact, DWORD dwAckType, LPBYTE lpbData, size_t dwDataSize);
DWORD	MraSendQueueFree(HANDLE hSendQueueHandle, DWORD dwCMDNum);

DWORD	MraSendQueueFind(HANDLE hSendQueueHandle, DWORD dwCMDNum, DWORD *pdwFlags, MCONTACT *phContact, DWORD *pdwAckType, LPBYTE *plpbData, size_t *pdwDataSize);
DWORD	MraSendQueueFindOlderThan(HANDLE hSendQueueHandle, DWORD dwTime, DWORD *pdwCMDNum, DWORD *pdwFlags, MCONTACT *phContact, DWORD *pdwAckType, LPBYTE *plpbData, size_t *pdwDataSize);

#endif // !defined(AFX_MRA_SENDQUEUE_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
