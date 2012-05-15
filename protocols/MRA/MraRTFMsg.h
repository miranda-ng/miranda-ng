#if !defined(AFX_MRA_RTFMSG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_RTFMSG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000





BOOL	MraIsMessageFlashAnimation	(LPWSTR lpwszMessage,SIZE_T dwMessageSize);
DWORD	MraConvertToRTFW			(LPWSTR lpwszMessage,SIZE_T dwMessageSize,LPSTR lpszMessageRTF,SIZE_T dwMessageRTFBuffSize,SIZE_T *pdwMessageRTFSize);




#endif // !defined(AFX_MRA_RTFMSG_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
