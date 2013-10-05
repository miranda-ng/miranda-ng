/*
Copyright (C) 2013 Miranda NG Project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

struct CVkProto : public PROTO<CVkProto>
{
				CVkProto(const char*, const TCHAR*);
				~CVkProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	HANDLE __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	HANDLE __cdecl AddToListByEvent(int flags, int iContact, HANDLE hDbEvent);

	virtual	int    __cdecl Authorize(HANDLE hDbEvent);
	virtual	int    __cdecl AuthDeny(HANDLE hDbEvent, const TCHAR *szReason);
	virtual	int    __cdecl AuthRecv(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl AuthRequest(HANDLE hContact, const TCHAR *szMessage);

	virtual	HANDLE __cdecl ChangeInfo(int iInfoType, void* pInfoData);

	virtual	HANDLE __cdecl FileAllow(HANDLE hContact, HANDLE hTransfer, const TCHAR *szPath);
	virtual	int    __cdecl FileCancel(HANDLE hContact, HANDLE hTransfer);
	virtual	int    __cdecl FileDeny(HANDLE hContact, HANDLE hTransfer, const TCHAR *szReason);
	virtual	int    __cdecl FileResume(HANDLE hTransfer, int* action, const TCHAR** szFilename);

	virtual	DWORD_PTR __cdecl GetCaps(int type, HANDLE hContact = NULL);
	virtual	int    __cdecl GetInfo(HANDLE hContact, int infoType);

	virtual	HANDLE __cdecl SearchBasic(const TCHAR *id);
	virtual	HANDLE __cdecl SearchByEmail(const TCHAR *email);
	virtual	HANDLE __cdecl SearchByName(const TCHAR *nick, const TCHAR *firstName, const TCHAR *lastName);
	virtual	HWND   __cdecl SearchAdvanced(HWND owner);
	virtual	HWND   __cdecl CreateExtendedSearchUI(HWND owner);

	virtual	int    __cdecl RecvContacts(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl RecvFile(HANDLE hContact, PROTORECVFILET*);
	virtual	int    __cdecl RecvMsg(HANDLE hContact, PROTORECVEVENT*);
	virtual	int    __cdecl RecvUrl(HANDLE hContact, PROTORECVEVENT*);

	virtual	int    __cdecl SendContacts(HANDLE hContact, int flags, int nContacts, HANDLE* hContactsList);
	virtual	HANDLE __cdecl SendFile(HANDLE hContact, const TCHAR *szDescription, TCHAR** ppszFiles);
	virtual	int    __cdecl SendMsg(HANDLE hContact, int flags, const char* msg);
	virtual	int    __cdecl SendUrl(HANDLE hContact, int flags, const char* url);

	virtual	int    __cdecl SetApparentMode(HANDLE hContact, int mode);
	virtual	int    __cdecl SetStatus(int iNewStatus);

	virtual	HANDLE __cdecl GetAwayMsg(HANDLE hContact);
	virtual	int    __cdecl RecvAwayMsg(HANDLE hContact, int mode, PROTORECVEVENT* evt);
	virtual	int    __cdecl SetAwayMsg(int m_iStatus, const TCHAR *msg);

	virtual	int    __cdecl UserIsTyping(HANDLE hContact, int type);

	virtual	int    __cdecl OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam);

	//==== Events ========================================================================

	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);

	//==== Services ======================================================================

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	//==== Misc ==========================================================================

	TCHAR* GetUserStoredPassword(void);

	__forceinline bool IsOnline() const { return m_bOnline; }

	void ShutdownSession();
	void OnLoggedOut();
	void __cdecl WorkerThread(void*);

private:
	int  SetServerStatus(int);

	bool m_bOnline;
	UINT m_hWorkerThread;
};
