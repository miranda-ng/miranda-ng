/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2018-23 Miranda NG team

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

#include "stdafx.h"
#include "jabber_strm_mgmt.h"

#define CACHE_SIZE 10

strm_mgmt::strm_mgmt(CJabberProto *_proto) :
	proto(_proto)
{
	ResetState();
}

void strm_mgmt::OnProcessEnabled(const TiXmlElement *node, ThreadData * /*info*/)
{
	m_bEnabled = true;
	m_tConnLostTime = 0;

	auto *val = XmlGetAttr(node, "resume");
	if (val) {
		if (mir_strcmp(val, "true") || mir_strcmp(val, "1")) {
			m_bResumeSupported = true;
			m_sResumeId = XmlGetAttr(node, "id");

			val = XmlGetAttr(node, "max");
			m_nResumeMaxSeconds = val ? atoi(val) : 86400;
		}
	}
	//TODO: handle 'location'
	m_nLocalHCount = 0;
}

void strm_mgmt::OnProcessResumed(const TiXmlElement *node, ThreadData * /*info*/)
{
	if (mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		return;

	auto *var = XmlGetAttr(node, "previd");
	if (!var)
		return;

	if (m_sResumeId != var)
		return; //TODO: unknown session, what we should do ?

	m_bSessionResumed = m_bEnabled = true;
	m_bPendingEnable = false;
	m_tConnLostTime = 0;

	//FinishLoginProcess(info);
	proto->OnLoggedIn();
	proto->ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)proto->m_iStatus, proto->m_iDesiredStatus);
	ProcessCache(node->IntAttribute("h", -1), true);
}

void strm_mgmt::OnProcessSMa(const TiXmlElement *node)
{
	if (mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		return;

	if (!m_bRequestPending)
		return;
	m_bRequestPending = false;
	ProcessCache(node->IntAttribute("h", -1), false);
}

void strm_mgmt::ProcessCache(uint32_t nSrvHCount, bool resuming)
{
	int lost = m_nReqLocalSCount - nSrvHCount;
	int untracked = m_nLocalSCount - m_nReqLocalSCount;

	if (nSrvHCount < 0 || m_nReqLocalSCount < 0
		|| lost < 0 || untracked < 0 || lost + untracked > NodeCache.size()) {
		proto->debugLogA("strm_mgmt: error: a bug or wrong ack node, resetting cache");
		for (auto it : NodeCache)
			xmlStorage.DeleteNode(it);
		NodeCache.clear();
		return;
	}

	proto->debugLogA("strm_mgmt: info: lost %d, untracked %d, cache_size %d", lost, untracked, NodeCache.size());

	//delete ack'ed nodes from the cache
	int todelete = (int)NodeCache.size() - (lost + untracked);
	for (int i = 0; i < todelete; i++) {
		xmlStorage.DeleteNode(NodeCache.front());
		NodeCache.pop_front();
	}

	int toresend = resuming ? lost + untracked : lost;
	for (int i = 0; i < toresend; i++) {
		TiXmlElement *it = NodeCache.front();
		proto->m_ThreadInfo->send(it);
		xmlStorage.DeleteNode(it);
		NodeCache.pop_front();
	}
	proto->debugLogA("strm_mgmt: info: deleting %d nodes, resending %d nodes", todelete, toresend);
}

void strm_mgmt::OnProcessSMr(const TiXmlElement *node)
{
	if (!mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		SendAck();
}

void strm_mgmt::OnProcessFailed(const TiXmlElement *node, ThreadData *info) //used failed instead of failure, notes: https://xmpp.org/extensions/xep-0198.html#errors
{
	if (mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		return;

	proto->debugLogA("strm_mgmt: error: Failed to resume session %s", m_sResumeId.c_str());

	m_bEnabled = m_bHalfEnabled = false;
	m_bSessionResumed = false;
	m_sResumeId.clear();

	//resume failed, reset contacts status
	for (auto &hContact : proto->AccContacts())
		proto->SetContactOfflineStatus(hContact);

	auto *subnode = XmlFirstChild(node, "item-not-found");
	if (subnode) {
		m_bPendingEnable = true;
		FinishLoginProcess(info);
	}
	else EnableStrmMgmt(); //resume failed, try to enable strm_mgmt instead
}

void strm_mgmt::CheckStreamFeatures(const TiXmlElement *node)
{
	if (!IsResumeIdPresent())
		ResetState(); //this may be necessary to reset counters if session resume id is not set
	if (mir_strcmp(node->Name(), "sm") || !XmlGetAttr(node, "xmlns") || mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3")) //we work only with version 3 or higher of sm
		return;
	if (!(proto->m_bJabberOnline))
		m_bPendingEnable = true;
	else
		EnableStrmMgmt();
}

void strm_mgmt::CheckState()
{
	if (!m_bEnabled && m_bPendingEnable)
		EnableStrmMgmt();
}

void strm_mgmt::HandleOutgoingNode(const TiXmlElement *node)
{
	if (!m_bHalfEnabled)
		return;

	auto *pNodeCopy = node->DeepClone(&xmlStorage)->ToElement();
	if (pNodeCopy == nullptr)
		return;

	m_nLocalSCount++;
	NodeCache.push_back(pNodeCopy);
	if (NodeCache.size() >= CACHE_SIZE)
		RequestAck();
}

void strm_mgmt::ResetState()
{
	// reset state of stream management
	m_bEnabled = m_bPendingEnable = m_bHalfEnabled = m_bRequestPending = false;
	m_nReqLocalSCount = 0;
	m_nResumeMaxSeconds = 0;
	m_tConnLostTime = 0;

	// reset stream management h counters
	m_nLocalHCount = m_nLocalSCount = 0;

	// clear resume id
	m_sResumeId.clear();
}

void strm_mgmt::HandleConnectionLost()
{
	m_bEnabled = m_bHalfEnabled = m_bRequestPending = false;
	m_nReqLocalSCount = 0;
	m_tConnLostTime = time(0);
}

bool strm_mgmt::HandleIncommingNode(const TiXmlElement *node)
{
	if (!m_bEnabled)
		return false;

	if (!mir_strcmp(node->Name(), "r"))
		OnProcessSMr(node);
	else if (!mir_strcmp(node->Name(), "a"))
		OnProcessSMa(node);
	else {
		m_nLocalHCount++;
		return false;
	}
	return true;
}

void strm_mgmt::EnableStrmMgmt()
{
	if (m_bEnabled)
		return;
	m_bHalfEnabled = true;

	if (m_sResumeId.empty()) {
		XmlNode enable_sm("enable");
		XmlAddAttr(enable_sm, "xmlns", "urn:xmpp:sm:3");
		XmlAddAttr(enable_sm, "resume", "true"); // enable resumption (most useful part of this xep)
		proto->m_ThreadInfo->send(enable_sm);
		m_nLocalSCount = 0;
	}
	else { // resume session
		XmlNode enable_sm("resume");
		enable_sm << XATTR("xmlns", "urn:xmpp:sm:3") << XATTRI("h", m_nLocalHCount) << XATTR("previd", m_sResumeId.c_str());
		proto->m_ThreadInfo->send(enable_sm);
	}
}

void strm_mgmt::SendAck()
{
	if (!m_bEnabled)
		return;

	proto->debugLogA("strm_mgmt: info: sending ack: locally received node count %d", m_nLocalHCount);
	XmlNode ack_node("a");
	ack_node << XATTR("xmlns", "urn:xmpp:sm:3") << XATTRI("h", m_nLocalHCount);
	proto->m_ThreadInfo->send_no_strm_mgmt(ack_node);
}

void strm_mgmt::RequestAck()
{
	if (!m_bEnabled)
		return;

	if (!m_bRequestPending) {
		// We should save m_nLocalSCount here bc the server acknowlages stanza count for the moment when it receives <r>
		// NOT for the moment it sends <a>
		m_bRequestPending = true;
		m_nReqLocalSCount = m_nLocalSCount;
		XmlNode req_node("r"); req_node << XATTR("xmlns", "urn:xmpp:sm:3");
		proto->m_ThreadInfo->send_no_strm_mgmt(req_node);
	}
}

bool strm_mgmt::IsSessionResumed()
{
	return m_bSessionResumed;
}

bool strm_mgmt::IsResumeIdPresent()
{
	if (m_tConnLostTime && m_nResumeMaxSeconds && time(0) - m_tConnLostTime > m_nResumeMaxSeconds) {
		ResetState();
		return false;
	}
	return !m_sResumeId.empty();
}

void strm_mgmt::FinishLoginProcess(ThreadData *info)
{
	if (proto->m_arAuthMechs.getCount()) { //We are already logged-in
		info->send(
			XmlNodeIq(proto->AddIQ(&CJabberProto::OnIqResultBind, JABBER_IQ_TYPE_SET))
			<< XCHILDNS("bind", JABBER_FEAT_BIND)
			<< XCHILD("resource", info->resource));
		return;
	}

	// mechanisms not available and we are not logged in
	proto->PerformIqAuth(info);
}
