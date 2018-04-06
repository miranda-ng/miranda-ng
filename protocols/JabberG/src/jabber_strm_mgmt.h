/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2018 Miranda NG team

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

#ifndef JABBER_STRM_MGMT_H
#define JABBER_STRM_MGMT_H

#include <list>

struct CJabberProto;

class strm_mgmt
{
	void OnProcessSMa(HXML node);
	void OnProcessSMr(HXML node);
	void ResendNodes(uint32_t count);


	CJabberProto *proto;
	uint32_t m_nStrmMgmtSrvHCount, m_nStrmMgmtLocalHCount, m_nStrmMgmtLocalSCount, m_nStrmMgmtResumeMaxSeconds;
	const uint32_t m_nStrmMgmtCacheSize = 10;
	bool m_bStrmMgmtPendingEnable, m_bStrmMgmtEnabled, m_bStrmMgmtResumeSupported, bSessionResumed;
	std::wstring m_sStrmMgmtResumeId;
	std::list<HXML> NodeCache;

public:
	strm_mgmt(CJabberProto *proto);
	void EnableStrmMgmt();
	void HandleOutgoingNode(HXML node);
	void HandleIncommingNode(HXML node);
	void OnProcessEnabled(HXML node, ThreadData *info);
	void OnProcessResumed(HXML node, ThreadData *info);
	void OnProcessFailed(HXML node, ThreadData * info);
	void CheckStreamFeatures(HXML node);
	void CheckState();
	void OnDisconnect();
	void SendAck();
	void RequestAck();
	bool IsSessionResumed();
};

#endif //JABBER_STRM_MGMT_H