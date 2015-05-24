/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015 Robert Pösel

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

class MinecraftDynmapProto : public PROTO<MinecraftDynmapProto>
{
public:
	MinecraftDynmapProto(const char *proto_name, const TCHAR *username);
	~MinecraftDynmapProto();

	inline const char* ModuleName() const {
		return m_szModuleName;
	}

	inline bool isOnline() {
		return (m_iStatus != ID_STATUS_OFFLINE && m_iStatus != ID_STATUS_CONNECTING);
	}

	inline bool isOffline() {
		return (m_iStatus == ID_STATUS_OFFLINE);
	}

	// PROTO_INTERFACE
	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);
	virtual	int       __cdecl SetStatus(int iNewStatus);
	virtual	int       __cdecl OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam);

	// Services
	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);

	// Events
	int  __cdecl OnModulesLoaded(WPARAM, LPARAM);
	// int  __cdecl OnOptionsInit(WPARAM, LPARAM);
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
	void __cdecl SignOnWorker(void*);
	void __cdecl SignOffWorker(void*);
	void __cdecl SendMsgWorker(void*);

	// Chat handling
 	void AddChat(const TCHAR *id,const TCHAR *name);
	void UpdateChat(const TCHAR *name, const TCHAR *message, const time_t timestamp = time(NULL), bool addtochat = true);
	void SendChatMessage(std::string message);
	void AddChatContact(const TCHAR *nick);
	void DeleteChatContact(const TCHAR *name);
	void SetChatStatus(int);
	void ClearChat();
	void SetTopic(const TCHAR *topic = NULL);
	MCONTACT GetChatHandle();

	// Locks
	HANDLE signon_lock_;	
	HANDLE connection_lock_;
	HANDLE send_message_lock_;	
	HANDLE events_loop_lock_;
	
	HANDLE events_loop_event_;
	
	// Handles
	HANDLE hConnection;
	HANDLE hEventsConnection;
	HANDLE chatHandle_;

	// Data storage
	void    store_headers(http::response *resp, NETLIBHTTPHEADER *headers, int headers_count);

	std::string get_server(bool not_last = false);
	std::string get_language();

	// Connection handling
	unsigned int error_count_;

	// Helpers
	bool    handleEntry(const std::string &method);
	bool    handleSuccess(const std::string &method);
	bool    handleError(const std::string &method, const std::string &error = "", bool force_disconnect = false);

	void __inline increment_error() { error_count_++; }
	void __inline decrement_error() { if (error_count_ > 0) error_count_--; }
	void __inline reset_error() { error_count_ = 0; }
	
	// HTTP communication
	http::response sendRequest(const int request_type, std::string *post_data = NULL, std::string *get_data = NULL);
	std::string chooseAction(int, std::string *get_data = NULL);
	NETLIBHTTPHEADER *get_request_headers(int request_type, int *headers_count);

	// Requests and processing
	bool doSignOn();
	bool doEvents();
	bool doSendMessage(const std::string &message_text);

	std::string doGetPage(int);

	// Configuration
	ptrT nick_;
	std::string m_cookie;
	std::string m_server;	
	std::string m_title;	
	std::string m_timestamp;
	int m_interval;	
	int m_updateRate;

};
