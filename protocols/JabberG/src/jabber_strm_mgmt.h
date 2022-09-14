/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2018-22 Miranda NG team

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

// XEP-0198 processing

class strm_mgmt
{
	void OnProcessSMa(const TiXmlElement *node);
	void OnProcessSMr(const TiXmlElement *node);
	void ResendNodes(uint32_t count);
	void FinishLoginProcess(ThreadData *info);

	CJabberProto *proto;
	TiXmlDocument xmlStorage;

	bool m_bEnabled;
	bool m_bPendingEnable;
	bool m_bResumeSupported = false;
	bool m_bSessionResumed = false;

	int m_nSrvHCount, m_nLocalHCount, m_nLocalSCount, m_nResumeMaxSeconds;
	time_t m_tConnLostTime;
	std::string m_sResumeId;
	std::list<TiXmlElement*> NodeCache;

public:
	strm_mgmt(CJabberProto *proto);
	void EnableStrmMgmt();
	void HandleOutgoingNode(TiXmlElement *node);
	bool HandleIncommingNode(const TiXmlElement *node);
	void HandleConnectionLost();
	void OnProcessEnabled(const TiXmlElement *node, ThreadData *info);
	void OnProcessResumed(const TiXmlElement *node, ThreadData *info);
	void OnProcessFailed(const TiXmlElement *node, ThreadData * info);
	void CheckStreamFeatures(const TiXmlElement *node);
	void CheckState();
	void ResetState();
	void SendAck();
	void RequestAck();
	bool IsSessionResumed();
	bool IsResumeIdPresent();
};

#endif //JABBER_STRM_MGMT_H