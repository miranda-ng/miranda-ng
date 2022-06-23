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

#include "stdafx.h"
#include "jabber_strm_mgmt.h"

strm_mgmt::strm_mgmt(CJabberProto *_proto) :
	proto(_proto)
{
}

void strm_mgmt::OnProcessEnabled(const TiXmlElement *node, ThreadData * /*info*/)
{
	m_bStrmMgmtEnabled = true;
	auto *val = XmlGetAttr(node, "max");
	if (val)
		m_nStrmMgmtResumeMaxSeconds = atoi(val);
	val = XmlGetAttr(node, "resume");
	if (val) {
		if (mir_strcmp(val, "true") || mir_strcmp(val, "1")) {
			m_bStrmMgmtResumeSupported = true;
			m_sStrmMgmtResumeId = XmlGetAttr(node, "id");
		}
	}
	//TODO: handle 'location'
	m_nStrmMgmtLocalHCount = 0;
	m_nStrmMgmtSrvHCount = 0;
}

void strm_mgmt::OnProcessResumed(const TiXmlElement *node, ThreadData * /*info*/)
{
	if (mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		return;
	
	auto *var = XmlGetAttr(node, "previd");
	if (!var)
		return;
	
	if (m_sStrmMgmtResumeId != var)
		return; //TODO: unknown session, what we should do ?
	
	var = XmlGetAttr(node, "h");
	if (!var)
		return;
	bSessionResumed = true;
	m_bStrmMgmtEnabled = true;
	m_bStrmMgmtPendingEnable = false;
	m_nStrmMgmtSrvHCount = atoi(var);
	int size = m_nStrmMgmtLocalSCount - m_nStrmMgmtSrvHCount;

	//FinishLoginProcess(info);
	proto->OnLoggedIn();
	proto->ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)proto->m_iStatus, proto->m_iDesiredStatus);

	if (size < 0) {
		proto->debugLogA("strm_mgmt: error: locally sent nodes count %d, server side received count %d", m_nStrmMgmtLocalSCount, m_nStrmMgmtSrvHCount);
		m_nStrmMgmtLocalSCount = m_nStrmMgmtSrvHCount; //temporary workaround
		//TODO: this should never happen, indicates server side bug
		//TODO: once our client side implementation good enough, abort stream in this case, noop for now
	}
	else if (size > 0 && !NodeCache.empty()) //TODO: NodeCache cannot be empty if size >0, it's a bug
		ResendNodes(size);
	else {
		for (auto i : NodeCache)
			xmlStorage.DeleteNode(i);
		NodeCache.clear();
	}
}

void strm_mgmt::OnProcessSMa(const TiXmlElement *node)
{
	if (mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		return;

	m_nStrmMgmtSrvHCount = node->IntAttribute("h");
	proto->debugLogA("strm_mgmt: info: locally sent nodes count %d, server side received count %d", m_nStrmMgmtLocalSCount, m_nStrmMgmtSrvHCount);
	int size = m_nStrmMgmtLocalSCount - m_nStrmMgmtSrvHCount;
	if (size < 0) {
		proto->debugLogA("strm_mgmt: error: locally sent nodes count %d, server side received count %d", m_nStrmMgmtLocalSCount, m_nStrmMgmtSrvHCount);
		m_nStrmMgmtLocalSCount = m_nStrmMgmtSrvHCount; //temporary workaround
		//TODO: this should never happen, indicates server side bug
		//TODO: once our client side implementation good enough, abort stream in this case, noop for now
	}
	else if (size > 0 && !NodeCache.empty()) //TODO: NodeCache cannot be empty if size >0, it's a bug
		ResendNodes(size);
	else {
		for (auto i : NodeCache)
			xmlStorage.DeleteNode(i);
		NodeCache.clear();
	}
}

void strm_mgmt::ResendNodes(uint32_t size)
{
	proto->debugLogA("strm_mgmt: info: resending  %d missed nodes", size);
	if (size < NodeCache.size()) {
		proto->debugLogA("strm_mgmt: info: resending nodes: need to resend %d nodes, nodes in cache %d, cleaning cache to match resending node count", size, NodeCache.size());
		const size_t diff = NodeCache.size() - size;
		if (diff) {
			size_t diff_tmp = diff;
			for (auto i : NodeCache) {
				if (diff_tmp > 0) {
					xmlStorage.DeleteNode(i);
					diff_tmp--;
				}
			}
			diff_tmp = diff;
			while (diff_tmp) {
				NodeCache.pop_front();
				diff_tmp--;
			}
		}
	}
	std::list<TiXmlElement*> tmp_list = NodeCache;
	NodeCache.clear();
	m_nStrmMgmtLocalSCount = m_nStrmMgmtSrvHCount; //we have handled missed nodes, set our counter to match server side value
	for (auto i : tmp_list)
		proto->m_ThreadInfo->send(i);
}

void strm_mgmt::OnProcessSMr(const TiXmlElement *node)
{
	if (!mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		SendAck();
}

void strm_mgmt::OnProcessFailed(const TiXmlElement *node, ThreadData * info) //used failed instead of failure, notes: https://xmpp.org/extensions/xep-0198.html#errors
{
	if (mir_strcmp(XmlGetAttr(node, "xmlns"), "urn:xmpp:sm:3"))
		return;
	
	proto->debugLogA("strm_mgmt: error: Failed to resume session %s", m_sStrmMgmtResumeId.c_str());

	m_bStrmMgmtEnabled = false;
	bSessionResumed = false;
	m_sStrmMgmtResumeId.clear();

	//resume failed, reset contacts status
	for (auto &hContact : proto->AccContacts())
		proto->SetContactOfflineStatus(hContact);

	auto *subnode = XmlFirstChild(node, "item-not-found");
	if (subnode) {
		m_bStrmMgmtPendingEnable = true;
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
		m_bStrmMgmtPendingEnable = true;
	else
		EnableStrmMgmt();
}

void strm_mgmt::CheckState()
{
	if (m_bStrmMgmtEnabled)
		return;
	if (!m_bStrmMgmtPendingEnable)
		return;
	EnableStrmMgmt();
}

void strm_mgmt::HandleOutgoingNode(TiXmlElement *node)
{
	if (!m_bStrmMgmtEnabled)
		return;
	
	auto *pNodeCopy = node->DeepClone(&xmlStorage)->ToElement();
	if (pNodeCopy == nullptr)
		return;

	m_nStrmMgmtLocalSCount++;
	NodeCache.push_back(pNodeCopy);
	if ((m_nStrmMgmtLocalSCount - m_nStrmMgmtSrvHCount) >= m_nStrmMgmtCacheSize
		|| m_nStrmMgmtLocalSCount % 3 == 0)
		RequestAck();
}

void strm_mgmt::ResetState()
{
	//reset state of stream management
	m_bStrmMgmtEnabled = false;
	m_bStrmMgmtPendingEnable = false;
	//reset stream management h counters
	m_nStrmMgmtLocalHCount = m_nStrmMgmtLocalSCount = m_nStrmMgmtSrvHCount = 0;
	//clear resume id
	m_sStrmMgmtResumeId.clear();
}

bool strm_mgmt::HandleIncommingNode(const TiXmlElement *node)
{
	if (!m_bStrmMgmtEnabled)
		return false;

	if (!mir_strcmp(node->Name(), "r"))
		OnProcessSMr(node);
	else if (!mir_strcmp(node->Name(), "a"))
		OnProcessSMa(node);
	else {
		m_nStrmMgmtLocalHCount++;
		return false;
	}
	return true;
}

void strm_mgmt::EnableStrmMgmt()
{
	if (m_bStrmMgmtEnabled)
		return;
	if (m_sStrmMgmtResumeId.empty()) {
		XmlNode enable_sm("enable");
		XmlAddAttr(enable_sm, "xmlns", "urn:xmpp:sm:3");
		XmlAddAttr(enable_sm, "resume", "true"); //enable resumption (most useful part of this xep)
		proto->m_ThreadInfo->send(enable_sm);
		m_nStrmMgmtLocalSCount = 1; //TODO: this MUST be 0, i have bug somewhere, feel free to fix it.
	}
	else //resume session
	{
		XmlNode enable_sm("resume");
		enable_sm << XATTR("xmlns", "urn:xmpp:sm:3") <<	XATTRI("h", m_nStrmMgmtLocalHCount) <<	XATTR("previd", m_sStrmMgmtResumeId.c_str());
		proto->m_ThreadInfo->send(enable_sm);
	}
}

void strm_mgmt::SendAck()
{
	if (!m_bStrmMgmtEnabled)
		return;
	proto->debugLogA("strm_mgmt: info: sending ack: locally received node count %d", m_nStrmMgmtLocalHCount);
	XmlNode enable_sm("a");
	enable_sm << XATTR("xmlns", "urn:xmpp:sm:3") << XATTRI("h", m_nStrmMgmtLocalHCount);
	proto->m_ThreadInfo->send_no_strm_mgmt(enable_sm);
}

void strm_mgmt::RequestAck()
{
	if (!m_bStrmMgmtEnabled)
		return;

	XmlNode enable_sm("r"); enable_sm << XATTR("xmlns", "urn:xmpp:sm:3");
	proto->m_ThreadInfo->send_no_strm_mgmt(enable_sm);
}

bool strm_mgmt::IsSessionResumed()
{
	return bSessionResumed;
}

bool strm_mgmt::IsResumeIdPresent()
{
	return !m_sStrmMgmtResumeId.empty();
}

void strm_mgmt::FinishLoginProcess(ThreadData *info)
{
	if (proto->m_arAuthMechs.getCount()) { //We are already logged-in
		info->send(
			XmlNodeIq(proto->AddIQ(&CJabberProto::OnIqResultBind, JABBER_IQ_TYPE_SET))
			<< XCHILDNS("bind", JABBER_FEAT_BIND)
			<< XCHILD("resource", info->resource));

		if (proto->m_isSessionAvailable)
			info->bIsSessionAvailable = true;

		return;
	}

	// mechanisms not available and we are not logged in
	proto->PerformIqAuth(info);
}
