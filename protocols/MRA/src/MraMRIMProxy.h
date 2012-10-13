#if !defined(AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

DWORD	 MraMrimProxySetData(HANDLE hMraMrimProxyData, LPSTR lpszEMail, size_t dwEMailSize, DWORD dwIDRequest, DWORD dwDataType, LPSTR lpszUserData, size_t dwUserDataSize, LPSTR lpszAddreses, size_t dwAddresesSize, MRA_GUID *pmguidSessionID);

HANDLE MraMrimProxyCreate();
void	 MraMrimProxyFree(HANDLE hMraMrimProxyData);
void	 MraMrimProxyCloseConnection(HANDLE hMraMrimProxyData);

#endif // !defined(AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
