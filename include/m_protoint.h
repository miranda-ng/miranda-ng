/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_PROTOINT_H__
#define M_PROTOINT_H__ 1

#include <m_system_cpp.h>
#include <m_protosvc.h>
#include <m_database.h>
#include <m_genmenu.h>
#include <m_utils.h>

/////////////////////////////////////////////////////////////////////////////////////////
// protocol helpers

struct PROTO_INTERFACE;

// Call it in the very beginning of your proto's constructor
EXTERN_C MIR_APP_DLL(void) ProtoConstructor(PROTO_INTERFACE *pThis, const char *pszModuleName, const wchar_t *ptszUserName);

// Call it in the very end of your proto's destructor
EXTERN_C MIR_APP_DLL(void) ProtoDestructor(PROTO_INTERFACE *pThis);

#if defined( __cplusplus )
typedef void (__cdecl PROTO_INTERFACE::*ProtoThreadFunc)(void*);
EXTERN_C MIR_APP_DLL(void)   ProtoForkThread(PROTO_INTERFACE *pThis, ProtoThreadFunc, void *param);
EXTERN_C MIR_APP_DLL(HANDLE) ProtoForkThreadEx(PROTO_INTERFACE *pThis, ProtoThreadFunc, void *param, UINT* threadID);
EXTERN_C MIR_APP_DLL(void)   ProtoWindowAdd(PROTO_INTERFACE *pThis, HWND hwnd);
EXTERN_C MIR_APP_DLL(void)   ProtoWindowRemove(PROTO_INTERFACE *pThis, HWND hwnd);

typedef int (__cdecl PROTO_INTERFACE::*ProtoEventFunc)(WPARAM, LPARAM);
EXTERN_C MIR_APP_DLL(void)   ProtoHookEvent(PROTO_INTERFACE *pThis, const char* szName, ProtoEventFunc pFunc);
EXTERN_C MIR_APP_DLL(HANDLE) ProtoCreateHookableEvent(PROTO_INTERFACE *pThis, const char* szService);

typedef INT_PTR (__cdecl PROTO_INTERFACE::*ProtoServiceFunc)(WPARAM, LPARAM);
EXTERN_C MIR_APP_DLL(void) ProtoCreateService(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFunc);

typedef INT_PTR (__cdecl PROTO_INTERFACE::*ProtoServiceFuncParam)(WPARAM, LPARAM, LPARAM);
EXTERN_C MIR_APP_DLL(void) ProtoCreateServiceParam(PROTO_INTERFACE *pThis, const char* szService, ProtoServiceFuncParam, LPARAM);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// interface declaration

struct MIR_APP_EXPORT PROTO_INTERFACE : public MZeroedObject
{

protected:
	MWindowList m_hWindowList;     // list of all windows which belong to this protocol's instance

public:
	int         m_iStatus;         // current protocol status
	int         m_iDesiredStatus;  // status to be set after logging in
	int         m_iXStatus;        // extanded status
	int         m_iVersion;        // version 2 or higher designate support of Unicode services
	wchar_t*    m_tszUserName;     // human readable protocol's name
	char*       m_szModuleName;    // internal protocol name, also its database module name
	HANDLE      m_hProtoIcon;      // icon to be displayed in the account manager
	HNETLIBUSER m_hNetlibUser;     // network agent
	HGENMENU    m_hmiMainMenu;     // if protocol menus are displayed in the main menu, this is the root
	HGENMENU    m_hmiReqAuth;      // a menu item for /RequestAuth service
	HGENMENU    m_hmiGrantAuth;    // a menu item for /GrantAuth service
	HGENMENU    m_hmiRevokeAuth;   // a menu item for /RevokeAuth service

	PROTO_INTERFACE(const char *pszModuleName, const wchar_t *ptszUserName);
	~PROTO_INTERFACE();

	//////////////////////////////////////////////////////////////////////////////////////
	// Helpers

	__forceinline INT_PTR ProtoBroadcastAck(MCONTACT hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam = 0) {
		return ::ProtoBroadcastAck(m_szModuleName, hContact, type, hResult, hProcess, lParam); }

	__forceinline INT_PTR delSetting(const char *name) { return db_unset(NULL, m_szModuleName, name); }
	__forceinline INT_PTR delSetting(MCONTACT hContact, const char *name) { return db_unset(hContact, m_szModuleName, name); }

	__forceinline bool getBool(const char *name, bool defaultValue = false) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue) != 0; }
	__forceinline bool getBool(MCONTACT hContact, const char *name, bool defaultValue = false) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue) != 0; }

	__forceinline bool isChatRoom(MCONTACT hContact) { return getBool(hContact, "ChatRoom", false); }

	__forceinline int getByte(const char *name, BYTE defaultValue = 0) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getByte(MCONTACT hContact, const char *name, BYTE defaultValue = 0) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue); }

	__forceinline int getWord(const char *name, WORD defaultValue = 0) {
		return db_get_w(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getWord(MCONTACT hContact, const char *name, WORD defaultValue = 0) {
		return db_get_w(hContact, m_szModuleName, name, defaultValue); }

	__forceinline DWORD getDword(const char *name, DWORD defaultValue = 0)  {
		return db_get_dw(NULL, m_szModuleName, name, defaultValue); }
	__forceinline DWORD getDword(MCONTACT hContact, const char *name, DWORD defaultValue = 0) {
		return db_get_dw(hContact, m_szModuleName, name, defaultValue); }

	__forceinline INT_PTR getString(const char *name, DBVARIANT *result) {
		return db_get_s(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getString(MCONTACT hContact, const char *name, DBVARIANT *result) {
		return db_get_s(hContact, m_szModuleName, name, result); }

	__forceinline INT_PTR getWString(const char *name, DBVARIANT *result) {
		return db_get_ws(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getWString(MCONTACT hContact, const char *name, DBVARIANT *result) {
		return db_get_ws(hContact, m_szModuleName, name, result); }

	__forceinline char* getStringA(const char *name) {
		return db_get_sa(NULL, m_szModuleName, name); }
	__forceinline char* getStringA(MCONTACT hContact, const char *name) {
		return db_get_sa(hContact, m_szModuleName, name); }

	__forceinline CMStringA getMStringA(const char *name) {
		return CMStringA(ptrA(db_get_sa(NULL, m_szModuleName, name))); }
	__forceinline CMStringA getMStringA(MCONTACT hContact, const char *name) {
		return CMStringA(ptrA(db_get_sa(hContact, m_szModuleName, name))); }

	__forceinline wchar_t* getWStringA(const char *name) {
		return db_get_wsa(NULL, m_szModuleName, name); }
	__forceinline wchar_t* getWStringA(MCONTACT hContact, const char *name) {
		return db_get_wsa(hContact, m_szModuleName, name); }

	__forceinline CMStringW getMStringW(const char *name) {
		return CMStringW(ptrW(db_get_wsa(NULL, m_szModuleName, name))); }
	__forceinline CMStringW getMStringW(MCONTACT hContact, const char *name) {
		return CMStringW(ptrW(db_get_wsa(hContact, m_szModuleName, name))); }

	__forceinline void setByte(const char *name, BYTE value) { db_set_b(NULL, m_szModuleName, name, value); }
	__forceinline void setByte(MCONTACT hContact, const char *name, BYTE value) { db_set_b(hContact, m_szModuleName, name, value); }

	__forceinline void setWord(const char *name, WORD value) { db_set_w(NULL, m_szModuleName, name, value); }
	__forceinline void setWord(MCONTACT hContact, const char *name, WORD value) { db_set_w(hContact, m_szModuleName, name, value); }

	__forceinline void setDword(const char *name, DWORD value) { db_set_dw(NULL, m_szModuleName, name, value); }
	__forceinline void setDword(MCONTACT hContact, const char *name, DWORD value) { db_set_dw(hContact, m_szModuleName, name, value); }

	__forceinline void setString(const char *name, const char* value) { db_set_s(NULL, m_szModuleName, name, value); }
	__forceinline void setString(MCONTACT hContact, const char *name, const char* value) { db_set_s(hContact, m_szModuleName, name, value); }

	__forceinline void setWString(const char *name, const wchar_t* value) { db_set_ws(NULL, m_szModuleName, name, value); }
	__forceinline void setWString(MCONTACT hContact, const char *name, const wchar_t* value) { db_set_ws(hContact, m_szModuleName, name, value); }

	__forceinline Contacts AccContacts() const { return Contacts(m_szModuleName); }

	//////////////////////////////////////////////////////////////////////////////////////
	// Service functions

	void debugLogA(const char *szFormat, ...);
	void debugLogW(const wchar_t *wszFormat, ...);

	void setAllContactStatuses(int iStatus, bool bSkipChats = true);

	void WindowSubscribe(HWND hwnd);
	void WindowUnsubscribe(HWND hwnd);

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);
						   
	virtual	int      Authorize(MEVENT hDbEvent);
	virtual	int      AuthDeny(MEVENT hDbEvent, const wchar_t* szReason);
	virtual	int      AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      AuthRequest(MCONTACT hContact, const wchar_t* szMessage);
						   
	virtual	HANDLE   FileAllow(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szPath);
	virtual	int      FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int      FileDeny(MCONTACT hContact, HANDLE hTransfer, const wchar_t* szReason);
	virtual	int      FileResume(HANDLE hTransfer, int* action, const wchar_t** szFilename);
						   
	virtual	INT_PTR  GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int      GetInfo(MCONTACT hContact, int infoType);
						   
	virtual	HANDLE   SearchBasic(const wchar_t* id);
	virtual	HANDLE   SearchByEmail(const wchar_t* email);
	virtual	HANDLE   SearchByName(const wchar_t* nick, const wchar_t* firstName, const wchar_t* lastName);
	virtual	HWND     SearchAdvanced(HWND owner);
	virtual	HWND     CreateExtendedSearchUI(HWND owner);
						   
	virtual	int      RecvContacts(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      RecvFile(MCONTACT hContact, PROTORECVFILE*);
	virtual	int      RecvMsg(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int      RecvUrl(MCONTACT hContact, PROTORECVEVENT*);
						   
	virtual	int      SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList);
	virtual	HANDLE   SendFile(MCONTACT hContact, const wchar_t *szDescription, wchar_t **ppszFiles);
	virtual	int      SendMsg(MCONTACT hContact, int flags, const char *msg);
	virtual	int      SendUrl(MCONTACT hContact, int flags, const char *url);
						   
	virtual	int      SetApparentMode(MCONTACT hContact, int mode);
	virtual	int      SetStatus(int iNewStatus);
						   
	virtual	HANDLE   GetAwayMsg(MCONTACT hContact);
	virtual	int      RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int      SetAwayMsg(int iStatus, const wchar_t* msg);
						   
	virtual	int      UserIsTyping(MCONTACT hContact, int type);
						   
	//////////////////////////////////////////////////////////////////////////////////////
	// events

	// builds the account's protocol menu
	virtual void OnBuildProtoMenu(void);

	// called when an account's contact is deleted
	virtual void OnContactDeleted(MCONTACT);

	// called when an account gets physically removed from the database
	virtual void OnErase();

	// the analog of ME_SYSTEM_MODULESLOADED for an account
	virtual void OnModulesLoaded(void);

	// same for ME_SYSTEM_SHUTDOWN
	virtual void OnShutdown(void);

	// same for ME_SYSTEM_OKTOEXIT
	virtual bool IsReadyToExit(void);
};

/////////////////////////////////////////////////////////////////////////////////////////
// Basic class for all protocols written in C++

template<class T> struct PROTO : public PROTO_INTERFACE
{
	typedef PROTO_INTERFACE CSuper;

	__forceinline PROTO(const char *szProto, const wchar_t *tszUserName) :
		PROTO_INTERFACE(szProto, tszUserName)
	{}

	__forceinline HANDLE CreateProtoEvent(const char *name) {
		return ::ProtoCreateHookableEvent(this, name); }

	typedef int(__cdecl T::*MyEventFunc)(WPARAM, LPARAM);
	__forceinline void HookProtoEvent(const char *name, MyEventFunc pFunc) {
		::ProtoHookEvent(this, name, (ProtoEventFunc)pFunc); }

	typedef void(__cdecl T::*MyThreadFunc)(void*);
	__forceinline void ForkThread(MyThreadFunc pFunc, void *param) {
		::ProtoForkThread(this, (ProtoThreadFunc)pFunc, param); }
	HANDLE __forceinline ForkThreadEx(MyThreadFunc pFunc, void *param, UINT *pThreadId) {
		return ::ProtoForkThreadEx(this, (ProtoThreadFunc)pFunc, param, pThreadId); }

	typedef INT_PTR(__cdecl T::*MyServiceFunc)(WPARAM, LPARAM);
	__forceinline void CreateProtoService(const char *name, MyServiceFunc pFunc) {
		::ProtoCreateService(this, name, (ProtoServiceFunc)pFunc); }

	typedef INT_PTR(__cdecl T::*MyServiceFuncParam)(WPARAM, LPARAM, LPARAM);
	__forceinline void CreateProtoServiceParam(const char *name, MyServiceFuncParam pFunc, LPARAM param) {
		::ProtoCreateServiceParam(this, name, (ProtoServiceFuncParam)pFunc, param); }
};
#endif // M_PROTOINT_H__
