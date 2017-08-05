/*
Copyright (c) 2015 Miranda NG project (https://miranda-ng.org)

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

#ifndef _T_PROTO_H_
#define _T_PROTO_H_


struct CTelegramProto : public PROTO < CTelegramProto >
{

public:

	//////////////////////////////////////////////////////////////////////////////////////
	//Ctors

	CTelegramProto(const char *protoName, const wchar_t *userName);
	~CTelegramProto();

	//////////////////////////////////////////////////////////////////////////////////////
	// Virtual functions

	virtual	MCONTACT  __cdecl AddToList(int flags, PROTOSEARCHRESULT* psr);
	virtual	MCONTACT  __cdecl AddToListByEvent(int flags, int iContact, MEVENT hDbEvent);
	virtual int       __cdecl AuthRequest(MCONTACT hContact, const TCHAR* szMessage);
	virtual	int       __cdecl Authorize(MEVENT hDbEvent);
	virtual	int       __cdecl AuthDeny(MEVENT hDbEvent, const TCHAR* szReason);
	virtual	int       __cdecl AuthRecv(MCONTACT hContact, PROTORECVEVENT*);
	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl GetInfo(MCONTACT hContact, int infoType);
	virtual	HANDLE    __cdecl SearchBasic(const TCHAR* id);
	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);
	virtual	int       __cdecl SetStatus(int iNewStatus);
	virtual	int       __cdecl UserIsTyping(MCONTACT hContact, int type);
	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	// accounts
	static CTelegramProto* InitAccount(const char *protoName, const TCHAR *userName);
	static int          UninitAccount(CTelegramProto *proto);

	// icons
	static void InitIcons();
	static void UninitIcons();

	// menus
	static void InitMenus();
	static void UninitMenus();

	//popups
	void InitPopups();
	void UninitPopups();

	void InitNetwork();
	void InitCallbacks();

	// languages
	static void InitLanguages();

	// events
	static int	OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnDbEventRead(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);
	//search
	void __cdecl SearchBasicThread(void* id);

	////////////////////////////////////////////
	static INT_PTR EventGetIcon(WPARAM wParam, LPARAM lParam);
	static INT_PTR GetEventText(WPARAM, LPARAM lParam);

	void TGLGetValue(tgl_value_type type, const char *prompt, int num_values, char **result);



	void OnMessage(tgl_message*);

	void OnUserTyping(tgl_user *U, tgl_typing_status status);

	MirTLS *TLS;
private:

	
	static mir_cs accountsLock;

	void ReadState();
	void SaveState();

	void ReadAuth();
	void SaveAuth();

	//---Accounts
	static LIST<CTelegramProto> CTelegramProto::Accounts; 
	static int CompareAccounts(const CTelegramProto *p1, const CTelegramProto *p2);
	static CTelegramProto* GetContactAccount(MCONTACT hContact);

	__forceinline bool IsOnline() const 
	{	return (m_iStatus > ID_STATUS_OFFLINE);
	}

	template<INT_PTR(__cdecl CTelegramProto::*Service)(WPARAM, LPARAM)>
	static INT_PTR __cdecl GlobalService(WPARAM wParam, LPARAM lParam)
	{
		CTelegramProto *proto = GetContactAccount((MCONTACT)wParam);
		return proto ? (proto->*Service)(wParam, lParam) : 0;
	}

};

#endif //_Telegram_PROTO_H_