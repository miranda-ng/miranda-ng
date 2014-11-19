/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

/////////////////////////////////////////////////////////////////////////////////////////
// add file session to a list

void CMsnProto::p2p_registerSession(filetransfer* ft)
{
	mir_cslock lck(m_csSessions);
	m_arSessions.insert(ft);
}

/////////////////////////////////////////////////////////////////////////////////////////
// remove file session from a list

void CMsnProto::p2p_unregisterSession(filetransfer* ft)
{
	mir_cslock lck(m_csSessions);
	m_arSessions.remove(ft);
}

/////////////////////////////////////////////////////////////////////////////////////////
// get session by some parameter

filetransfer* CMsnProto::p2p_getSessionByID(unsigned id)
{
	if (id == 0)
		return NULL;

	mir_cslock lck(m_csSessions);

	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (FT->p2p_sessionid == id)
			return FT;
	}

	return NULL;
}

filetransfer* CMsnProto::p2p_getSessionByUniqueID(unsigned id)
{
	if (id == 0)
		return NULL;

	mir_cslock lck(m_csSessions);

	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (FT->p2p_acksessid == id)
			return FT;
	}

	return NULL;
}


bool CMsnProto::p2p_sessionRegistered(filetransfer* ft)
{
	if (ft != NULL && ft->p2p_appID == 0)
		return true;

	mir_cslock lck(m_csSessions);
	return m_arSessions.getIndex(ft) > -1;
}

filetransfer* CMsnProto::p2p_getThreadSession(MCONTACT hContact, TInfoType mType)
{
	mir_cslock lck(m_csSessions);

	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (FT->std.hContact == hContact && FT->tType == mType)
			return FT;
	}

	return NULL;
}

void CMsnProto::p2p_clearThreadSessions(MCONTACT hContact, TInfoType mType)
{
	mir_cslock lck(m_csSessions);

	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* ft = &m_arSessions[i];
		if (ft->std.hContact == hContact && ft->tType == mType) {
			ft->bCanceled = true;
			ft->tType = SERVER_NOTIFICATION;
			p2p_sendCancel(ft);
		}
	}
}

filetransfer* CMsnProto::p2p_getAvatarSession(MCONTACT hContact)
{
	mir_cslock lck(m_csSessions);

	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (FT->std.hContact == hContact && !(FT->std.flags & PFTS_SENDING) && FT->p2p_type == MSN_APPID_AVATAR)
			return FT;
	}

	return NULL;
}

bool CMsnProto::p2p_isAvatarOnly(MCONTACT hContact)
{
	mir_cslock lck(m_csSessions);

	bool result = true;
	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		result &= FT->std.hContact != hContact || FT->p2p_type != MSN_APPID_FILE;
	}

	return result;
}

void CMsnProto::p2p_clearDormantSessions(void)
{
	mir_cslockfull lck(m_csSessions);

	time_t ts = time(NULL);
	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (!FT->p2p_sessionid && !MSN_GetUnconnectedThread(FT->p2p_dest, SERVER_P2P_DIRECT))
			p2p_invite(FT->p2p_type, FT, NULL);
		else if (FT->p2p_waitack && (ts - FT->ts) > 120) {
			FT->bCanceled = true;
			p2p_sendCancel(FT);
			lck.unlock();
			p2p_unregisterSession(FT);
			lck.lock();
			i = 0;
		}
	}
}

void CMsnProto::p2p_redirectSessions(const char *wlid)
{
	mir_cslock lck(m_csSessions);

	ThreadData* T = MSN_GetP2PThreadByContact(wlid);
	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (_stricmp(FT->p2p_dest, wlid) == 0 &&
			FT->std.currentFileProgress < FT->std.currentFileSize &&
			(T == NULL || (FT->tType != T->mType && FT->tType != 0))) {
			if (FT->p2p_isV2) {
				if ((FT->std.flags & PFTS_SENDING) && T)
					FT->tType = T->mType;
			}
			else {
				if (!(FT->std.flags & PFTS_SENDING))
					p2p_sendRedirect(FT);
			}
		}
	}
}

void CMsnProto::p2p_startSessions(const char* wlid)
{
	mir_cslock lck(m_csSessions);

	char* szEmail;
	parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);

	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (!FT->bAccepted  && !_stricmp(FT->p2p_dest, szEmail)) {
			if (FT->p2p_appID == MSN_APPID_FILE && (FT->std.flags & PFTS_SENDING))
				p2p_invite(FT->p2p_type, FT, wlid);
			else if (FT->p2p_appID != MSN_APPID_FILE && !(FT->std.flags & PFTS_SENDING))
				p2p_invite(FT->p2p_type, FT, wlid);
		}
	}
}

void CMsnProto::p2p_cancelAllSessions(void)
{
	mir_cslock lck(m_csSessions);

	for (int i = 0; i < m_arSessions.getCount(); i++) {
		m_arSessions[i].bCanceled = true;
		p2p_sendCancel(&m_arSessions[i]);
	}
}

filetransfer* CMsnProto::p2p_getSessionByCallID(const char* CallID, const char* wlid)
{
	if (CallID == NULL)
		return NULL;

	mir_cslock lck(m_csSessions);

	char* szEmail = NULL;
	for (int i = 0; i < m_arSessions.getCount(); i++) {
		filetransfer* FT = &m_arSessions[i];
		if (FT->p2p_callID && !_stricmp(FT->p2p_callID, CallID)) {
			if (_stricmp(FT->p2p_dest, wlid)) {
				if (!szEmail)
					parseWLID(NEWSTR_ALLOCA(wlid), NULL, &szEmail, NULL);
				if (_stricmp(FT->p2p_dest, szEmail))
					continue;
			}
			return FT;
		}
	}

	return NULL;
}


void CMsnProto::p2p_registerDC(directconnection* dc)
{
	mir_cslock lck(m_csSessions);
	m_arDirect.insert(dc);
}

void CMsnProto::p2p_unregisterDC(directconnection* dc)
{
	mir_cslock lck(m_csSessions);
	m_arDirect.remove(dc);
}

directconnection* CMsnProto::p2p_getDCByCallID(const char* CallID, const char* wlid)
{
	if (CallID == NULL)
		return NULL;

	mir_cslock lck(m_csSessions);

	for (int i = 0; i < m_arDirect.getCount(); i++) {
		directconnection* DC = &m_arDirect[i];
		if (DC->callId != NULL && !strcmp(DC->callId, CallID) && !strcmp(DC->wlid, wlid))
			return DC;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// external functions

void CMsnProto::P2pSessions_Uninit(void)
{
	mir_cslock lck(m_csSessions);
	m_arSessions.destroy();
	m_arDirect.destroy();
}
