//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#pragma once

#define MESSAGE_URL_FORMAT_STANDARD _T("%s/#inbox/%x%08x")
#define MESSAGE_URL_FORMAT_HTML     _T("%s/h/?v=c&th=%x%08x")

#define JABBER_FEAT_GTALK_SHARED_STATUS _T("google:shared-status")

#define JABBER_EXT_GTALK_PMUC      _T("pmuc-v1")

#define NOTIFY_FEATURE_XMLNS       _T("google:mail:notify")
#define SETTING_FEATURE_XMLNS      _T("google:setting")
#define DISCOVERY_XMLNS            _T("http://jabber.org/protocol/disco#info")

#define ATTRNAME_TYPE              _T("type")
#define ATTRNAME_FROM              _T("from")
#define ATTRNAME_TO                _T("to")
#define ATTRNAME_URL               _T("url")
#define ATTRNAME_TID               _T("tid")
#define ATTRNAME_UNREAD            _T("unread")
#define ATTRNAME_XMLNS             _T("xmlns")
#define ATTRNAME_ID                _T("id")
#define ATTRNAME_TOTAL_MATCHED     _T("total-matched")
#define ATTRNAME_NAME              _T("name")
#define ATTRNAME_ADDRESS           _T("address")
#define ATTRNAME_RESULT_TIME       _T("result-time")
#define ATTRNAME_NEWER_THAN_TIME   _T("newer-than-time")
#define ATTRNAME_NEWER_THAN_TID    _T("newer-than-tid")
#define ATTRNAME_VALUE             _T("value")
#define ATTRNAME_VAR               _T("var")

#define IQTYPE_RESULT              _T("result")
#define IQTYPE_SET                 _T("set")
#define IQTYPE_GET                 _T("get")

#define NODENAME_MAILBOX           _T("mailbox")
#define NODENAME_QUERY             _T("query")
#define NODENAME_IQ                _T("iq")
#define NODENAME_USERSETTING       _T("usersetting")
#define NODENAME_MAILNOTIFICATIONS _T("mailnotifications")
#define NODENAME_SUBJECT           _T("subject")
#define NODENAME_SNIPPET           _T("snippet")
#define NODENAME_SENDERS           _T("senders")
#define NODENAME_FEATURE           _T("feature")
#define NODENAME_NEW_MAIL          _T("new-mail")

#define SETTING_TRUE               _T("true")

#define RESPONSE_TIMEOUT (1000 * 60 * 60)
#define TIMER_INTERVAL   (1000 * 60 * 2)

struct GoogleTalkAcc : public MZeroedObject
{
	IJabberInterface *m_japi;
	PROTOACCOUNT *m_pa;

	// Google Shared Status
	BOOL m_bGoogleSharedStatus;
	BOOL m_bGoogleSharedStatusLock;
	void SendIqGoogleSharedStatus(LPCTSTR status, LPCTSTR msg);
};

GoogleTalkAcc* isGoogle(LPARAM);
