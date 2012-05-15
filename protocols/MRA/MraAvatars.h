#if !defined(AFX_MRA_AVATARS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_AVATARS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000





DWORD			MraAvatarsQueueInitialize			(HANDLE *phAvatarsQueueHandle);
void			MraAvatarsQueueDestroy				(HANDLE hAvatarsQueueHandle);


#define PA_FORMAT_DEFAULT	255 // return file name of def avatar
#define GetContactAvatarFormat(hContact,dwDefaultFormat)	DB_Mra_GetByte(hContact,"AvatarType",dwDefaultFormat)
#define SetContactAvatarFormat(hContact,dwFormat)			DB_Mra_SetByte(hContact,"AvatarType",(BYTE)dwFormat)

DWORD			MraAvatarsGetFileName				(HANDLE hAvatarsQueueHandle,HANDLE hContact,DWORD dwFormat,LPSTR lpszPath,SIZE_T dwPathSize,SIZE_T *pdwPathSizeRet);

// mir flag: GAIF_FORCE
DWORD			MraAvatarsQueueGetAvatar			(HANDLE hAvatarsQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD *pdwAvatarsQueueID,DWORD *pdwFormat,LPSTR lpszPath);
DWORD			MraAvatarsQueueGetAvatarSimple		(HANDLE hAvatarsQueueHandle,DWORD dwFlags,HANDLE hContact,DWORD dwSourceID);


INT_PTR CALLBACK MraAvatarsQueueDlgProcOpts			(HWND hWndDlg,UINT msg,WPARAM wParam,LPARAM lParam);

DWORD			MraAvatarsDeleteContactAvatarFile	(HANDLE hAvatarsQueueHandle,HANDLE hContact);




#endif // !defined(AFX_MRA_AVATARS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
