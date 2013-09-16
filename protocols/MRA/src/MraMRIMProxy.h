#if !defined(AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#pragma once

DWORD	 MraMrimProxySetData(HANDLE hMraMrimProxyData, const CMStringA &szEmail, DWORD dwIDRequest, DWORD dwDataType, const CMStringA &lpszUserData, const CMStringA &szAddresses, MRA_GUID *pmguidSessionID);

HANDLE MraMrimProxyCreate();
void	 MraMrimProxyFree(HANDLE hMraMrimProxyData);
void	 MraMrimProxyCloseConnection(HANDLE hMraMrimProxyData);

#endif // !defined(AFX_MRA_MRIMPROXY_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
