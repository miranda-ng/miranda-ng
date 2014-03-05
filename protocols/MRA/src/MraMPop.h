#if !defined(AFX_MRA_MPOP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_MPOP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#pragma once

DWORD			MraMPopSessionQueueInitialize		(HANDLE *phMPopSessionQueue);
void			MraMPopSessionQueueDestroy		(HANDLE hMPopSessionQueue);
DWORD			MraMPopSessionQueueSetNewMPopKey	(HANDLE hMPopSessionQueue, const CMStringA &szKey);

#endif // !defined(AFX_MRA_MPOP_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
