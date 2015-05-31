/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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
#include <m_protomod.h>
#include <m_database.h>

typedef enum
{
	EV_PROTO_ONLOAD,
	EV_PROTO_ONREADYTOEXIT,
	EV_PROTO_ONEXIT,
	EV_PROTO_ONRENAME,
	EV_PROTO_ONOPTIONS,
	EV_PROTO_ONERASE,
	EV_PROTO_ONMENU,
	EV_PROTO_ONCONTACTDELETED,
	EV_PROTO_DBSETTINGSCHANGED,
}
	PROTOEVENTTYPE;

struct MIR_CORE_EXPORT PROTO_INTERFACE : public MZeroedObject
{
	int    m_iStatus,         // current protocol status
	       m_iDesiredStatus,  // status to be set after logging in
	       m_iXStatus,		  // extanded status
	       m_iVersion;        // version 2 or higher designate support of Unicode services
	TCHAR* m_tszUserName;     // human readable protocol's name
	char*  m_szModuleName;    // internal protocol name, also its database module name
	HANDLE m_hProtoIcon;      // icon to be displayed in the account manager
	HANDLE m_hNetlibUser;     // network agent
	HANDLE m_hWindowList;     // list of all windows which belong to this protocol's instance

	//////////////////////////////////////////////////////////////////////////////////////
	// Helpers

	__inline void debugLogA(LPCSTR szFormat, ...)
	{
		va_list args;
		va_start(args, szFormat);
		ProtoLogA(this, szFormat, args);
	}
	__inline void debugLogW(LPCWSTR wszFormat, ...)
	{
		va_list args;
		va_start(args, wszFormat);
		ProtoLogW(this, wszFormat, args);
	}

	__forceinline void WindowSubscribe(HWND hwnd) {
		::ProtoWindowAdd(this, hwnd); }
	__forceinline void WindowUnsubscribe(HWND hwnd) {
		::ProtoWindowRemove(this, hwnd); }

	__forceinline INT_PTR ProtoBroadcastAck(MCONTACT hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam) {
		return ::ProtoBroadcastAck(m_szModuleName, hContact, type, hResult, hProcess, lParam); }

	__forceinline INT_PTR delSetting(const char *name) { return db_unset(NULL, m_szModuleName, name); }
	__forceinline INT_PTR delSetting(MCONTACT hContact, const char *name) { return db_unset(hContact, m_szModuleName, name); }

	__forceinline bool getBool(const char *name, bool defaultValue) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue) != 0; }
	__forceinline bool getBool(MCONTACT hContact, const char *name, bool defaultValue) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue) != 0; }

	__forceinline bool isChatRoom(MCONTACT hContact) { return getBool(hContact, "ChatRoom", false); }

	__forceinline int getByte(const char *name, BYTE defaultValue) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getByte(MCONTACT hContact, const char *name, BYTE defaultValue) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue); }

	__forceinline int getWord(const char *name, WORD defaultValue) {
		return db_get_w(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getWord(MCONTACT hContact, const char *name, WORD defaultValue) {
		return db_get_w(hContact, m_szModuleName, name, defaultValue); }

	__forceinline DWORD getDword(const char *name, DWORD defaultValue)  {
		return db_get_dw(NULL, m_szModuleName, name, defaultValue); }
	__forceinline DWORD getDword(MCONTACT hContact, const char *name, DWORD defaultValue) {
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

	__forceinline WCHAR* getWStringA(const char *name) {
		return db_get_wsa(NULL, m_szModuleName, name); }
	__forceinline WCHAR* getWStringA(MCONTACT hContact, const char *name) {
		return db_get_wsa(hContact, m_szModuleName, name); }

	__forceinline void setByte(const char *name, BYTE value) { db_set_b(NULL, m_szModuleName, name, value); }
	__forceinline void setByte(MCONTACT hContact, const char *name, BYTE value) { db_set_b(hContact, m_szModuleName, name, value); }

	__forceinline void setWord(const char *name, WORD value) { db_set_w(NULL, m_szModuleName, name, value); }
	__forceinline void setWord(MCONTACT hContact, const char *name, WORD value) { db_set_w(hContact, m_szModuleName, name, value); }

	__forceinline void setDword(const char *name, DWORD value) { db_set_dw(NULL, m_szModuleName, name, value); }
	__forceinline void setDword(MCONTACT hContact, const char *name, DWORD value) { db_set_dw(hContact, m_szModuleName, name, value); }

	__forceinline void setString(const char *name, const char* value) { db_set_s(NULL, m_szModuleName, name, value); }
	__forceinline void setString(MCONTACT hContact, const char *name, const char* value) { db_set_s(hContact, m_szModuleName, name, value); }

	__forceinline void setWString(const char *name, const WCHAR* value) { db_set_ws(NULL, m_szModuleName, name, value); }
	__forceinline void setWString(MCONTACT hContact, const char *name, const WCHAR* value) { db_set_ws(hContact, m_szModuleName, name, value); }

	#if defined(_UNICODE)
		#define debugLog    debugLogW
		#define getTString  getWString
		#define getTStringA getWStringA
		#define setTString  setWString
	#else
		#define debugLog    debugLogA
		#define getTString  getString
		#define getTStringA getStringA
		#define setTString  setString
	#endif

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT  __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);
						    
	virtual	int       __cdecl Authorize(MEVENT hDbEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR* szReason);
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage);
						    
	virtual	HANDLE    __cdecl FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath);
	virtual	int       __cdecl FileCancel(MCONTACT hContact, HANDLE hTransfer);
	virtual	int       __cdecl FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason);
	virtual	int       __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);

	virtual	HANDLE    __cdecl SearchBasic(const TCHAR* id);
	virtual	HANDLE    __cdecl SearchByEmail(const TCHAR* email);
	virtual	HANDLE    __cdecl SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName);
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
	virtual	int       __cdecl RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT* evt);
	virtual	int       __cdecl SetAwayMsg(int iStatus, const TCHAR* msg);

	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);
};

/////////////////////////////////////////////////////////////////////////////////////////
// Basic class for all protocols written in C++

template<class T> struct PROTO : public PROTO_INTERFACE
{
	__forceinline PROTO(const char *szProto, const TCHAR *tszUserName) {
		::ProtoConstructor(this, szProto, tszUserName); }

	__forceinline ~PROTO() {
		::ProtoDestructor(this); }

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
