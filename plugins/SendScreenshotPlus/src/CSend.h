/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

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

/////////////////////////////////////////////////////////////////////////////////////////

#define SS_AUTOSEND              1
#define SS_DELETEAFTERSSEND      2

#define SS_DLG_AUTOSEND          1 // Button_Enable(GetDlgItem(Owner, ID_chkEmulateClick), TRUE);
#define SS_DLG_DELETEAFTERSSEND  2 // Button_Enable(GetDlgItem(Owner, ID_chkDeleteAfterSend), TRUE);
#define SS_DLG_DESCRIPTION       4 // Button_Enable(GetDlgItem(Owner, ID_chkDesc), TRUE);

#define GC_RESULT_SUCCESS        200
#define GC_RESULT_WRONGVER       201
#define GC_RESULT_ERROR          202
#define GC_RESULT_NOSESSION      209

const wchar_t SS_ERR_INIT[] = LPGENW("Unable to initiate %s.");
const wchar_t SS_ERR_MAPI[] = LPGENW("MAPI error (%i):\n%s.");
const wchar_t SS_ERR_RESPONSE[] = LPGENW("Unknown response from %s (%i)");
const wchar_t SS_ERR_NORESPONSE[] = LPGENW("Got no response from %s (%i)");

/////////////////////////////////////////////////////////////////////////////////////////

class CSend
{
public:
	CSend(HWND Owner, MCONTACT hContact, bool bAsync, bool bSilent=false); // oder (TfrmMain & Owner)
	virtual ~CSend();

	virtual int Send() = 0; // returns 1 if sent (you must delete class) and 0 when still sending (class deletes itself)
	int         SendSilent() { m_bAsync = m_bSilent = true; return Send(); }
		
	void        SetFile(const wchar_t* file) { replaceStrW(m_pszFile, file); }
	void        SetFile(const char* file) { mir_free(m_pszFile), m_pszFile=mir_a2u(file); }
	void        SetDescription(const wchar_t* descr){ replaceStrW(m_pszFileDesc, descr); }
	void        SetContact(MCONTACT hContact);
	const char* GetURL() { return m_URL; }
	const char* GetURLthumbnail() {return m_URLthumb; }
	uint8_t        GetEnableItem() {return m_EnableItem;};
	wchar_t*    GetErrorMsg() {return m_ErrorMsg;};

	bool        m_bDeleteAfterSend;

protected:
	bool        m_bAsync;
	bool        m_bSilent;
	wchar_t*    m_pszFile;
	wchar_t*    m_pszFileDesc;
	CMStringA   m_URL;
	CMStringA   m_URLthumb;
	static int  OnSend(void *obj, WPARAM wParam, LPARAM lParam);
	wchar_t*    m_pszSendTyp;      // hold string for error mess
	char*       m_pszProto;        // Contact Proto Module
	MCONTACT    m_hContact;        // Contact handle
	uint8_t        m_EnableItem;      // hold flag for send type
	uint8_t        m_ChatRoom;        // is Contact chatroom
				   
	void        Error(LPCTSTR pszFormat, ...);
	void        svcSendFileExit();
	void        svcSendMsgExit(const char* szMessage);
	void        Exit(unsigned int Result);
				   
	uint32_t       m_cbEventMsg;      // sizeof EventMsg(T) buffer
	CMStringA   m_szEventMsg;      // EventMsg char*
	HANDLE      m_hSend;           // protocol send handle
	HANDLE      m_hOnSend;         // HookEventObj on ME_PROTO_ACK
				   
	MSGBOX      m_box;
	wchar_t*    m_ErrorMsg;
	wchar_t*    m_ErrorTitle;
      
	void Unhook(){if(m_hOnSend) {UnhookEvent(m_hOnSend);m_hOnSend = nullptr;}}
	void DB_EventAdd(uint16_t EventType);
      
	static INT_PTR CALLBACK ResultDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
      
	/// HTTP upload helper stuff
	enum HTTPFormFlags
	{
		HTTPFF_HEADER = 0x80,
		HTTPFF_TEXT   = 0x00,
		HTTPFF_8BIT   = 0x01,
		HTTPFF_FILE   = 0x02,
		HTTPFF_INT    = 0x04,
	};
	
	#define HTTPFORM_HEADER(str) str,HTTPFF_HEADER
	#define HTTPFORM_TEXT(str) str,HTTPFF_TEXT
	#define HTTPFORM_8BIT(str) str,HTTPFF_8BIT
	#define HTTPFORM_FILE(str) str,HTTPFF_FILE
	#define HTTPFORM_INT(int) (const char*)(int),HTTPFF_INT

	struct HTTPFormData
	{
		const char *name;
		union{
			const char* value_str;
			intptr_t value_int;
		};
		int flags;
	};

	static const char* GetHTMLContent(char* str, const char* startTag, const char* endTag); // changes "str", can be successfully used only once
	void HTTPFormDestroy(NETLIBHTTPREQUEST* nlhr); // use to free data inside "nlhr" created by HTTPFormCreate
	int HTTPFormCreate(NETLIBHTTPREQUEST* nlhr, int requestType, const char* url, HTTPFormData* frm, size_t frmNum); // returns "0" on success, Exit() will be called on failure (stop processing)
};

#endif
