#if !defined(AFX_SMS_RECVDLG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_SMS_RECVDLG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_

DWORD	RecvSMSWindowInitialize		();
void	RecvSMSWindowDestroy		();
HWND	RecvSMSWindowAdd			(MCONTACT hContact,DWORD dwEventType,LPWSTR lpwszPhone,size_t dwPhoneSize,LPSTR lpszMessage,size_t dwMessageSize);
void	RecvSMSWindowRemove			(HWND hWndDlg);



#endif // !defined(AFX_SMS_RECVDLG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
