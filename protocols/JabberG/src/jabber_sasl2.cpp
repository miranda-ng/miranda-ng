/*
Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void CJabberProto::OnProcessUpgrade(const TiXmlElement *n, ThreadData *info)
{
	/*
	TJabberAuth *pAuth = nullptr;
	auto *szMechanism = n->GetText();
	if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-1"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha1(), 500);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-1-PLUS"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha1(), 601);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-224"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha224(), 510);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-224-PLUS"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha224(), 611);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-256"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha256(), 520);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-256-PLUS"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha256(), 621);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-384"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha384(), 530);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-384-PLUS"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha384(), 631);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-512"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha512(), 540);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-512-PLUS"))
		pAuth = new TScramAuth(info, szMechanism, EVP_sha512(), 641);

	if (pAuth == nullptr) {
		debugLogA("Unsupported mechanism for upgrade: %s, skipping", szMechanism);
		return;
	}

	if (!pAuth->isValid())
		delete pAuth;
	else
		m_arSaslUpgrade.insert(pAuth);*/
}

void CJabberProto::OnProcessContinue(const TiXmlElement *node, ThreadData *info)
{
	if (!node->Attribute("xmlns", JABBER_FEAT_SASL2)) {
		debugLogA("Missing xmlns for continue, ignoring");
		return;
	}

	TJabberAuth *pTask = nullptr;
	for (auto *task : TiXmlFilter(node->FirstChildElement("tasks"), "task"))
		for (auto &it : m_arSaslUpgrade)
			if (!mir_strcmp(it->getName(), task->GetText())) {
				pTask = it;
				break;
			}

	if (!pTask) {
		debugLogA("Unsupported task type, ignoring");
		info->send("</stream:stream>"); // bye-bye
		return;
	}

	info->m_saslUpgrade = pTask;
	if (auto *n = node->FirstChildElement("additional-data"))
		info->saslInitData = mir_strdup(n->GetText());

	XmlNode next("next");
	next << XATTR("xmlns", JABBER_FEAT_SASL2) << XATTR("task", pTask->getName());
	info->send(next);
}

void CJabberProto::OnProcessTaskData(const TiXmlElement *node, ThreadData *info)
{

}
