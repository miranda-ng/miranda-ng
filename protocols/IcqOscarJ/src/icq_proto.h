// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera, George Hazan
// Copyright © 2012-2014 Miranda NG Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Protocol Interface declarations
// -----------------------------------------------------------------------------

#ifndef _ICQ_PROTO_H_
#define _ICQ_PROTO_H_

#include "m_system_cpp.h"
#include "m_protoint.h"

#define LISTSIZE 100

#define XSTATUS_COUNT 86

// for InfoUpdate
struct userinfo
{
	DWORD dwUin;
	MCONTACT hContact;
	time_t queued;
};

struct CIcqProto : public PROTO<CIcqProto>
{
				CIcqProto(const char*, const TCHAR*);
				~CIcqProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	MCONTACT  __cdecl AddToList( int flags, PROTOSEARCHRESULT *psr);
	virtual	MCONTACT  __cdecl AddToListByEvent( int flags, int iContact, HANDLE hDbEvent);

	virtual	int       __cdecl Authorize(HANDLE hDbEvent);
	virtual	int       __cdecl AuthDeny(HANDLE hDbEvent, const TCHAR *szReason);
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR *szMessage);

	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szReason);
	virtual	int       __cdecl FileResume( HANDLE hTransfer, int *action, const TCHAR **szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR *id);
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR *email);
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR *nick, const PROTOCHAR *firstName, const PROTOCHAR *lastName);
	virtual	HWND      __cdecl SearchAdvanced(HWND owner);
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int       __cdecl RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvFile(MCONTACT hContact, PROTORECVFILET*);
	virtual	int       __cdecl RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl RecvUrl(MCONTACT hContact, PROTORECVEVENT*);

	virtual	int       __cdecl SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE    __cdecl SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char *msg);
	virtual	int       __cdecl SendUrl(MCONTACT hContact, int flags, const char *url);

	virtual	int       __cdecl SetApparentMode(MCONTACT hContact, int mode);
	virtual	int       __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE    __cdecl GetAwayMsg(MCONTACT hContact);
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT *evt);
	virtual	int       __cdecl SetAwayMsg(int m_iStatus, const TCHAR *msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	//====| Services |====================================================================
	INT_PTR  __cdecl AddServerContact(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetInfoSetting(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl ChangeInfoEx(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetAvatarCaps(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetAvatarInfo(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetMyAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetMyAwayMsg(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetXStatusEx(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GetXStatusIcon(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl GrantAuthorization(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl menuXStatus(WPARAM wParam,LPARAM lParam,LPARAM fParam);
	INT_PTR  __cdecl OpenWebProfile(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl RequestAdvStatusIconIdx(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl RequestAuthorization(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl RequestXStatusDetails(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl RevokeAuthorization(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SendSms(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SendYouWereAdded(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SetMyAvatar(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SetNickName(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl SetXStatusEx(WPARAM wParam, LPARAM lParam);
	INT_PTR  __cdecl ShowXStatusDetails(WPARAM wParam, LPARAM lParam);

	INT_PTR  __cdecl OnCreateAccMgrUI(WPARAM, LPARAM);

    //====| Events |======================================================================
	void __cdecl OnAddContactForever(DBCONTACTWRITESETTING *cws, MCONTACT hContact);
	int  __cdecl OnIdleChanged(WPARAM, LPARAM);
	int  __cdecl OnModernOptInit(WPARAM, LPARAM);
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnPreShutdown(WPARAM, LPARAM);
	int  __cdecl OnPreBuildContactMenu(WPARAM, LPARAM);
	int  __cdecl OnMsgUserTyping(WPARAM, LPARAM);
	int  __cdecl OnProcessSrmmIconClick(WPARAM, LPARAM);
	int  __cdecl OnProcessSrmmEvent(WPARAM, LPARAM);
	int  __cdecl OnReloadIcons(WPARAM, LPARAM);
	void __cdecl OnRenameContact(DBCONTACTWRITESETTING *cws, MCONTACT hContact);
	void __cdecl OnRenameGroup( DBCONTACTWRITESETTING *cws, MCONTACT hContact);
	int  __cdecl OnUserInfoInit(WPARAM, LPARAM);

	int  __cdecl OnPreBuildStatusMenu(WPARAM, LPARAM);

	//====| Data |========================================================================
	HANDLE m_hDirectNetlibUser;

	BYTE m_bGatewayMode;
	BYTE m_bSecureLogin;
	BYTE m_bSecureConnection;
	BYTE m_bLegacyFix;
	BYTE m_bAimEnabled;
	BYTE m_bUtfEnabled;
	WORD m_wAnsiCodepage;
	BYTE m_bDCMsgEnabled;
	BYTE m_bTempVisListEnabled;
	BYTE m_bSsiEnabled;
	BYTE m_bSsiSimpleGroups;
	BYTE m_bAvatarsEnabled;
	BYTE m_bXStatusEnabled;
	BYTE m_bMoodsEnabled;

	icq_critical_section *localSeqMutex;
	icq_critical_section *connectionHandleMutex;

	int   m_bIdleAllow;
	DWORD m_dwLocalUIN;
	BYTE m_bConnectionLost;

	char m_szPassword[PASSWORDMAXLEN];
	BYTE m_bRememberPwd;

	int cheekySearchId;
	DWORD cheekySearchUin;
	char* cheekySearchUid;

	/*******************************************************************
	* Function declarations
	*******************************************************************/

	//----| capabilities.cpp |------------------------------------------------------------
	// Deletes all oscar capabilities for a given contact.
	void ClearAllContactCapabilities(MCONTACT hContact);

	// Deletes one or many oscar capabilities for a given contact.
	void ClearContactCapabilities(MCONTACT hContact, DWORD fdwCapabilities);

	// Sets one or many oscar capabilities for a given contact.
	void SetContactCapabilities(MCONTACT hContact, DWORD fdwCapabilities);

	// Returns true if the given contact supports the requested capabilites.
	BOOL CheckContactCapabilities(MCONTACT hContact, DWORD fdwCapabilities);

	// Scans a binary buffer for oscar capabilities and adds them to the contact.
	void AddCapabilitiesFromBuffer(MCONTACT hContact, BYTE *pBuffer, int nLength);

	// Scans a binary buffer for oscar capabilities and sets them to the contact.
	void SetCapabilitiesFromBuffer(MCONTACT hContact, BYTE *pBuffer, int nLength, BOOL bReset);

	//----| chan_01login.cpp |------------------------------------------------------------
	void   handleLoginChannel(BYTE *buf, size_t datalen, serverthread_info *info);

	//----| chan_02data.cpp |-------------------------------------------------------------
	void   handleDataChannel(BYTE *buf, size_t wLen, serverthread_info *info);

	void   LogFamilyError(WORD wFamily, WORD wError);

	//----| chan_03error.cpp |------------------------------------------------------------
	void   handleErrorChannel(unsigned char *buf, size_t datalen);

	//----| chan_04close.cpp |------------------------------------------------------------
	void   handleCloseChannel(BYTE *buf, size_t datalen, serverthread_info *info);
	void   handleLoginReply(BYTE *buf, size_t datalen, serverthread_info *info);
	void   handleMigration(serverthread_info *info);
	void   handleSignonError(WORD wError);

	int    connectNewServer(serverthread_info *info);

	//----| chan_05ping.cpp |-------------------------------------------------------------
	void   handlePingChannel(BYTE *buf, size_t wLen);

	void   StartKeepAlive(serverthread_info *info);
	void   StopKeepAlive(serverthread_info *info);
	void   CheckKeepAlive(serverthread_info *info);

	//----| cookies.cpp |-----------------------------------------------------------------
	icq_critical_section *cookieMutex; // we want this in avatar thread, used as queue lock
	LIST<icq_cookie_info> cookies;
	WORD   wCookieSeq;

	DWORD  AllocateCookie(BYTE bType, WORD wIdent, MCONTACT hContact, void *pvExtra);
	void   FreeCookie(DWORD dwCookie);
	void   FreeCookieByData(BYTE bType, void *pvExtra);
	void   ReleaseCookie(DWORD dwCookie);
	DWORD  GenerateCookie(WORD wIdent);

	int    GetCookieType(DWORD dwCookie);

	int    FindCookie(DWORD wCookie, MCONTACT *phContact, void **ppvExtra);
	int    FindCookieByData(void *pvExtra, DWORD *pdwCookie, MCONTACT *phContact);
	int    FindCookieByType(BYTE bType, DWORD *pdwCookie, MCONTACT *phContact, void **ppvExtra);
	int    FindMessageCookie(DWORD dwMsgID1, DWORD dwMsgID2, DWORD *pdwCookie, MCONTACT *phContact, cookie_message_data **ppvExtra);

	void   InitMessageCookie(cookie_message_data *pCookie);
	cookie_message_data* CreateMessageCookie(WORD bMsgType, BYTE bAckType);
	cookie_message_data* CreateMessageCookieData(BYTE bMsgType, MCONTACT hContact, DWORD dwUin, int bUseSrvRelay);

	void   RemoveExpiredCookies(void);

	//----| directpackets.cpp |-----------------------------------------------------------
	void   icq_sendDirectMsgAck(directconnect* dc, WORD wCookie, BYTE bMsgType, BYTE bMsgFlags, char* szCap);
	DWORD  icq_sendGetAwayMsgDirect(MCONTACT hContact, int type);
	void   icq_sendAwayMsgReplyDirect(directconnect *dc, WORD wCookie, BYTE msgType, const char** szMsg);
	void   icq_sendFileAcceptDirect(MCONTACT hContact, filetransfer *ft);
	void   icq_sendFileDenyDirect(MCONTACT hContact, filetransfer *ft, const char *szReason);
	int    icq_sendFileSendDirectv7(filetransfer *ft, const char *pszFiles);
	int    icq_sendFileSendDirectv8(filetransfer *ft, const char *pszFiles);
	DWORD  icq_SendDirectMessage(MCONTACT hContact, const char *szMessage, size_t nBodyLength, WORD wPriority, cookie_message_data *pCookieData, char *szCap);
	void   icq_sendXtrazRequestDirect(MCONTACT hContact, DWORD dwCookie, char* szBody, size_t nBodyLen, WORD wType);
	void   icq_sendXtrazResponseDirect(MCONTACT hContact, WORD wCookie, char* szBody, size_t nBodyLen, WORD wType);

	//----| fam_01service.cpp |-----------------------------------------------------------
	void   handleServiceFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader, serverthread_info *info);
	char*  buildUinList(int subtype, size_t wMaxLen, MCONTACT *hContactResume);
	void   sendEntireListServ(WORD wFamily, WORD wSubtype, int listType);
	void   setUserInfo(void);
	void   handleServUINSettings(int nPort, serverthread_info *info);

	//----| fam_02location.cpp |----------------------------------------------------------
	void   handleLocationFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader);
	void   handleLocationUserInfoReply(BYTE* buf, size_t wLen, DWORD dwCookie);

	//----| fam_03buddy.cpp |-------------------------------------------------------------
	void   handleBuddyFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader, serverthread_info *info);
	void   handleReplyBuddy(BYTE *buf, size_t wPackLen);
	void   handleUserOffline(BYTE *buf, size_t wPackLen);
	void   handleUserOnline(BYTE *buf, size_t wPackLen, serverthread_info *info);
	void   parseStatusNote(DWORD dwUin, char *szUid, MCONTACT hContact, oscar_tlv_chain *pChain);
	void   handleNotifyRejected(BYTE *buf, size_t wPackLen);

	//----| fam_04message.cpp |-----------------------------------------------------------
	icq_mode_messages m_modeMsgs;
	icq_critical_section *m_modeMsgsMutex;
	HANDLE m_modeMsgsEvent;

	void   handleMsgFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader);

	void   handleReplyICBM(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleRecvServMsg(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleRecvServMsgType1(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, DWORD dwRef);
	void   handleRecvServMsgType2(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, DWORD dwRef);
	void   handleRecvServMsgType4(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, DWORD dwRef);
	void   handleRecvServMsgError(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleRecvMsgResponse(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleServerAck(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleStatusMsgReply(const char *szPrefix, MCONTACT hContact, DWORD dwUin, WORD wVersion, int bMsgType, WORD wCookie, const char *szMsg, int nMsgFlags);
	void   handleTypingNotification(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleMissedMsg(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleOffineMessagesReply(BYTE *buf, size_t wLen, WORD wFlags, DWORD dwRef);
	void   handleRecvServMsgContacts(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwID1, DWORD dwID2, WORD wCommand);
	void   handleRuntimeError(WORD wError);

	void   parseServRelayData(BYTE *pDataBuf, size_t wLen, MCONTACT hContact, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, WORD wAckType);
	void   parseServRelayPluginData(BYTE *pDataBuf, size_t wLen, MCONTACT hContact, DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, WORD wAckType, BYTE bFlags, WORD wStatus, WORD wCookie, WORD wVersion);

	HANDLE handleMessageAck(DWORD dwUin, char *szUID, WORD wCookie, WORD wVersion, int type, size_t wMsgLen, PBYTE buf, BYTE bFlags, int nMsgFlags);
	void   handleMessageTypes(DWORD dwUin, char *szUID, DWORD dwTimestamp, DWORD dwMsgID, DWORD dwMsgID2, WORD wCookie, WORD wVersion, int type, int flags, WORD wAckType, size_t dwDataLen, size_t wMsgLen, char *pMsg, int nMsgFlags, message_ack_params *pAckParams);
	void   sendMessageTypesAck(MCONTACT hContact, int bUnicode, message_ack_params *pArgs);
	void   sendTypingNotification(MCONTACT hContact, WORD wMTNCode);

	int    unpackPluginTypeId(BYTE **pBuffer, size_t *pwLen, int *pTypeId, WORD *pFunctionId, BOOL bThruDC);

	char*  convertMsgToUserSpecificUtf(MCONTACT hContact, const char *szMsg);

	//----| fam_09bos.cpp |---------------------------------------------------------------
	void   handleBosFam(unsigned char *pBuffer, size_t wBufferLength, snac_header* pSnacHeader);
	void   handlePrivacyRightsReply(unsigned char *pBuffer, size_t wBufferLength);
	void   makeContactTemporaryVisible(MCONTACT hContact);

	//----| fam_0alookup.cpp |------------------------------------------------------------
	void   handleLookupFam(unsigned char *pBuffer, size_t wBufferLength, snac_header* pSnacHeader);

	void   handleLookupEmailReply(BYTE* buf, size_t wLen, DWORD dwCookie);
	void   ReleaseLookupCookie(DWORD dwCookie, cookie_search *pCookie);

	//----| fam_0bstatus.cpp |------------------------------------------------------------
	void   handleStatusFam(unsigned char *pBuffer, size_t wBufferLength, snac_header* pSnacHeader);

	//----| fam_13servclist.cpp |---------------------------------------------------------
	BOOL   bIsSyncingCL;

	WORD   m_wServerListLimits[0x20];
	WORD   m_wServerListGroupMaxContacts;
	WORD   m_wServerListRecordNameMaxLength;

	void   handleServCListFam(BYTE *pBuffer, size_t wBufferLength, snac_header* pSnacHeader, serverthread_info *info);
	void   handleServerCListRightsReply(BYTE *buf, size_t wLen);
	void   handleServerCListAck(cookie_servlist_action* sc, WORD wError);
	void   handleServerCListReply(BYTE *buf, size_t wLen, WORD wFlags, serverthread_info *info);
	void   handleServerCListItemAdd(const char *szRecordName, WORD wGroupId, WORD wItemId, WORD wItemType, oscar_tlv_chain *pItemData);
	void   handleServerCListItemUpdate(const char *szRecordName, WORD wGroupId, WORD wItemId, WORD wItemType, oscar_tlv_chain *pItemData);
	void   handleServerCListItemDelete(const char *szRecordName, WORD wGroupId, WORD wItemId, WORD wItemType, oscar_tlv_chain *pItemData);
	void   handleRecvAuthRequest(BYTE *buf, size_t wLen);
	void   handleRecvAuthResponse(BYTE *buf, size_t wLen);
	void   handleRecvAdded(BYTE *buf, size_t wLen);

	MCONTACT HContactFromRecordName(const char *szRecordName, int *bAdded);

	void   processCListReply(const char *szRecordName, WORD wGroupId, WORD wItemId, WORD wItemType, oscar_tlv_chain *pItemData);

	void   icq_sendServerBeginOperation(int bImport);
	void   icq_sendServerEndOperation();
	void   sendRosterAck(void);

	int    getServerDataFromItemTLV(oscar_tlv_chain* pChain, unsigned char *buf);
	DWORD  updateServerGroupData(WORD wGroupId, void *groupData, int groupSize, DWORD dwOperationFlags);
	void   updateServAvatarHash(BYTE *pHash, int size);
	void   updateServVisibilityCode(BYTE bCode);

	//----| fam_15icqserver.cpp |---------------------------------------------------------
	void   handleIcqExtensionsFam(BYTE *pBuffer, size_t wBufferLength, snac_header* pSnacHeader);

	void   handleExtensionError(BYTE *buf, size_t wPackLen);
	void   handleExtensionServerInfo(BYTE *buf, size_t wPackLen, WORD wFlags);
	void   handleExtensionMetaResponse(BYTE *databuf, size_t wPacketLen, WORD wCookie, WORD wFlags);

	int    parseUserInfoRecord(MCONTACT hContact, oscar_tlv *pData, UserInfoRecordItem pRecordDef[], int nRecordDef, int nMaxRecords);

	void   handleDirectoryQueryResponse(BYTE *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype, WORD wFlags);
	void   handleDirectoryUpdateResponse(BYTE *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype);

	void   parseDirectoryUserDetailsData(MCONTACT hContact, oscar_tlv_chain *cDetails, DWORD dwCookie, cookie_directory_data *pCookieData, WORD wReplySubType);
	void   parseDirectorySearchData(oscar_tlv_chain *cDetails, DWORD dwCookie, cookie_directory_data *pCookieData, WORD wReplySubType);

	void   parseSearchReplies(unsigned char *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype, BYTE bResultCode);
	void   parseUserInfoUpdateAck(unsigned char *databuf, size_t wPacketLen, WORD wCookie, WORD wReplySubtype, BYTE bResultCode);

	void   ReleaseSearchCookie(DWORD dwCookie, cookie_search *pCookie);

	//----| fam_17signon.cpp |------------------------------------------------------------
	void   handleAuthorizationFam(BYTE *pBuffer, size_t wBufferLength, snac_header *pSnacHeader, serverthread_info *info);
	void   handleAuthKeyResponse(BYTE *buf, size_t wPacketLen, serverthread_info *info);

	void   sendClientAuth(const char *szKey, size_t wKeyLen, BOOL bSecure);

	//----| icq_avatars.cpp |-------------------------------------------------------------
	icq_critical_section *m_avatarsMutex;
	avatars_request *m_avatarsQueue;

	BOOL   m_avatarsConnectionPending;
	avatars_server_connection *m_avatarsConnection;

	void   requestAvatarConnection();
	void   __cdecl AvatarThread(avatars_server_connection *pInfo);

	void   handleAvatarOwnerHash(WORD wItemID, BYTE bFlags, BYTE *pData, BYTE nDataLen);
	void   handleAvatarContactHash(DWORD dwUIN, char *szUID, MCONTACT hContact, BYTE *pHash, int nHashLen, WORD wOldStatus);

	avatars_request *ReleaseAvatarRequestInQueue(avatars_request *request);

	TCHAR* GetOwnAvatarFileName();
	void   GetFullAvatarFileName(int dwUin, const char *szUid, int dwFormat, TCHAR *pszDest, int cbLen);
	void   GetAvatarFileName(int dwUin, const char *szUid, TCHAR *pszDest, int cbLen);
	int    IsAvatarChanged(MCONTACT hContact, const BYTE *pHash, int nHashLen);

	int    GetAvatarData(MCONTACT hContact, DWORD dwUin, const char *szUid, const BYTE *hash, unsigned int hashlen, const TCHAR *file);
	int    SetAvatarData(MCONTACT hContact, WORD wRef, const BYTE *data, unsigned int datalen);

	void   StartAvatarThread(HANDLE hConn, char* cookie, size_t cookieLen);
	void   StopAvatarThread();

	//----| icq_clients.cpp |-------------------------------------------------------------
	const char* detectUserClient(MCONTACT hContact, int nIsICQ, WORD wUserClass, DWORD dwOnlineSince, const char *szCurrentClient, WORD wVersion, DWORD dwFT1, DWORD dwFT2, DWORD dwFT3, BYTE bDirectFlag, DWORD dwDirectCookie, DWORD dwWebPort, BYTE *caps, size_t wLen, BYTE *bClientId, char *szClientBuf);

	//----| icq_db.cpp |------------------------------------------------------------------
	HANDLE AddEvent(MCONTACT hContact, WORD wType, DWORD dwTime, DWORD flags, size_t cbBlob, PBYTE pBlob);
	int    IsICQContact(MCONTACT hContact);

	int    getSetting(MCONTACT hContact, const char *szSetting, DBVARIANT *dbv);
	double getSettingDouble(MCONTACT hContact, const char *szSetting, double dDef);
	int    getSettingStringStatic(MCONTACT hContact, const char *szSetting, char *dest, int dest_len);
	char*  getSettingStringUtf(MCONTACT hContact, const char *szModule, const char *szSetting, char *szDef);
	char*  getSettingStringUtf(MCONTACT hContact, const char *szSetting, char *szDef);
	int    getContactUid(MCONTACT hContact, DWORD *pdwUin, uid_str *ppszUid);
	DWORD  getContactUin(MCONTACT hContact);
	WORD   getContactStatus(MCONTACT hContact);
	char*  getContactCListGroup(MCONTACT hContact);

	int    setSettingDouble(MCONTACT hContact, const char *szSetting, double dValue);
	int    setSettingBlob(MCONTACT hContact, const char *szSetting, const BYTE *pValue, const int cbValue);
	int    setContactHidden(MCONTACT hContact, BYTE bHidden);
	void   setStatusMsgVar(MCONTACT hContact, char* szStatusMsg, bool isAnsi);

	//----| icq_direct.cpp |--------------------------------------------------------------
	icq_critical_section *directConnListMutex;
	LIST<directconnect> directConns;

	icq_critical_section *expectedFileRecvMutex;
	LIST<filetransfer> expectedFileRecvs;

	void   __cdecl icq_directThread(struct directthreadstartinfo* dtsi);

	void   handleDirectPacket(directconnect* dc, PBYTE buf, size_t wLen);
	void   sendPeerInit_v78(directconnect* dc);
	void   sendPeerInitAck(directconnect* dc);
	void   sendPeerMsgInit(directconnect* dc, DWORD dwSeq);
	void   sendPeerFileInit(directconnect* dc);
	int    sendDirectPacket(directconnect* dc, icq_packet* pkt);

	void   CloseContactDirectConns(MCONTACT hContact);
	directconnect* FindFileTransferDC(filetransfer* ft);
	filetransfer*  FindExpectedFileRecv(DWORD dwUin, DWORD dwTotalSize);
	BOOL   IsDirectConnectionOpen(MCONTACT hContact, int type, int bPassive);
	void   OpenDirectConnection(MCONTACT hContact, int type, void* pvExtra);
	void   CloseDirectConnection(directconnect *dc);
	int    SendDirectMessage(MCONTACT hContact, icq_packet *pkt);

	//----| icq_directmsg.cpp |-----------------------------------------------------------
	void   handleDirectMessage(directconnect* dc, PBYTE buf, size_t wLen);
	void   handleDirectGreetingMessage(directconnect* dc, PBYTE buf, size_t wLen, WORD wCommand, WORD wCookie, BYTE bMsgType, BYTE bMsgFlags, WORD wStatus, WORD wFlags, char* pszText);

	//----| icq_filerequests.cpp |--------------------------------------------------------
	filetransfer* CreateFileTransfer(MCONTACT hContact, DWORD dwUin, int nVersion);

	void   handleFileAck(PBYTE buf, size_t wLen, DWORD dwUin, DWORD dwCookie, WORD wStatus, char* pszText);
	void   handleFileRequest(PBYTE buf, size_t wLen, DWORD dwUin, DWORD dwCookie, DWORD dwID1, DWORD dwID2, char* pszDescription, int nVersion, BOOL bDC);
	void   handleDirectCancel(directconnect *dc, PBYTE buf, size_t wLen, WORD wCommand, DWORD dwCookie, WORD wMessageType, WORD wStatus, WORD wFlags, char* pszText);

	void   icq_CancelFileTransfer(MCONTACT hContact, filetransfer* ft);

	//----| icq_filetransfer.cpp |--------------------------------------------------------
	void   icq_AcceptFileTransfer(MCONTACT hContact, filetransfer *ft);
	void   icq_sendFileResume(filetransfer *ft, int action, const char *szFilename);
	void   icq_InitFileSend(filetransfer *ft);

	void   handleFileTransferPacket(directconnect *dc, PBYTE buf, size_t wLen);
	void   handleFileTransferIdle(directconnect *dc);

	//----| icq_infoupdate.cpp |----------------------------------------------------------
	icq_critical_section *infoUpdateMutex;
	HANDLE hInfoQueueEvent;
	int    nInfoUserCount;
	int    bInfoPendingUsers;
	BOOL   bInfoUpdateEnabled;
	BOOL   bInfoUpdateRunning;
	HANDLE hInfoThread;
	DWORD  dwInfoActiveRequest;
	userinfo m_infoUpdateList[LISTSIZE];

	void   __cdecl InfoUpdateThread(void*);

	void   icq_InitInfoUpdate(void);           // Queues all outdated users
	BOOL   icq_QueueUser(MCONTACT hContact);     // Queue one UIN to the list for updating
	void   icq_DequeueUser(DWORD dwUin);       // Remove one UIN from the list
	void   icq_RescanInfoUpdate();             // Add all outdated contacts to the list
	void   icq_InfoUpdateCleanup(void);        // Clean up on exit
	void   icq_EnableUserLookup(BOOL bEnable); // Enable/disable user info lookups

	//----| log.cpp |-----------------------------------------------------------------
	BOOL   bErrorBoxVisible;

	void   __cdecl icq_LogMessageThread(void* arg);

	void   icq_LogMessage(int level, const char *szMsg);
	void   icq_LogUsingErrorCode(int level, DWORD dwError, const char *szMsg);  //szMsg is optional
	void   icq_LogFatalParam(const char *szMsg, WORD wError);

	//----| icq_packet.cpp |--------------------------------------------------------------
	void   ppackLETLVLNTSfromDB(PBYTE *buf, size_t *buflen, const char *szSetting, WORD wType);
	void   ppackLETLVWordLNTSfromDB(PBYTE *buf, size_t *buflen, WORD w, const char *szSetting, WORD wType);
	void   ppackLETLVLNTSBytefromDB(PBYTE *buf, size_t *buflen, const char *szSetting, BYTE b, WORD wType);

	void   ppackTLVStringFromDB(PBYTE *buf, size_t *buflen, const char *szSetting, WORD wType);
	void   ppackTLVStringUtfFromDB(PBYTE *buf, size_t *buflen, const char *szSetting, WORD wType);
	void   ppackTLVDateFromDB(PBYTE *buf, size_t *buflen, const char *szSettingYear, const char *szSettingMonth, const char *szSettingDay, WORD wType);

	int    ppackTLVWordStringItemFromDB(PBYTE *buf, size_t *buflen, const char *szSetting, WORD wTypeID, WORD wTypeData, WORD wID);
	int    ppackTLVWordStringUtfItemFromDB(PBYTE *buf, size_t *buflen, const char *szSetting, WORD wTypeID, WORD wTypeData, WORD wID);

	BOOL   unpackUID(BYTE **ppBuf, size_t *pwLen, DWORD *pdwUIN, uid_str *ppszUID);

	//----| icq_popups.cpp |--------------------------------------------------------------
	int    ShowPopupMsg(MCONTACT hContact, const char *szTitle, const char *szMsg, BYTE bType);

	//----| icq_proto.cpp |--------------------------------------------------------------
	void   __cdecl CheekySearchThread( void* );

	void   __cdecl GetAwayMsgThread( void *pStatusData );

	char*  PrepareStatusNote(int nStatus);

	//----| icq_rates.cpp |---------------------------------------------------------------
	icq_critical_section *m_ratesMutex;
	rates  *m_rates;

	rates_queue *m_ratesQueue_Request; // rate queue for xtraz requests
	rates_queue *m_ratesQueue_Response; // rate queue for msg responses

	int    handleRateItem(rates_queue_item *item, int nQueueType = RQT_DEFAULT, int nMinDelay = 0, BOOL bAllowDelay = TRUE);

	void   __cdecl rateDelayThread(struct rate_delay_args *pArgs);

	//----| icq_server.cpp |--------------------------------------------------------------
	HANDLE hServerConn;
	WORD   wListenPort;
	WORD   wLocalSequence;
	UINT   serverThreadId;
	HANDLE serverThreadHandle;

	__inline bool icqOnline() const
	{	return (m_iStatus != ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_CONNECTING);
	}

	void   __cdecl SendPacketAsyncThread(icq_packet* pArgs);
	void   __cdecl ServerThread(serverthread_start_info *infoParam);

	void   icq_serverDisconnect(void);
	void   icq_login(const char* szPassword);

	int    handleServerPackets(BYTE *buf, int len, serverthread_info *info);
	void   sendServPacket(icq_packet *pPacket);
	void   sendServPacketAsync(icq_packet *pPacket);

	int    IsServerOverRate(WORD wFamily, WORD wCommand, int nLevel);

	//----| icq_servlist.cpp |------------------------------------------------------------
	HANDLE hHookSettingChanged;
	HANDLE hHookContactDeleted;
	HANDLE hHookCListGroupChange;
	icq_critical_section *servlistMutex;

	DWORD* pdwServerIDList;
	int    nServerIDListCount;
	int    nServerIDListSize;

	// server-list update board
	icq_critical_section *servlistQueueMutex;
	int    servlistQueueCount;
	int    servlistQueueSize;
	ssiqueueditems **servlistQueueList;
	int    servlistQueueState;
	HANDLE servlistQueueThreadHandle;
	int    servlistEditCount;

	void   servlistBeginOperation(int operationCount, int bImport);
	void   servlistEndOperation(int operationCount);

	void   __cdecl servlistQueueThread(void* queueState);

	void   servlistQueueAddGroupItem(servlistgroupitem* pGroupItem, int dwTimeout);
	int    servlistHandlePrimitives(DWORD dwOperation);
	void   servlistProcessLogin();

	void   servlistPostPacket(icq_packet* packet, DWORD dwCookie, DWORD dwOperation, DWORD dwTimeout);
	void   servlistPostPacketDouble(icq_packet* packet1, DWORD dwCookie, DWORD dwOperation, DWORD dwTimeout, icq_packet* packet2, WORD wAction2);

	// server-list pending queue
	int    servlistPendingCount;
	int    servlistPendingSize;
	servlistpendingitem** servlistPendingList;

	int    servlistPendingFindItem(int nType, MCONTACT hContact, const char *pszGroup);
	void   servlistPendingAddItem(servlistpendingitem* pItem);
	servlistpendingitem* servlistPendingRemoveItem(int nType, MCONTACT hContact, const char *pszGroup);

	void   servlistPendingAddContactOperation(MCONTACT hContact, LPARAM param, PENDING_CONTACT_CALLBACK callback, DWORD flags);
	void   servlistPendingAddGroupOperation(const char *pszGroup, LPARAM param, PENDING_GROUP_CALLBACK callback, DWORD flags);
	int    servlistPendingAddContact(MCONTACT hContact, WORD wContactID, WORD wGroupID, LPARAM param, PENDING_CONTACT_CALLBACK callback, int bDoInline, LPARAM operationParam = 0, PENDING_CONTACT_CALLBACK operationCallback = NULL);
	int    servlistPendingAddGroup(const char *pszGroup, WORD wGroupID, LPARAM param, PENDING_GROUP_CALLBACK callback, int bDoInline, LPARAM operationParam = 0, PENDING_GROUP_CALLBACK operationCallback = NULL);
	void   servlistPendingRemoveContact(MCONTACT hContact, WORD wContactID, WORD wGroupID, int nResult);
	void   servlistPendingRemoveGroup(const char *pszGroup, WORD wGroupID, int nResult);
	void   servlistPendingFlushOperations();

	// server-list support functions
	int    nJustAddedCount;
	int    nJustAddedSize;
	MCONTACT* pdwJustAddedList;

	void   AddJustAddedContact(MCONTACT hContact);
	BOOL   IsContactJustAdded(MCONTACT hContact);
	void   FlushJustAddedContacts();

	WORD   GenerateServerID(int bGroupType, int bFlags, int wCount = 0);
	void   ReserveServerID(WORD wID, int bGroupType, int bFlags);
	void   FreeServerID(WORD wID, int bGroupType);
	BOOL   CheckServerID(WORD wID, unsigned int wCount);
	void   FlushServerIDs();
	void   LoadServerIDs();
	void   StoreServerIDs();

	void*  collectGroups(int *count);
	void*  collectBuddyGroup(WORD wGroupID, int *count);
	char*  getServListGroupName(WORD wGroupID);
	void   setServListGroupName(WORD wGroupID, const char *szGroupName);
	WORD   getServListGroupLinkID(const char *szPath);
	void   setServListGroupLinkID(const char *szPath, WORD wGroupID);
	int    IsServerGroupsDefined();
	char*  getServListGroupCListPath(WORD wGroupId);
	char*  getServListUniqueGroupName(const char *szGroupName, int bAlloced);

	int __cdecl servlistCreateGroup_gotParentGroup(const char *szGroup, WORD wGroupID, LPARAM param, int nResult);
	int __cdecl servlistCreateGroup_Ready(const char *szGroup, WORD groupID, LPARAM param, int nResult);
	void   servlistCreateGroup(const char *szGroupPath, LPARAM param, PENDING_GROUP_CALLBACK callback);

	int __cdecl servlistAddContact_gotGroup(const char *szGroup, WORD wGroupID, LPARAM lParam, int nResult);
	int __cdecl servlistAddContact_Ready(MCONTACT hContact, WORD wContactID, WORD wGroupID, LPARAM lParam, int nResult);
	void   servlistAddContact(MCONTACT hContact, const char *pszGroup);

	int __cdecl servlistRemoveContact_Ready(MCONTACT hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult);
	void   servlistRemoveContact(MCONTACT hContact);

	int __cdecl servlistMoveContact_gotTargetGroup(const char *szGroup, WORD wNewGroupID, LPARAM lParam, int nResult);
	int __cdecl servlistMoveContact_Ready(MCONTACT hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult);
	void   servlistMoveContact(MCONTACT hContact, const char *pszNewGroup);

	int __cdecl servlistUpdateContact_Ready(MCONTACT hContact, WORD contactID, WORD groupID, LPARAM lParam, int nResult);
	void   servlistUpdateContact(MCONTACT hContact);

	int __cdecl servlistRenameGroup_Ready(const char *szGroup, WORD wGroupID, LPARAM lParam, int nResult);
	void   servlistRenameGroup(char *szGroup, WORD wGroupId, char *szNewGroup);

	int __cdecl servlistRemoveGroup_Ready(const char *szGroup, WORD groupID, LPARAM lParam, int nResult);
	void   servlistRemoveGroup(const char *szGroup, WORD wGroupId);

	void   removeGroupPathLinks(WORD wGroupID);
	int    getServListGroupLevel(WORD wGroupId);

	void   resetServContactAuthState(MCONTACT hContact, DWORD dwUin);

	void   FlushSrvGroupsCache();
	int    getCListGroupExists(const char *szGroup);
	int    moveContactToCListGroup(MCONTACT hContact, const char *szGroup); /// TODO: this should be DB function

	DWORD  icq_sendServerItem(DWORD dwCookie, WORD wAction, WORD wGroupId, WORD wItemId, const char *szName, BYTE *pTLVs, size_t nTlvLength, WORD wItemType, DWORD dwOperation, DWORD dwTimeout, void **doubleObject);
	DWORD  icq_sendServerContact(MCONTACT hContact, DWORD dwCookie, WORD wAction, WORD wGroupId, WORD wContactId, DWORD dwOperation, DWORD dwTimeout, void **doubleObject);
	DWORD  icq_sendSimpleItem(DWORD dwCookie, WORD wAction, DWORD dwUin, char* szUID, WORD wGroupId, WORD wItemId, WORD wItemType, DWORD dwOperation, DWORD dwTimeout);
	DWORD  icq_sendServerGroup(DWORD dwCookie, WORD wAction, WORD wGroupId, const char *szName, void *pContent, int cbContent, DWORD dwOperationFlags);

	DWORD  icq_modifyServerPrivacyItem(MCONTACT hContact, DWORD dwUin, char *szUid, WORD wAction, DWORD dwOperation, WORD wItemId, WORD wType);
	DWORD  icq_removeServerPrivacyItem(MCONTACT hContact, DWORD dwUin, char *szUid, WORD wItemId, WORD wType);
	DWORD  icq_addServerPrivacyItem(MCONTACT hContact, DWORD dwUin, char *szUid, WORD wItemId, WORD wType);

	int __cdecl ServListDbSettingChanged(WPARAM wParam, LPARAM lParam);
	int __cdecl ServListDbContactDeleted(WPARAM wParam, LPARAM lParam);
	int __cdecl ServListCListGroupChange(WPARAM wParam, LPARAM lParam);

	//----| stdpackets.cpp |----------------------------------------------------------
	void   icq_sendCloseConnection();

	void   icq_requestnewfamily(WORD wFamily, void (CIcqProto::*familyhandler)(HANDLE hConn, char* cookie, size_t cookieLen));

	void   icq_setidle(int bAllow);
	void   icq_setstatus(WORD wStatus, const char *szStatusNote = NULL);
	DWORD  icq_sendGetInfoServ(MCONTACT hContact, DWORD, int);
	DWORD  icq_sendGetAimProfileServ(MCONTACT hContact, char *szUid);
	DWORD  icq_sendGetAwayMsgServ(MCONTACT hContact, DWORD, int, WORD);
	DWORD  icq_sendGetAwayMsgServExt(MCONTACT hContact, DWORD dwUin, char *szUID, int type, WORD wVersion);
	DWORD  icq_sendGetAimAwayMsgServ(MCONTACT hContact, char *szUID, int type);
	void   icq_sendSetAimAwayMsgServ(const char *szMsg);

	void   icq_sendFileSendServv7(filetransfer* ft, const char *szFiles);
	void   icq_sendFileSendServv8(filetransfer* ft, const char *szFiles, int nAckType);

	void   icq_sendFileAcceptServ(DWORD dwUin, filetransfer *ft, int nAckType);
	void   icq_sendFileAcceptServv7(DWORD dwUin, DWORD TS1, DWORD TS2, DWORD dwCookie, const char *szFiles, const char *szDescr, DWORD dwTotalSize, WORD wPort, BOOL accepted, int nAckType);
	void   icq_sendFileAcceptServv8(DWORD dwUin, DWORD TS1, DWORD TS2, DWORD dwCookie, const char *szFiles, const char *szDescr, DWORD dwTotalSize, WORD wPort, BOOL accepted, int nAckType);

	void   icq_sendFileDenyServ(DWORD dwUin, filetransfer *ft, const char *szReason, int nAckType);

	DWORD  icq_sendAdvancedSearchServ(BYTE *fieldsBuffer, size_t bufferLen);
	DWORD  icq_changeUserPasswordServ(const char *szPassword);
	DWORD  icq_changeUserDirectoryInfoServ(const BYTE *pData, size_t wDataLen, BYTE bRequestType);
	void   icq_sendGenericContact(DWORD dwUin, const char *szUid, WORD wFamily, WORD wSubType);
	void   icq_sendNewContact(DWORD dwUin, const char *szUid);
	void   icq_sendRemoveContact(DWORD dwUin, const char *szUid);
	void   icq_sendChangeVisInvis(MCONTACT hContact, DWORD dwUin, char* szUID, int list, int add);
	void   icq_sendEntireVisInvisList(int);
	void   icq_sendAwayMsgReplyServ(DWORD, DWORD, DWORD, WORD, WORD, BYTE, char **);
	void   icq_sendAwayMsgReplyServExt(DWORD dwUin, char *szUID, DWORD dwMsgID1, DWORD dwMsgID2, WORD wCookie, WORD wVersion, BYTE msgType, char **szMsg);

	DWORD  icq_sendSMSServ(const char *szPhoneNumber, const char *szMsg);
	void   icq_sendMessageCapsServ(DWORD dwUin);
	void   icq_sendRevokeAuthServ(DWORD dwUin, char *szUid);
	void   icq_sendGrantAuthServ(DWORD dwUin, const char *szUid, const char *szMsg);
	void   icq_sendAuthReqServ(DWORD dwUin, char* szUid, const char *szMsg);
	void   icq_sendAuthResponseServ(DWORD dwUin, char* szUid,int auth,const TCHAR *szReason);
	void   icq_sendYouWereAddedServ(DWORD,DWORD);

	DWORD  sendDirectorySearchPacket(const BYTE *pSearchData, size_t wDataLen, WORD wPage, BOOL bOnlineUsersOnly);
	DWORD  sendTLVSearchPacket(BYTE bType, char* pSearchDataBuf, WORD wSearchType, size_t wInfoLen, BOOL bOnlineUsersOnly);
	void   sendOwnerInfoRequest(void);
	DWORD  sendUserInfoMultiRequest(BYTE *pRequestData, size_t dataLen, int nItems);

	DWORD  icq_SendChannel1Message(DWORD dwUin, char *szUID, MCONTACT hContact, char *pszText, cookie_message_data *pCookieData);
	DWORD  icq_SendChannel1MessageW(DWORD dwUin, char *szUID, MCONTACT hContact, WCHAR *pszText, cookie_message_data *pCookieData); // UTF-16
	DWORD  icq_SendChannel2Message(DWORD dwUin, MCONTACT hContact, const char *szMessage, size_t nBodyLength, WORD wPriority, cookie_message_data *pCookieData, char *szCap);
	DWORD  icq_SendChannel2Contacts(DWORD dwUin, char *szUid, MCONTACT hContact, const char *pData, size_t wDataLen, const char *pNames, size_t wNamesLen, cookie_message_data *pCookieData);
	DWORD  icq_SendChannel4Message(DWORD dwUin, MCONTACT hContact, BYTE bMsgType, size_t wMsgLen, const char *szMsg, cookie_message_data *pCookieData);

	void   icq_sendAdvancedMsgAck(DWORD, DWORD, DWORD, WORD, BYTE, BYTE);
	void   icq_sendContactsAck(DWORD dwUin, char *szUid, DWORD dwMsgID1, DWORD dwMsgID2);

	void   icq_sendReverseReq(directconnect *dc, DWORD dwCookie, cookie_message_data *pCookie);
	void   icq_sendReverseFailed(directconnect* dc, DWORD dwMsgID1, DWORD dwMsgID2, DWORD dwCookie);

	void   icq_sendXtrazRequestServ(DWORD dwUin, DWORD dwCookie, char* szBody, size_t nBodyLen, cookie_message_data *pCookieData);
	void   icq_sendXtrazResponseServ(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szBody, size_t nBodyLen, int nType);

	DWORD  SearchByUin(DWORD dwUin);
	DWORD  SearchByNames(const char *pszNick, const char *pszFirstName, const char *pszLastName, WORD wPage);
	DWORD  SearchByMail(const char *pszEmail);

	DWORD  icq_searchAimByEmail(const char* pszEmail, DWORD dwSearchId);

	void   oft_sendFileRequest(DWORD dwUin, char *szUid, oscar_filetransfer *ft, const char *pszFiles, DWORD dwLocalInternalIP);
	void   oft_sendFileAccept(DWORD dwUin, char *szUid, oscar_filetransfer *ft);
	void   oft_sendFileDeny(DWORD dwUin, char *szUid, oscar_filetransfer *ft);
	void   oft_sendFileCancel(DWORD dwUin, char *szUid, oscar_filetransfer *ft);
	void   oft_sendFileResponse(DWORD dwUin, char *szUid, oscar_filetransfer *ft, WORD wResponse);
	void   oft_sendFileRedirect(DWORD dwUin, char *szUid, oscar_filetransfer *ft, DWORD dwIP, WORD wPort, int bProxy);

	//---- | icq_svcs.cpp |----------------------------------------------------------------
	MCONTACT AddToListByUIN(DWORD dwUin, DWORD dwFlags);
	MCONTACT AddToListByUID(const char *szUID, DWORD dwFlags);

	void   ICQAddRecvEvent(MCONTACT hContact, WORD wType, PROTORECVEVENT* pre, size_t cbBlob, PBYTE pBlob, DWORD flags);
	INT_PTR __cdecl IcqAddCapability(WPARAM wParam, LPARAM lParam);
	INT_PTR __cdecl IcqCheckCapability(WPARAM wParam, LPARAM lParam);

	OBJLIST<ICQ_CUSTOMCAP> CustomCapList;

	//----| icq_uploadui.cpp |------------------------------------------------------------
	void   ShowUploadContactsDialog(void);

	//----| icq_xstatus.cpp |-------------------------------------------------------------
	int    m_bHideXStatusUI;
	int    m_bHideXStatusMenu;
	int    bXStatusExtraIconsReady;
	HANDLE hHookExtraIconsRebuild;
	HANDLE hHookStatusBuild;
	HANDLE hHookExtraIconsApply;
	HGENMENU hXStatusItems[XSTATUS_COUNT + 1];

	void   InitXStatusItems(BOOL bAllowStatus);
	BYTE   getContactXStatus(MCONTACT hContact);
	DWORD  sendXStatusDetailsRequest(MCONTACT hContact, int bForced);
	DWORD  requestXStatusDetails(MCONTACT hContact, BOOL bAllowDelay);
	HICON  getXStatusIcon(int bStatus, UINT flags);
	void   setXStatusEx(BYTE bXStatus, BYTE bQuiet);
	void   handleXStatusCaps(DWORD dwUIN, char *szUID, MCONTACT hContact, BYTE *caps, int capsize, char *moods, int moodsize);
	void   updateServerCustomStatus(int fullUpdate);

	//----| icq_xtraz.cpp |---------------------------------------------------------------
	void   handleXtrazNotify(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szMsg, size_t nMsgLen, BOOL bThruDC);
	void   handleXtrazNotifyResponse(DWORD dwUin, MCONTACT hContact, WORD wCookie, char* szMsg, size_t nMsgLen);

	void   handleXtrazInvitation(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szMsg, size_t nMsgLen, BOOL bThruDC);
	void   handleXtrazData(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szMsg, size_t nMsgLen, BOOL bThruDC);

	DWORD  SendXtrazNotifyRequest(MCONTACT hContact, char* szQuery, char* szNotify, int bForced);
	void   SendXtrazNotifyResponse(DWORD dwUin, DWORD dwMID, DWORD dwMID2, WORD wCookie, char* szResponse, size_t nResponseLen, BOOL bThruDC);

	//----| init.cpp |--------------------------------------------------------------------
	void   UpdateGlobalSettings();

	//----| loginpassword.cpp |-----------------------------------------------------------
	void   RequestPassword();

	//----| oscar_filetransfer.cpp |------------------------------------------------------
	icq_critical_section *oftMutex;
	int fileTransferCount;
	basic_filetransfer** fileTransferList;

	oscar_filetransfer* CreateOscarTransfer();
	filetransfer *CreateIcqFileTransfer();
	void   ReleaseFileTransfer(void *ft);
	void   SafeReleaseFileTransfer(void **ft);
	oscar_filetransfer* FindOscarTransfer(MCONTACT hContact, DWORD dwID1, DWORD dwID2);

	oscar_listener* CreateOscarListener(oscar_filetransfer *ft, NETLIBNEWCONNECTIONPROC_V2 handler);
	void   ReleaseOscarListener(oscar_listener **pListener);

	void   OpenOscarConnection(MCONTACT hContact, oscar_filetransfer *ft, int type);
	void   CloseOscarConnection(oscar_connection *oc);
	int    CreateOscarProxyConnection(oscar_connection *oc);

	int    getFileTransferIndex(void *ft);
	int    IsValidFileTransfer(void *ft);
	int    IsValidOscarTransfer(void *ft);

	void   handleRecvServMsgOFT(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, DWORD dwID1, DWORD dwID2, WORD wCommand);
	void   handleRecvServResponseOFT(BYTE *buf, size_t wLen, DWORD dwUin, char *szUID, void* ft);

	HANDLE oftInitTransfer(MCONTACT hContact, DWORD dwUin, char *szUid, const TCHAR **pszFiles, const TCHAR *szDescription);
	HANDLE oftFileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szPath);
	DWORD  oftFileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR *szReason);
	DWORD  oftFileCancel(MCONTACT hContact, HANDLE hTransfer);
	void   oftFileResume(oscar_filetransfer *ft, int action, const TCHAR *szFilename);

	void   sendOscarPacket(oscar_connection *oc, icq_packet *packet);
	void   handleOFT2FramePacket(oscar_connection *oc, WORD datatype, BYTE *pBuffer, size_t wLen);
	void   sendOFT2FramePacket(oscar_connection *oc, WORD datatype);

	void   proxy_sendInitTunnel(oscar_connection *oc);
	void   proxy_sendJoinTunnel(oscar_connection *oc, WORD wPort);

	//----| stdpackets.cpp |--------------------------------------------------------------
	void   __cdecl oft_connectionThread(struct oscarthreadstartinfo *otsi);

	int    oft_handlePackets(oscar_connection *oc, BYTE *buf, int len);
	int    oft_handleFileData(oscar_connection *oc, BYTE *buf, int len);
	int    oft_handleProxyData(oscar_connection *oc, BYTE *buf, int len);
	void   oft_sendFileData(oscar_connection *oc);
	void   oft_sendPeerInit(oscar_connection *oc);
	void   oft_sendFileReply(DWORD dwUin, char *szUid, oscar_filetransfer *ft, WORD wResult);

	//----| upload.cpp |------------------------------------------------------------------
	int    StringToListItemId(const char *szSetting,int def);

	//----| utilities.cpp |---------------------------------------------------------------
	char*  ConvertMsgToUserSpecificAnsi(MCONTACT hContact, const char* szMsg);

	char*  GetUserStoredPassword(char *szBuffer, int cbSize);
	char*  GetUserPassword(BOOL bAlways);
	WORD   GetMyStatusFlags();

	DWORD  ReportGenericSendError(MCONTACT hContact, int nType, const char* szErrorMsg);
	void   SetCurrentStatus(int nStatus);

	void   __cdecl ProtocolAckThread(icq_ack_args* pArguments);
	void   SendProtoAck(MCONTACT hContact, DWORD dwCookie, int nAckResult, int nAckType, char* pszMessage);

	int    NetLog_Direct(const char *fmt,...);
	int    NetLog_Uni(BOOL bDC, const char *fmt,...);

	icq_critical_section *contactsCacheMutex;
	LIST<icq_contacts_cache> contactsCache;

	void   AddToContactsCache(MCONTACT hContact, DWORD dwUin, const char *szUid);
	void   DeleteFromContactsCache(MCONTACT hContact);
	void   InitContactsCache();
	void   UninitContactsCache();

	void   AddToSpammerList(DWORD dwUIN);
	BOOL   IsOnSpammerList(DWORD dwUIN);

	HANDLE NetLib_BindPort(NETLIBNEWCONNECTIONPROC_V2 pFunc, void* lParam, WORD *pwPort, DWORD *pdwIntIP);

	MCONTACT HandleFromCacheByUid(DWORD dwUin, const char *szUid);
	MCONTACT HContactFromUIN(DWORD dwUin, int *Added);
	MCONTACT HContactFromUID(DWORD dwUin, const char *szUid, int *Added);
	MCONTACT HContactFromAuthEvent(HANDLE hEvent);

	void   ResetSettingsOnListReload();
	void   ResetSettingsOnConnect();
	void   ResetSettingsOnLoad();

	int    IsMetaInfoChanged(MCONTACT hContact);

	char   *setStatusNoteText, *setStatusMoodData;
	void   __cdecl SetStatusNoteThread(void *pArguments);
	int    SetStatusNote(const char *szStatusNote, DWORD dwDelay, int bForced);
	int    SetStatusMood(const char *szMoodData, DWORD dwDelay);

	void   writeDbInfoSettingTLVStringUtf(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv);
	void   writeDbInfoSettingTLVWord(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv);
	void   writeDbInfoSettingTLVByte(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv);
	void   writeDbInfoSettingTLVDouble(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv);
	void   writeDbInfoSettingTLVDate(MCONTACT hContact, const char *szSettingYear, const char *szSettingMonth, const char *szSettingDay, oscar_tlv_chain *chain, WORD wTlv);
	void   writeDbInfoSettingTLVBlob(MCONTACT hContact, const char *szSetting, oscar_tlv_chain *chain, WORD wTlv);

	char** MirandaStatusToAwayMsg(int nStatus);

	bool   validateStatusMessageRequest(MCONTACT hContact, WORD byMessageType);
};

#endif
