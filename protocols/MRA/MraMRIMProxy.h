#if !defined(AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000





HANDLE	MraMrimProxyCreate			();
DWORD	MraMrimProxySetData			(HANDLE hMraMrimProxyData,LPSTR lpszEMail,SIZE_T dwEMailSize,DWORD dwIDRequest,DWORD dwDataType,LPSTR lpszUserData,SIZE_T dwUserDataSize,LPSTR lpszAddreses,SIZE_T dwAddresesSize,MRA_GUID *pmguidSessionID);
void	MraMrimProxyFree			(HANDLE hMraMrimProxyData);
void	MraMrimProxyCloseConnection	(HANDLE hMraMrimProxyData);
DWORD	MraMrimProxyConnect			(HANDLE hMraMrimProxyData,HANDLE *phConnection);



#endif // !defined(AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
