/*
	New Away System - plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "stdafx.h"
#include "MsgTree.h"
#include "ContactList.h"
#include "statusmodes.h"

#define DB_STATUSMSG "StatusMsg"
#define DB_ENABLEREPLY "EnableReply"
#define DB_IGNOREREQUESTS "IgnoreRequests"

#define DB_UNK_CONTACT_PREFIX "Unk" // DB_ENABLEREPLY, DB_IGNOREREQUESTS and DB_POPUPNOTIFY settings prefix for not-on-list contacts

class _CWndUserData
{
public:
	_CWndUserData(): MsgTree(nullptr), CList(nullptr) {}

	CMsgTree *MsgTree;
	CCList *CList;
};

class CWndUserData
{
public:
	CWndUserData(HWND hWnd) : hWnd(hWnd)
	{
		_ASSERT(hWnd);
		dat = (_CWndUserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (!dat) {
			dat = new _CWndUserData;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)dat);
		}
	}

	~CWndUserData()
	{
		_ASSERT(dat == (_CWndUserData*)GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!dat->MsgTree && !dat->CList) {
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			delete dat; // TODO: memory leak - this is never executed - fix it
		}
	}

	CMsgTree *GetMsgTree() {return dat->MsgTree;}
	void SetMsgTree(CMsgTree *MsgTree) {dat->MsgTree = MsgTree;}
	CCList *GetCList() {return dat->CList;}
	void SetCList(CCList *CList) {dat->CList = CList;}

private:
	HWND hWnd;
	_CWndUserData *dat;
};

class CProtoStates;

class CProtoState
{
public:
	CProtoState(const char *szProto, CProtoStates *Parent) : m_szProto(szProto), m_parent(Parent), m_status(szProto, Parent), m_awaySince(szProto, Parent) {}

	class CStatus
	{
	public:
		CStatus(const char *szProto, CProtoStates *GrandParent) : m_szProto(szProto), m_grandParent(GrandParent), m_status(ID_STATUS_OFFLINE) {}
		CStatus &operator=(int Status);
		operator int() { return m_status; }
		friend class CProtoState;
	private:
		int m_status;
		CString m_szProto;
		CProtoStates *m_grandParent;
	} m_status;

	class CAwaySince
	{
	public:
		CAwaySince(const char *szProto, CProtoStates *GrandParent) : m_szProto(szProto), m_grandParent(GrandParent) { Reset(); }
		void Reset();
		operator LPSYSTEMTIME() { return &m_awaySince; }
		friend class CProtoState;
	private:
		SYSTEMTIME m_awaySince;
		CString m_szProto;
		CProtoStates *m_grandParent;
	} m_awaySince;

	class CCurStatusMsg
	{
	public:
		CCurStatusMsg() { *this = nullptr; }
		CCurStatusMsg &operator=(TCString Msg)
		{
			CurStatusMsg = Msg;
			SYSTEMTIME st;
			GetLocalTime(&st);
			SystemTimeToFileTime(&st, (LPFILETIME)&LastUpdateTime); // i'm too lazy to declare FILETIME structure and then copy its data to absolutely equivalent ULARGE_INTEGER structure, so we'll just pass a pointer to the ULARGE_INTEGER directly ;-P
			return *this;
		}
		operator TCString() { return CurStatusMsg; }
		uint32_t GetUpdateTimeDifference()
		{
			ULARGE_INTEGER CurTime;
			SYSTEMTIME st;
			GetLocalTime(&st);
			SystemTimeToFileTime(&st, (LPFILETIME)&CurTime);
			return (uint32_t)((CurTime.QuadPart - LastUpdateTime.QuadPart) / 10000); // in milliseconds
		}
	private:
		TCString CurStatusMsg;
		ULARGE_INTEGER LastUpdateTime;
	} CurStatusMsg;

	class CTempMsg
	{ // we use temporary messages to keep user-defined per-protocol messages intact, when changing messages through MS_NAS_SETSTATE, or when autoaway becomes active etc.. temporary messages are automatically resetted when protocol status changes
	public:
		CTempMsg() : iIsSet(0) {}
		CTempMsg &operator=(TCString _Msg) { this->Msg = _Msg; iIsSet = true; return *this; }
		operator TCString()
		{
			_ASSERT(iIsSet);
			return Msg;
		}
		void Unset() { iIsSet = false; }
		int IsSet() { return iIsSet; }

	private:
		int iIsSet; // as we need TempMsg to support every possible value, including NULL and "", we'll use this variable to specify whether TempMsg is set
		TCString Msg;
	} TempMsg;

	void SetParent(CProtoStates *_Parent)
	{
		m_parent = _Parent;
		m_status.m_grandParent = _Parent;
		m_awaySince.m_grandParent = _Parent;
	}

	CString &GetProto() { return m_szProto; }

	//NightFox: fix?
	//private:
public:
	CString m_szProto;
	CProtoStates *m_parent;
};

class CProtoStates // this class stores all protocols' dynamic data
{
public:
	CProtoStates() {}

	CProtoStates(const CProtoStates &States) { *this = States; }
	CProtoStates &operator=(const CProtoStates &States)
	{
		ProtoStates = States.ProtoStates;
		for (int i = 0; i < ProtoStates.GetSize(); i++)
			ProtoStates[i].SetParent(this);
		return *this;
	}

	CProtoState &operator[](const char *szProto)
	{
		for (int i = 0; i < ProtoStates.GetSize(); i++)
			if (ProtoStates[i].GetProto() == szProto)
				return ProtoStates[i];

		// we need to be sure that we have _all_ protocols in the list, before dealing with global status, so we're adding them here.
		if (!szProto)
			for (auto &pa : Accounts())
				(*this)[pa->szModuleName]; // add a protocol if it isn't in the list yet

		return ProtoStates[ProtoStates.AddElem(CProtoState(szProto, this))];
	}

	friend class CProtoState;
	friend class CProtoState::CStatus;
	friend class CProtoState::CAwaySince;

private:
	CProtoState &operator[](int nIndex) { return ProtoStates[nIndex]; }
	int GetSize() { return ProtoStates.GetSize(); }

	TMyArray<CProtoState> ProtoStates;
};

extern CProtoStates g_ProtoStates;


static struct
{
	int Status;
	char *Setting;
}
StatusSettings[] = {
	{ ID_STATUS_OFFLINE, "Off" },
	{ ID_STATUS_ONLINE, "Onl" },
	{ ID_STATUS_AWAY, "Away" },
	{ ID_STATUS_NA, "Na" },
	{ ID_STATUS_DND, "Dnd" },
	{ ID_STATUS_OCCUPIED, "Occ" },
	{ ID_STATUS_FREECHAT, "Ffc" },
	{ ID_STATUS_INVISIBLE, "Inv" },
	{ ID_STATUS_IDLE, "Idle" }
};


class CProtoSettings
{
	LPCSTR szProto;

public:
	CProtoSettings(LPCSTR szProto = nullptr, int iStatus = 0) :
		szProto(szProto),
		Status(iStatus, szProto)
	{
		Autoreply.Parent = this;
	}

	CString ProtoStatusToDBSetting(const char *Prefix, int MoreOpt_PerStatusID = 0)
	{
		if (!MoreOpt_PerStatusID || g_MoreOptPage.GetDBValueCopy(MoreOpt_PerStatusID)) {
			for (auto &it: StatusSettings)
				if (Status == it.Status)
					return szProto ? (CString(Prefix) + "_" + szProto + "_" + it.Setting) : (CString(Prefix) + it.Setting);
		}
		return szProto ? (CString(Prefix) + "_" + szProto) : CString(Prefix);
	}

	class CAutoreply
	{
	public:
		CAutoreply &operator=(const int m_value)
		{
			CString Setting(Parent->szProto ? Parent->ProtoStatusToDBSetting(DB_ENABLEREPLY, IDC_MOREOPTDLG_PERSTATUSPROTOSETTINGS) : DB_ENABLEREPLY);
			if (g_plugin.getByte(Setting, VAL_USEDEFAULT) == m_value)
				return *this;

			if (m_value != VAL_USEDEFAULT)
				g_plugin.setByte(Setting, m_value != 0);
			else
				g_plugin.delSetting(Setting);
			return *this;
		}

		operator int()
		{
			return g_plugin.getByte(Parent->szProto ? Parent->ProtoStatusToDBSetting(DB_ENABLEREPLY, IDC_MOREOPTDLG_PERSTATUSPROTOSETTINGS) : DB_ENABLEREPLY,
				Parent->szProto ? VAL_USEDEFAULT : AUTOREPLY_DEF_REPLY);
		}

		int IncludingParents() // takes into account global data also, if per-protocol setting is not defined
		{
			_ASSERT(Parent->szProto);
			int m_value = *this;
			return (m_value == VAL_USEDEFAULT) ? CProtoSettings(nullptr).Autoreply : m_value;
		}
		friend class CProtoSettings;
	private:
		CProtoSettings *Parent;
	} Autoreply;

	class CStatus
	{
	public:
		CStatus(int iStatus = 0, const char *szProto = nullptr) : Status(iStatus), szProto(szProto) {}
		CStatus &operator=(int _Status) { this->Status = _Status; return *this; }
		operator int()
		{
			if (!Status)
				Status = g_ProtoStates[szProto].m_status;
			return Status;
		}
	private:
		int Status;
		const char *szProto;
	} Status;

	void SetMsgFormat(int Flags, TCString Message);
	TCString GetMsgFormat(int Flags, int *pOrder = nullptr);
};


__inline CString StatusToDBSetting(int Status, const char *Prefix, int MoreOpt_PerStatusID = 0)
{
	if (!MoreOpt_PerStatusID || g_MoreOptPage.GetDBValueCopy(MoreOpt_PerStatusID))
		for (auto &it: StatusSettings)
			if (Status == it.Status)
				return CString(Prefix) + it.Setting;

	return CString(Prefix);
}


__inline CString ContactStatusToDBSetting(int Status, const char *Prefix, int MoreOpt_PerStatusID, MCONTACT hContact)
{
	// it's a not-on-list contact
	if (hContact == INVALID_CONTACT_ID)
		return CString(DB_UNK_CONTACT_PREFIX) + Prefix;

	if (hContact)
		StatusToDBSetting(Status, Prefix, MoreOpt_PerStatusID);
	return CString(Prefix);
}


class CContactSettings
{
	MCONTACT m_hContact;

public:
	CContactSettings(int iStatus = 0, MCONTACT _hContact = NULL) : Status(iStatus, _hContact), m_hContact(_hContact)
	{
		Ignore.Parent = this;
		Autoreply.Parent = this;
	}

	CString ContactStatusToDBSetting(const char *Prefix, int MoreOpt_PerStatusID = 0)
	{
		return ::ContactStatusToDBSetting((m_hContact != INVALID_CONTACT_ID) ? Status : NULL, Prefix, MoreOpt_PerStatusID, m_hContact);
	}

	class CIgnore
	{
	public:
		CIgnore &operator=(const int m_value)
		{
			CString Setting(Parent->ContactStatusToDBSetting(DB_IGNOREREQUESTS, IDC_MOREOPTDLG_PERSTATUSPERSONALSETTINGS));
			MCONTACT hContact = (Parent->m_hContact != INVALID_CONTACT_ID) ? Parent->m_hContact : NULL;
			if (m_value)
				g_plugin.setByte(hContact, Setting, 1);
			else
				g_plugin.delSetting(hContact, Setting);
			return *this;
		}

		operator int()
		{
			return db_get_b((Parent->m_hContact != INVALID_CONTACT_ID) ? Parent->m_hContact : NULL, MODULENAME,
				Parent->ContactStatusToDBSetting(DB_IGNOREREQUESTS, IDC_MOREOPTDLG_PERSTATUSPERSONALSETTINGS), 0);
		}

		friend class CContactSettings;
	private:
		CContactSettings *Parent;
	} Ignore;

	class CAutoreply
	{
	public:
		CAutoreply &operator=(const int m_value)
		{
			CString Setting(Parent->ContactStatusToDBSetting(DB_ENABLEREPLY, IDC_MOREOPTDLG_PERSTATUSPERSONALSETTINGS));
			MCONTACT hContact = (Parent->m_hContact != INVALID_CONTACT_ID) ? Parent->m_hContact : NULL;
			if (g_plugin.getByte(hContact, Setting, VAL_USEDEFAULT) == m_value)
				return *this;

			if (m_value != VAL_USEDEFAULT)
				g_plugin.setByte(hContact, Setting, m_value != 0);
			else
				g_plugin.delSetting(hContact, Setting);
			return *this;
		}
		operator int() { return db_get_b((Parent->m_hContact != INVALID_CONTACT_ID) ? Parent->m_hContact : NULL, MODULENAME, Parent->ContactStatusToDBSetting(DB_ENABLEREPLY, IDC_MOREOPTDLG_PERSTATUSPERSONALSETTINGS), Parent->m_hContact ? VAL_USEDEFAULT : AUTOREPLY_DEF_REPLY); }
		int IncludingParents(const char *szProtoOverride = nullptr) // takes into account protocol and global data also, if per-contact setting is not defined
		{
			_ASSERT((Parent->m_hContact && Parent->m_hContact != INVALID_CONTACT_ID) || szProtoOverride); // we need either correct protocol or a correct hContact to determine its protocol
			int m_value = *this;
			if (m_value == VAL_USEDEFAULT) {
				const char *szProto = (Parent->m_hContact && Parent->m_hContact != INVALID_CONTACT_ID) ? Proto_GetBaseAccountName(Parent->m_hContact) : szProtoOverride;
				return CProtoSettings(szProto).Autoreply.IncludingParents();
			}
			return m_value;
		}
		int GetNextToggleValue()
		{
			switch ((int)*this) {
			case VAL_USEDEFAULT: return 0; break;
			case 0: return 1; break;
			default: return Parent->m_hContact ? VAL_USEDEFAULT : AUTOREPLY_DEF_REPLY; break;
			}
		}
		int Toggle() { return *this = GetNextToggleValue(); }
		friend class CContactSettings;
	private:
		CContactSettings *Parent;
	} Autoreply;

	class CStatus
	{
	public:
		CStatus(int iStatus = 0, MCONTACT _hContact = NULL) : Status(iStatus), m_hContact(_hContact) {}
		CStatus &operator=(int _Status) { this->Status = _Status; return *this; }
		operator int()
		{
			if (!Status) {
				_ASSERT(m_hContact != INVALID_CONTACT_ID);
				char *szProto = m_hContact ? Proto_GetBaseAccountName(m_hContact) : nullptr;
				Status = (szProto || !m_hContact) ? g_ProtoStates[szProto].m_status : ID_STATUS_AWAY;
			}
			return Status;
		}
		friend class CPopupNotify;
		friend class CAutoreply;
	private:
		int Status;
		MCONTACT m_hContact;
	} Status;

	void SetMsgFormat(int Flags, TCString Message);
	TCString GetMsgFormat(int Flags, int *pOrder = nullptr, char *szProtoOverride = nullptr);
};
