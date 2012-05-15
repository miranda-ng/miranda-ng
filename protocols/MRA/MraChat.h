#if !defined(AFX_MRA_CHAT_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_MRA_CHAT_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



void		MraChatDllError					();
BOOL		MraChatRegister					();
INT_PTR		MraChatSessionNew				(HANDLE hContactChatSession);
void		MraChatSessionDestroy			(HANDLE hContactChatSession);
INT_PTR		MraChatSessionEventSendByHandle	(HANDLE hContactChatSession,DWORD dwType,DWORD dwFlags,LPSTR lpszUID,SIZE_T dwUIDSize,LPWSTR lpwszStatus,LPWSTR lpwszMessage,DWORD_PTR dwItemData,DWORD dwTime);
INT_PTR		MraChatSessionInvite			(HANDLE hContactChatSession,LPSTR lpszEMailInMultiChat,SIZE_T dwEMailInMultiChatSize,DWORD dwTime);
INT_PTR		MraChatSessionMembersAdd		(HANDLE hContactChatSession,LPSTR lpszEMailInMultiChat,SIZE_T dwEMailInMultiChatSize,DWORD dwTime);
INT_PTR		MraChatSessionJoinUser			(HANDLE hContactChatSession,LPSTR lpszEMailInMultiChat,SIZE_T dwEMailInMultiChatSize,DWORD dwTime);
INT_PTR		MraChatSessionLeftUser			(HANDLE hContactChatSession,LPSTR lpszEMailInMultiChat,SIZE_T dwEMailInMultiChatSize,DWORD dwTime);
INT_PTR		MraChatSessionSetIviter			(HANDLE hContactChatSession,LPSTR lpszEMailInMultiChat,SIZE_T dwEMailInMultiChatSize);
INT_PTR		MraChatSessionSetOwner			(HANDLE hContactChatSession,LPSTR lpszEMailInMultiChat,SIZE_T dwEMailInMultiChatSize);
INT_PTR		MraChatSessionMessageAdd		(HANDLE hContactChatSession,LPSTR lpszEMailInMultiChat,SIZE_T dwEMailInMultiChatSize,LPWSTR lpwszMessage,SIZE_T dwMessageSize,DWORD dwTime);







#endif // !defined(AFX_MRA_CHAT_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
