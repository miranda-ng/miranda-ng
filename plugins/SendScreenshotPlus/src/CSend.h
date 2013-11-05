/*

Miranda IM: the free IM client for Microsoft* Windows*
Copyright 2000-2009 Miranda ICQ/IM project, 

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _CSEND_H
#define _CSEND_H

//---------------------------------------------------------------------------
#define SS_AUTOSEND				1
#define SS_DELETEAFTERSSEND		2

#define SS_DLG_AUTOSEND				1		//Button_Enable(GetDlgItem(Owner, ID_chkEmulateClick),	TRUE);
#define SS_DLG_DELETEAFTERSSEND		2		//Button_Enable(GetDlgItem(Owner, ID_btnDeleteAfterSend),	TRUE);
#define SS_DLG_DESCRIPTION			4		//Button_Enable(GetDlgItem(Owner, ID_btnDesc),			TRUE);

#define GC_RESULT_SUCCESS			200
#define GC_RESULT_WRONGVER			201
#define GC_RESULT_ERROR				202
#define GC_RESULT_NOSESSION			209

#define SS_ERR_INIT					LPGENT("Unable to initiate %s.")
#define SS_ERR_MAPI					LPGENT("MAPI error (%i):\n%s.")

//---------------------------------------------------------------------------
class CSend {
	public:
		CSend(HWND Owner, HANDLE hContact, bool bFreeOnExit); // oder (TfrmMain & Owner)
		virtual ~CSend();

		virtual void	Send() = 0;
//		void			SendSync(bool Sync) {m_SendSync = Sync;};
		bool			m_bFreeOnExit;		// need to "delete object;" on exit ?
		void			SetContact(HANDLE hContact);
		BYTE			GetEnableItem() {return m_EnableItem;};
		LPTSTR			GetErrorMsg() {return m_ErrorMsg;};

		LPTSTR			m_pszFile;
		LPTSTR			m_pszFileDesc;

		BOOL			m_bDeleteAfterSend;

	private:

	protected:
		LPTSTR			m_pszSendTyp;		//hold string for error mess
		HWND			m_hWndO;			//window handle of caller
		HANDLE			m_hContact;			//Contact handle
		char*			m_pszProto;			//Contact Proto Modul
		BYTE			m_EnableItem;		//hold flag for send type
		void			AfterSendDelete();
		BYTE			m_ChatRoom;			//is Contact chatroom
//		bool			m_SendSync;			//send sync / async

		bool			hasCap(unsigned int Flag);
		unsigned int	m_PFflag;

		void			svcSendFile();
		void			svcSendUrl (const char* url);
		void			svcSendMsg (const char* szMessage);
		void			svcSendChat();						//main GC service
		void			svcSendChat(const char* szMessage);	//GC ansi wrapper

		DWORD			m_cbEventMsg;						//sizeof EventMsg(T) buffer
		char*			m_szEventMsg;						//EventMsg char*
		LPTSTR			m_szEventMsgT;						//EventMsg TCHAR*
		HANDLE			m_hSend;							//protocol send handle
		HANDLE			m_hOnSend;							//HookEventObj on ME_PROTO_ACK
		int __cdecl		OnSend(WPARAM wParam, LPARAM lParam);
		void			Unhook(){if(m_hOnSend) {UnhookEvent(m_hOnSend);m_hOnSend = NULL;}}
		void			DB_EventAdd(WORD EventType);
		void			Exit(unsigned int Result);

		MSGBOX			m_box;
		LPTSTR			m_ErrorMsg;
		LPTSTR			m_ErrorTitle;
		void			Error(LPCTSTR pszFormat, ...);
};

#endif
