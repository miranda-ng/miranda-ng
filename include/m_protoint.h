/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
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

#define PROTOCHAR TCHAR
#define PROTOFILEEVENT PROTORECVFILET

struct  PROTO_INTERFACE : public MZeroedObject
{
	int    m_iStatus,
	       m_iDesiredStatus,
	       m_iXStatus,
	       m_iVersion;  // version 2 or higher designate support of Unicode services
	TCHAR* m_tszUserName;
	char*  m_szModuleName;
	HANDLE m_hProtoIcon;

	//////////////////////////////////////////////////////////////////////////////////////
	// Helpers

	__forceinline INT_PTR ProtoBroadcastAck(HANDLE hContact, int type, int hResult, HANDLE hProcess, LPARAM lParam) {
		return ::ProtoBroadcastAck(m_szModuleName, hContact, type, hResult, hProcess, lParam); }

	__forceinline INT_PTR delSetting(const char *name) { return db_unset(NULL, m_szModuleName, name); }
	__forceinline INT_PTR delSetting(HANDLE hContact, const char *name) { return db_unset(hContact, m_szModuleName, name); }

	__forceinline bool getBool(const char *name, bool defaultValue) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue) != 0; }
	__forceinline bool getBool(HANDLE hContact, const char *name, bool defaultValue) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue) != 0; }

	__forceinline bool isChatRoom(HANDLE hContact) { return getBool(hContact, "ChatRoom", false); }

	__forceinline int getByte(const char *name, BYTE defaultValue) {
		return db_get_b(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getByte(HANDLE hContact, const char *name, BYTE defaultValue) {
		return db_get_b(hContact, m_szModuleName, name, defaultValue); }

	__forceinline int getWord(const char *name, WORD defaultValue) {
		return db_get_w(NULL, m_szModuleName, name, defaultValue); }
	__forceinline int getWord(HANDLE hContact, const char *name, WORD defaultValue) {
		return db_get_w(hContact, m_szModuleName, name, defaultValue); }

	__forceinline DWORD getDword(const char *name, DWORD defaultValue)  {
		return db_get_dw(NULL, m_szModuleName, name, defaultValue); }
	__forceinline DWORD getDword(HANDLE hContact, const char *name, DWORD defaultValue) {
		return db_get_dw(hContact, m_szModuleName, name, defaultValue); }

	__forceinline INT_PTR getString(const char *name, DBVARIANT *result) {
		return db_get_s(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getString(HANDLE hContact, const char *name, DBVARIANT *result) {
		return db_get_s(hContact, m_szModuleName, name, result); }

	__forceinline INT_PTR getWString(const char *name, DBVARIANT *result) {
		return db_get_ws(NULL, m_szModuleName, name, result); }
	__forceinline INT_PTR getWString(HANDLE hContact, const char *name, DBVARIANT *result) {
		return db_get_ws(hContact, m_szModuleName, name, result); }

	__forceinline char* getStringA(const char *name) {
		return db_get_sa(NULL, m_szModuleName, name); }
	__forceinline char* getStringA(HANDLE hContact, const char *name) {
		return db_get_sa(hContact, m_szModuleName, name); }

	__forceinline WCHAR* getWStringA(const char *name) {
		return db_get_wsa(NULL, m_szModuleName, name); }
	__forceinline WCHAR* getWStringA(HANDLE hContact, const char *name) {
		return db_get_wsa(hContact, m_szModuleName, name); }

	__forceinline void setByte(const char *name, BYTE value) { db_set_b(NULL, m_szModuleName, name, value); }
	__forceinline void setByte(HANDLE hContact, const char *name, BYTE value) { db_set_b(hContact, m_szModuleName, name, value); }

	__forceinline void setWord(const char *name, WORD value) { db_set_w(NULL, m_szModuleName, name, value); }
	__forceinline void setWord(HANDLE hContact, const char *name, WORD value) { db_set_w(hContact, m_szModuleName, name, value); }

	__forceinline void setDword(const char *name, DWORD value) { db_set_dw(NULL, m_szModuleName, name, value); }
	__forceinline void setDword(HANDLE hContact, const char *name, DWORD value) { db_set_dw(hContact, m_szModuleName, name, value); }

	__forceinline void setString(const char *name, const char* value) { db_set_s(NULL, m_szModuleName, name, value); }
	__forceinline void setString(HANDLE hContact, const char *name, const char* value) { db_set_s(hContact, m_szModuleName, name, value); }

	__forceinline void setWString(const char *name, const WCHAR* value) { db_set_ws(NULL, m_szModuleName, name, value); }
	__forceinline void setWString(HANDLE hContact, const char *name, const WCHAR* value) { db_set_ws(hContact, m_szModuleName, name, value); }

	#if defined(_UNICODE)
		#define getTString  getWString
		#define getTStringA getWStringA
		#define setTString  setWString
	#else
		#define getTString  getString
		#define getTStringA getStringA
		#define setTString  setString
	#endif

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	HANDLE   __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr) = 0;
	virtual	HANDLE   __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent) = 0;

	virtual	int      __cdecl Authorize(HANDLE hDbEvent) = 0;
	virtual	int      __cdecl AuthDeny(HANDLE hDbEvent, const PROTOCHAR* szReason) = 0;
	virtual	int      __cdecl AuthRecv(HANDLE hContact, PROTORECVEVENT*) = 0;
	virtual	int      __cdecl AuthRequest(HANDLE hContact, const PROTOCHAR* szMessage) = 0;

	virtual	HANDLE   __cdecl ChangeInfo(int iInfoType, void* pInfoData) = 0;

	virtual	HANDLE   __cdecl FileAllow(HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath) = 0;
	virtual	int      __cdecl FileCancel(HANDLE hContact, HANDLE hTransfer) = 0;
	virtual	int      __cdecl FileDeny(HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason) = 0;
	virtual	int      __cdecl FileResume(HANDLE hTransfer, int* action, const PROTOCHAR** szFilename) = 0;

	virtual	DWORD_PTR __cdecl GetCaps(int type, HANDLE hContact = NULL) = 0;
	virtual	int       __cdecl GetInfo(HANDLE hContact, int infoType) = 0;

	virtual	HANDLE    __cdecl SearchBasic(const PROTOCHAR* id) = 0;
	virtual	HANDLE    __cdecl SearchByEmail(const PROTOCHAR* email) = 0;
	virtual	HANDLE    __cdecl SearchByName(const PROTOCHAR* nick, const PROTOCHAR* firstName, const PROTOCHAR* lastName) = 0;
	virtual	HWND      __cdecl SearchAdvanced(HWND owner) = 0;
	virtual	HWND      __cdecl CreateExtendedSearchUI(HWND owner) = 0;

	virtual	int       __cdecl RecvContacts(HANDLE hContact, PROTORECVEVENT*) = 0;
	virtual	int       __cdecl RecvFile(HANDLE hContact, PROTOFILEEVENT*) = 0;
	virtual	int       __cdecl RecvMsg(HANDLE hContact, PROTORECVEVENT*) = 0;
	virtual	int       __cdecl RecvUrl(HANDLE hContact, PROTORECVEVENT*) = 0;

	virtual	int       __cdecl SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList) = 0;
	virtual	HANDLE    __cdecl SendFile(HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles) = 0;
	virtual	int       __cdecl SendMsg(HANDLE hContact, int flags, const char* msg) = 0;
	virtual	int       __cdecl SendUrl(HANDLE hContact, int flags, const char* url) = 0;

	virtual	int       __cdecl SetApparentMode(HANDLE hContact, int mode) = 0;
	virtual	int       __cdecl SetStatus(int iNewStatus) = 0;

	virtual	HANDLE    __cdecl GetAwayMsg(HANDLE hContact) = 0;
	virtual	int       __cdecl RecvAwayMsg(HANDLE hContact, int mode, PROTORECVEVENT* evt) = 0;
	virtual	int       __cdecl SendAwayMsg(HANDLE hContact, HANDLE hProcess, const char* msg) = 0;
	virtual	int       __cdecl SetAwayMsg(int iStatus, const PROTOCHAR* msg) = 0;

	virtual	int       __cdecl UserIsTyping(HANDLE hContact, int type) = 0;

	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam) = 0;
};

template<class T> class PROTO : public PROTO_INTERFACE
{

public:
	__forceinline PROTO(const char *szProto, const TCHAR *tszUserName)
	{
		::ProtoConstructor(this, szProto, tszUserName);
	}

	__forceinline ~PROTO()
	{
		::ProtoDestructor(this);
	}

	__forceinline HANDLE CreateProtoEvent(const char *name)
	{	return ::ProtoCreateHookableEvent(this, name); } 

	typedef int (__cdecl T::*MyEventFunc)(WPARAM, LPARAM);
	__forceinline void HookProtoEvent(const char *name, MyEventFunc pFunc)
	{	::ProtoHookEvent(this, name, (ProtoEventFunc)pFunc); } 

	typedef void (__cdecl T::*MyThreadFunc)(void*);
	__forceinline void ForkThread(MyThreadFunc pFunc, void *param)
	{	::ProtoForkThread(this, (ProtoThreadFunc)pFunc, param); } 
	HANDLE __forceinline ForkThreadEx(MyThreadFunc pFunc, void *param, UINT *pThreadId)
	{	return ::ProtoForkThreadEx(this, (ProtoThreadFunc)pFunc, param, pThreadId); } 

	typedef INT_PTR (__cdecl T::*MyServiceFunc)(WPARAM, LPARAM);
	__forceinline void CreateProtoService(const char *name, MyServiceFunc pFunc)
	{  ::ProtoCreateService(this, name, (ProtoServiceFunc)pFunc); }

	typedef INT_PTR (__cdecl T::*MyServiceFuncParam)(WPARAM, LPARAM, LPARAM);
	__forceinline void CreateProtoServiceParam(const char *name, MyServiceFuncParam pFunc, LPARAM param)
	{  ::ProtoCreateServiceParam(this, name, (ProtoServiceFuncParam)pFunc, param); }
};
#endif // M_PROTOINT_H__
