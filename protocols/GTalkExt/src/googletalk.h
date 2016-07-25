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

#define MESSAGE_URL_FORMAT_STANDARD L"%s/#inbox/%x%08x"
#define MESSAGE_URL_FORMAT_HTML     L"%s/h/?v=c&th=%x%08x"

#define JABBER_FEAT_GTALK_SHARED_STATUS L"google:shared-status"

#define JABBER_EXT_GTALK_PMUC      L"pmuc-v1"

#define NOTIFY_FEATURE_XMLNS       L"google:mail:notify"
#define SETTING_FEATURE_XMLNS      L"google:setting"
#define DISCOVERY_XMLNS            L"http://jabber.org/protocol/disco#info"

#define ATTRNAME_TYPE              L"type"
#define ATTRNAME_FROM              L"from"
#define ATTRNAME_TO                L"to"
#define ATTRNAME_URL               L"url"
#define ATTRNAME_TID               L"tid"
#define ATTRNAME_UNREAD            L"unread"
#define ATTRNAME_XMLNS             L"xmlns"
#define ATTRNAME_ID                L"id"
#define ATTRNAME_TOTAL_MATCHED     L"total-matched"
#define ATTRNAME_NAME              L"name"
#define ATTRNAME_ADDRESS           L"address"
#define ATTRNAME_RESULT_TIME       L"result-time"
#define ATTRNAME_NEWER_THAN_TIME   L"newer-than-time"
#define ATTRNAME_NEWER_THAN_TID    L"newer-than-tid"
#define ATTRNAME_VALUE             L"value"
#define ATTRNAME_VAR               L"var"

#define IQTYPE_RESULT              L"result"
#define IQTYPE_SET                 L"set"
#define IQTYPE_GET                 L"get"

#define NODENAME_MAILBOX           L"mailbox"
#define NODENAME_QUERY             L"query"
#define NODENAME_IQ                L"iq"
#define NODENAME_USERSETTING       L"usersetting"
#define NODENAME_MAILNOTIFICATIONS L"mailnotifications"
#define NODENAME_SUBJECT           L"subject"
#define NODENAME_SNIPPET           L"snippet"
#define NODENAME_SENDERS           L"senders"
#define NODENAME_FEATURE           L"feature"
#define NODENAME_NEW_MAIL          L"new-mail"

#define SETTING_TRUE               L"true"

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
