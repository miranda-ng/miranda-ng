/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-15 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

class OmegleProto : public PROTO<OmegleProto>
{
public:
	OmegleProto( const char *proto_name, const TCHAR *username );
	~OmegleProto( );

	inline const char* ModuleName( ) const
	{
		return m_szModuleName;
	}

	inline bool isOnline( )
	{
		return ( m_iStatus != ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_CONNECTING );
	}

	inline bool isOffline( )
	{
		return ( m_iStatus == ID_STATUS_OFFLINE );
	}

	// PROTO_INTERFACE

	virtual	DWORD_PTR __cdecl GetCaps( int type, MCONTACT hContact = NULL );

	virtual	int       __cdecl SetStatus( int iNewStatus );

	virtual	int       __cdecl UserIsTyping( MCONTACT hContact, int type );

	virtual	int       __cdecl OnEvent( PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam );

	// Services
	INT_PTR __cdecl SvcCreateAccMgrUI( WPARAM, LPARAM );

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int  __cdecl OnOptionsInit(WPARAM, LPARAM);
	int  __cdecl OnContactDeleted(WPARAM,LPARAM);
	int  __cdecl OnPreShutdown(WPARAM,LPARAM);
	int  __cdecl OnPrebuildContactMenu(WPARAM,LPARAM);

	// Chat handling
	int     __cdecl OnChatEvent(WPARAM,LPARAM);
	INT_PTR __cdecl OnJoinChat(WPARAM,LPARAM);
	INT_PTR __cdecl OnLeaveChat(WPARAM,LPARAM);
		  
	// Loops
	void __cdecl EventsLoop(void*);

	// Worker threads
	void __cdecl SignOn(void*);
	void __cdecl SignOff(void*);

	void __cdecl SendMsgWorker(void*);
	void __cdecl SendTypingWorker(void*);

	void __cdecl NewChatWorker(void*);
	void __cdecl StopChatWorker(void*);

	void StopChat(bool disconnect = true);
	void NewChat();

	// Contacts handling
	//bool    IsMyContact(HANDLE, bool include_chat = false);

	// Chat handling
 	void AddChat(const TCHAR *id,const TCHAR *name);
	void UpdateChat(const TCHAR *name, const TCHAR *message, bool addtochat = true);
	void SendChatMessage(std::string message);
	void AddChatContact(const TCHAR *nick);
	void DeleteChatContact(const TCHAR *name);
	void SetChatStatus(int);
	void ClearChat();
	void SetTopic(const TCHAR *topic = NULL);
	MCONTACT GetChatHandle();

	// Connection client
	Omegle_client facy;

	HANDLE  signon_lock_;
	HANDLE  log_lock_;
	HANDLE  events_loop_lock_;

	static void CALLBACK APC_callback(ULONG_PTR p);
};
