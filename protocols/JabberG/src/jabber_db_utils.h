/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007-09  Maxim Mluhov
Copyright (c) 2007-09  Victor Pavlychko
Copyright (ñ) 2012-15 Miranda NG project

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

#ifndef __jabber_db_utils_h__
#define __jabber_db_utils_h__

struct CJabberOptions
{
	CMOption<BYTE> AllowVersionRequests;
	CMOption<BYTE> AcceptHttpAuth;
	CMOption<BYTE> AddRoster2Bookmarks;
	CMOption<BYTE> AutoAcceptAuthorization;
	CMOption<BYTE> AutoAcceptMUC;
	CMOption<BYTE> AutoAdd;
	CMOption<BYTE> AutoJoinBookmarks;
	CMOption<BYTE> AutoJoinConferences;
	CMOption<BYTE> AutoJoinHidden;
	CMOption<BYTE> AvatarType;
	CMOption<BYTE> BsDirect;
	CMOption<BYTE> BsDirectManual;
	CMOption<BYTE> BsOnlyIBB;
	CMOption<BYTE> BsProxyManual;
	CMOption<BYTE> Disable3920auth;
	CMOption<BYTE> DisableFrame;
	CMOption<BYTE> EnableAvatars;
	CMOption<BYTE> EnableRemoteControl;
	CMOption<BYTE> EnableUserActivity;
	CMOption<BYTE> EnableUserMood;
	CMOption<BYTE> EnableUserTune;
	CMOption<BYTE> EnableZlib;
	CMOption<BYTE> ExtendedSearch;
	CMOption<BYTE> FixIncorrectTimestamps;
	CMOption<BYTE> GcLogAffiliations;
	CMOption<BYTE> GcLogBans;
	CMOption<BYTE> GcLogConfig;
	CMOption<BYTE> GcLogRoles;
	CMOption<BYTE> GcLogStatuses;
	CMOption<BYTE> GcLogChatHistory;
	CMOption<BYTE> HostNameAsResource;
	CMOption<BYTE> IgnoreMUCInvites;
	CMOption<BYTE> KeepAlive;
	CMOption<BYTE> LogChatstates;
	CMOption<BYTE> LogPresence;
	CMOption<BYTE> LogPresenceErrors;
	CMOption<BYTE> ManualConnect;
	CMOption<BYTE> MsgAck;
	CMOption<BYTE> RosterSync;
	CMOption<BYTE> SavePassword;
	CMOption<BYTE> UseDomainLogin;
	CMOption<BYTE> ShowForeignResourceInMirVer;
	CMOption<BYTE> ShowOSVersion;
	CMOption<BYTE> ShowTransport;
	CMOption<BYTE> UseSSL;
	CMOption<BYTE> UseTLS;
	CMOption<BYTE> AcceptNotes;
	CMOption<BYTE> AutosaveNotes;
	CMOption<BYTE> EnableMsgArchive;
	CMOption<BYTE> RcMarkMessagesAsRead;
	CMOption<DWORD> ConnectionKeepAliveInterval;
	CMOption<DWORD> ConnectionKeepAliveTimeout;
	CMOption<BYTE> ProcessXMPPLinks;
	CMOption<BYTE> IgnoreRosterGroups;

	CJabberOptions(PROTO_INTERFACE *proto) :
		BsDirect(proto, "BsDirect", TRUE),
		AllowVersionRequests(proto, "AllowVersionRequests", TRUE),
		AcceptHttpAuth(proto, "AcceptHttpAuth", TRUE),
		AddRoster2Bookmarks(proto, "AddRoster2Bookmarks", TRUE),
		AutoAcceptAuthorization(proto, "AutoAcceptAuthorization", FALSE),
		AutoAcceptMUC(proto, "AutoAcceptMUC", FALSE),
		AutoAdd(proto, "AutoAdd", TRUE),
		AutoJoinBookmarks(proto, "AutoJoinBookmarks", TRUE),
		AutoJoinConferences(proto, "AutoJoinConferences", 0),
		AutoJoinHidden(proto, "AutoJoinHidden", TRUE),
		AvatarType(proto, "AvatarType", PA_FORMAT_UNKNOWN),
		BsDirectManual(proto, "BsDirectManual", FALSE),
		BsOnlyIBB(proto, "BsOnlyIBB", FALSE),
		BsProxyManual(proto, "BsProxyManual", FALSE),
		Disable3920auth(proto, "Disable3920auth", FALSE),
		DisableFrame(proto, "DisableFrame", TRUE),
		EnableAvatars(proto, "EnableAvatars", TRUE),
		EnableRemoteControl(proto, "EnableRemoteControl", FALSE),
		EnableMsgArchive(proto, "EnableMsgArchive", FALSE),
		EnableUserActivity(proto, "EnableUserActivity", TRUE),
		EnableUserMood(proto, "EnableUserMood", TRUE),
		EnableUserTune(proto, "EnableUserTune", FALSE),
		EnableZlib(proto, "EnableZlib", TRUE),
		ExtendedSearch(proto, "ExtendedSearch", TRUE),
		FixIncorrectTimestamps(proto, "FixIncorrectTimestamps", TRUE),
		GcLogAffiliations(proto, "GcLogAffiliations", FALSE),
		GcLogBans(proto, "GcLogBans", TRUE),
		GcLogConfig(proto, "GcLogConfig", FALSE),
		GcLogRoles(proto, "GcLogRoles", FALSE),
		GcLogStatuses(proto, "GcLogStatuses", FALSE),
		GcLogChatHistory(proto, "GcLogChatHistory", TRUE),
		HostNameAsResource(proto, "HostNameAsResource", FALSE),
		IgnoreMUCInvites(proto, "IgnoreMUCInvites", FALSE),
		KeepAlive(proto, "KeepAlive", TRUE),
		LogChatstates(proto, "LogChatstates", FALSE),
		LogPresence(proto, "LogPresence", TRUE),
		LogPresenceErrors(proto, "LogPresenceErrors", FALSE),
		ManualConnect(proto, "ManualConnect", FALSE),
		MsgAck(proto, "MsgAck", FALSE),
		RosterSync(proto, "RosterSync", FALSE),
		SavePassword(proto, "SavePassword", TRUE),
		ShowForeignResourceInMirVer(proto, "ShowForeignResourceInMirVer", FALSE),
		ShowOSVersion(proto, "ShowOSVersion", TRUE),
		ShowTransport(proto, "ShowTransport", TRUE),
		UseSSL(proto, "UseSSL", FALSE),
		UseTLS(proto, "UseTLS", TRUE),
		UseDomainLogin(proto, "UseDomainLogin", FALSE),
		AcceptNotes(proto, "AcceptNotes", TRUE),
		AutosaveNotes(proto, "AutosaveNotes", FALSE),
		RcMarkMessagesAsRead(proto, "RcMarkMessagesAsRead", 1),
		ConnectionKeepAliveInterval(proto, "ConnectionKeepAliveInterval", 60000),
		ConnectionKeepAliveTimeout(proto, "ConnectionKeepAliveTimeout", 50000),
		ProcessXMPPLinks(proto, "ProcessXMPPLinks", FALSE),
		IgnoreRosterGroups(proto, "IgnoreRosterGroups", FALSE)
	{}
};

#endif // __jabber_db_utils_h__
