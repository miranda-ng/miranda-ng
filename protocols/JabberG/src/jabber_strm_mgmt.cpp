/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2018-19 Miranda NG team

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

strm_mgmt::strm_mgmt(CJabberProto *_proto) : proto(_proto), m_bStrmMgmtPendingEnable(false),
m_bStrmMgmtEnabled(false),
m_bStrmMgmtResumeSupported(false),
bSessionResumed(false)
{

}

void strm_mgmt::OnProcessEnabled(HXML node, ThreadData * /*info*/)
{
	m_bStrmMgmtEnabled = true;
	auto val = XmlGetAttrValue(node, L"max");
	if(val)
		m_nStrmMgmtResumeMaxSeconds = _wtoi(val);
	val = XmlGetAttrValue(node, L"resume");
	if (val)
	{
		if (mir_wstrcmp(val, L"true") || mir_wstrcmp(val, L"1"))
		{
			m_bStrmMgmtResumeSupported = true;
			m_sStrmMgmtResumeId = XmlGetAttrValue(node, L"id");
		}
	}
	//TODO: handle 'location'
	m_nStrmMgmtLocalHCount = 0;
	m_nStrmMgmtSrvHCount = 0;
}

void strm_mgmt::OnProcessResumed(HXML node, ThreadData * /*info*/)
{
	if (mir_wstrcmp(XmlGetAttrValue(node, L"xmlns"), L"urn:xmpp:sm:3"))
		return;
	auto var = XmlGetAttrValue(node, L"previd");
	if (!var)
		return;
	if (m_sStrmMgmtResumeId != var)
		return; //TODO: unknown session, what we should do ?
	var = XmlGetAttrValue(node, L"h");
	if (!var)
		return;
	bSessionResumed = true;
	m_bStrmMgmtEnabled = true;
	m_bStrmMgmtPendingEnable = false;
	m_nStrmMgmtSrvHCount = _wtoi(var);
	int size = m_nStrmMgmtLocalSCount - m_nStrmMgmtSrvHCount;

	//FinishLoginProcess(info);
	proto->OnLoggedIn();
	proto->ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)proto->m_iStatus, proto->m_iDesiredStatus);

	if (size < 0)
	{
		proto->debugLogA("strm_mgmt: error: locally sent nodes count %d, server side received count %d", m_nStrmMgmtLocalSCount, m_nStrmMgmtSrvHCount);
		m_nStrmMgmtLocalSCount = m_nStrmMgmtSrvHCount; //temporary workaround
		//TODO: this should never happen, indicates server side bug
		//TODO: once our client side implementation good enough, abort stream in this case, noop for now
	}
	else if (size > 0 && !NodeCache.empty()) //TODO: NodeCache cannot be empty if size >0, it's a bug
		ResendNodes(size);
	else
	{
		for (auto i : NodeCache)
			xmlFree(i);
		NodeCache.clear();
	}
}

void strm_mgmt::OnProcessSMa(HXML node)
{
	if (mir_wstrcmp(XmlGetAttrValue(node, L"xmlns"), L"urn:xmpp:sm:3"))
		return;
	auto val = XmlGetAttrValue(node, L"h");
	m_nStrmMgmtSrvHCount = _wtoi(val);
	proto->debugLogA("strm_mgmt: info: locally sent nodes count %d, server side received count %d", m_nStrmMgmtLocalSCount, m_nStrmMgmtSrvHCount);
	int size = m_nStrmMgmtLocalSCount - m_nStrmMgmtSrvHCount;
	if (size < 0)
	{
		proto->debugLogA("strm_mgmt: error: locally sent nodes count %d, server side received count %d", m_nStrmMgmtLocalSCount, m_nStrmMgmtSrvHCount);
		m_nStrmMgmtLocalSCount = m_nStrmMgmtSrvHCount; //temporary workaround
		//TODO: this should never happen, indicates server side bug
		//TODO: once our client side implementation good enough, abort stream in this case, noop for now
	}
	else if (size > 0 && !NodeCache.empty()) //TODO: NodeCache cannot be empty if size >0, it's a bug
		ResendNodes(size);
	else
	{
		for (auto i : NodeCache)
			xmlFree(i);
		NodeCache.clear();
	}
}

void strm_mgmt::ResendNodes(uint32_t size)
{
	proto->debugLogA("strm_mgmt: info: resending  %d missed nodes", size);
	if (size < NodeCache.size())
	{
		proto->debugLogA("strm_mgmt: info: resending nodes: need to resend %d nodes, nodes in cache %d, cleaning cache to match resending node count", size, NodeCache.size());
		const size_t diff = NodeCache.size() - size;
		if (diff)
		{
			size_t diff_tmp = diff;
			for (auto i : NodeCache)
			{
				if (diff_tmp > 0)
				{
					xmlFree(i);
					diff_tmp--;
				}
			}
			diff_tmp = diff;
			while (diff_tmp)
			{
				NodeCache.pop_front();
				diff_tmp--;
			}
		}
	}
	std::list<HXML> tmp_list = NodeCache;
	NodeCache.clear();
	m_nStrmMgmtLocalSCount = m_nStrmMgmtSrvHCount; //we have handled missed nodes, set our counter to match server side value
	for (auto i : tmp_list)
	{
		proto->m_ThreadInfo->send(i);
		//proto->m_ThreadInfo->send_no_strm_mgmt(i); //freed by send ?
												   //xmlFree(i);
	}
}

void strm_mgmt::OnProcessSMr(HXML node)
{
	if (!mir_wstrcmp(XmlGetAttrValue(node, L"xmlns"), L"urn:xmpp:sm:3"))
		SendAck();
}

void strm_mgmt::OnProcessFailed(HXML node, ThreadData * info) //used failed instead of failure, notes: https://xmpp.org/extensions/xep-0198.html#errors
{
	if (mir_wstrcmp(XmlGetAttrValue(node, L"xmlns"), L"urn:xmpp:sm:3"))
		return;
	proto->debugLogW(L"strm_mgmt: error: Failed to resume session %s", m_sStrmMgmtResumeId.c_str());

	m_bStrmMgmtEnabled = false;
	bSessionResumed = false;
	m_sStrmMgmtResumeId.clear();

	//resume failed, reset contacts status
	for (auto &hContact : proto->AccContacts())
		proto->SetContactOfflineStatus(hContact);

	{
		HXML subnode = XmlGetChild(node, L"item-not-found");
		if (subnode)
		{
			m_bStrmMgmtPendingEnable = true;
			FinishLoginProcess(info);
		}
		else
			EnableStrmMgmt(); //resume failed, try to enable strm_mgmt instead
	}
}

void strm_mgmt::CheckStreamFeatures(HXML node)
{
	if (!IsResumeIdPresent())
		ResetState(); //this may be necessary to reset counters if session resume id is not set
	if (mir_wstrcmp(XmlGetName(node), L"sm") || !XmlGetAttrValue(node, L"xmlns") || mir_wstrcmp(XmlGetAttrValue(node, L"xmlns"), L"urn:xmpp:sm:3")) //we work only with version 3 or higher of sm
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

void strm_mgmt::HandleOutgoingNode(HXML node)
{
	if (!m_bStrmMgmtEnabled)
		return;
	m_nStrmMgmtLocalSCount++;
	NodeCache.push_back(xmlCopyNode(node));
	if ((m_nStrmMgmtLocalSCount - m_nStrmMgmtSrvHCount) >= m_nStrmMgmtCacheSize)
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

void strm_mgmt::HandleIncommingNode(HXML node)
{
	if (m_bStrmMgmtEnabled && mir_wstrcmp(XmlGetName(node), L"r") && mir_wstrcmp(XmlGetName(node), L"a")) //TODO: something better
		m_nStrmMgmtLocalHCount++;
	else if (!mir_wstrcmp(XmlGetName(node), L"r"))
		OnProcessSMr(node);
	else if (!mir_wstrcmp(XmlGetName(node), L"a"))
		OnProcessSMa(node);
}

void strm_mgmt::EnableStrmMgmt()
{
	if (m_bStrmMgmtEnabled)
		return;
	if (m_sStrmMgmtResumeId.empty())
	{
		XmlNode enable_sm(L"enable");
		XmlAddAttr(enable_sm, L"xmlns", L"urn:xmpp:sm:3");
		XmlAddAttr(enable_sm, L"resume", L"true"); //enable resumption (most useful part of this xep)
		proto->m_ThreadInfo->send(enable_sm);
		m_nStrmMgmtLocalSCount = 1; //TODO: this MUST be 0, i have bug somewhere, feel free to fix it.
	}
	else //resume session
	{
		XmlNode enable_sm(L"resume");
		XmlAddAttr(enable_sm, L"xmlns", L"urn:xmpp:sm:3");
		xmlAddAttrInt(enable_sm, L"h", m_nStrmMgmtLocalHCount);
		XmlAddAttr(enable_sm, L"previd", m_sStrmMgmtResumeId.c_str());
		proto->m_ThreadInfo->send(enable_sm);
	}
}

void strm_mgmt::SendAck()
{
	if (!m_bStrmMgmtEnabled)
		return;
	proto->debugLogA("strm_mgmt: info: sending ack: locally received node count %d", m_nStrmMgmtLocalHCount);
	XmlNode enable_sm(L"a");
	XmlAddAttr(enable_sm, L"xmlns", L"urn:xmpp:sm:3");
	xmlAddAttrInt(enable_sm, L"h", m_nStrmMgmtLocalHCount);
	proto->m_ThreadInfo->send_no_strm_mgmt(enable_sm);
}

void strm_mgmt::RequestAck()
{
	if (!m_bStrmMgmtEnabled)
		return;
	XmlNode enable_sm(L"r");
	XmlAddAttr(enable_sm, L"xmlns", L"urn:xmpp:sm:3");
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

	if (info->auth) 
	{ //We are already logged-in
		info->send(
			XmlNodeIq(proto->AddIQ(&CJabberProto::OnIqResultBind, JABBER_IQ_TYPE_SET))
			<< XCHILDNS(L"bind", L"urn:ietf:params:xml:ns:xmpp-bind")
			<< XCHILD(L"resource", info->resource));

		if (proto->m_AuthMechs.isSessionAvailable)
			info->bIsSessionAvailable = TRUE;

		return;
	}

	//mechanisms not available and we are not logged in
	proto->PerformIqAuth(info);

}
