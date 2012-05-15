#if !defined(AFX_MRA_MPOP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_MPOP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




DWORD			MraMPopSessionQueueInitialize		(HANDLE *phMPopSessionQueue);
void			MraMPopSessionQueueDestroy			(HANDLE hMPopSessionQueue);
void			MraMPopSessionQueueFlush			(HANDLE hMPopSessionQueue);
DWORD			MraMPopSessionQueueAddUrl			(HANDLE hMPopSessionQueue,LPSTR lpszUrl,SIZE_T dwUrlSize);
DWORD			MraMPopSessionQueueAddUrlAndEMail	(HANDLE hMPopSessionQueue,LPSTR lpszUrl,SIZE_T dwUrlSize,LPSTR lpszEMail,SIZE_T dwEMailSize);
DWORD			MraMPopSessionQueueStart			(HANDLE hMPopSessionQueue);
DWORD			MraMPopSessionQueueSetNewMPopKey	(HANDLE hMPopSessionQueue,LPSTR lpszKey,SIZE_T dwKeySize);







#endif // !defined(AFX_MRA_MPOP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
