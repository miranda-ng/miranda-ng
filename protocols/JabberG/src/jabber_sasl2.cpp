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

void Hi(const EVP_MD *hashMethod, uint8_t *res, char *passw, size_t passwLen, char *salt, size_t saltLen, int iterations);

struct TScramTask : public TUpgradeTask
{
	const EVP_MD *hashMethod;

	TScramTask(ThreadData *info, const char *pszMech, const EVP_MD *pMethod, int iPriority) :
		TUpgradeTask(info, pszMech),
		hashMethod(pMethod)
	{
		priority = iPriority;
	}

	~TScramTask() {}

	bool perform(const TiXmlElement *src, TiXmlElement *dest) override
	{
		auto *salt = XmlGetChildByTag(src, "salt", "xmlns", "urn:xmpp:scram-upgrade:0");
		if (!salt || !mir_strlen(szInitData))
			return false;

		int iterations = salt->IntAttribute("iterations");
		auto *pszSalt = salt->GetText();
		if (!mir_strlen(pszSalt) || !iterations)
			return false;

		size_t cbNonce, cbSalt;
		ptrA szInit((char *)mir_base64_decode(szInitData, &cbNonce));
		ptrA szNonce((char*)mir_base64_decode(szInit.get() + 2, &cbNonce));
		ptrA szSalt((char *)mir_base64_decode(pszSalt, &cbSalt));
		ptrA cbd(mir_base64_encode("n,,", 3)), chl(mir_strdup("")), msg1(mir_strdup(""));

		int hashSize = EVP_MD_size(hashMethod);

		uint8_t saltedPassw[EVP_MAX_MD_SIZE];
		Hi(hashMethod, saltedPassw, info->conn.password, mir_strlen(info->conn.password), szSalt, cbSalt, iterations);

		uint8_t clientKey[EVP_MAX_MD_SIZE];
		unsigned int len;
		HMAC(hashMethod, saltedPassw, hashSize, (uint8_t *)"Client Key", 10, clientKey, &len);

		uint8_t storedKey[EVP_MAX_MD_SIZE];
		{
			EVP_MD_CTX *pctx = EVP_MD_CTX_new();
			EVP_DigestInit(pctx, hashMethod);
			EVP_DigestUpdate(pctx, clientKey, hashSize);
			EVP_DigestFinal(pctx, storedKey, &len);
			EVP_MD_CTX_free(pctx);
		}

		uint8_t clientSig[EVP_MAX_MD_SIZE];
		CMStringA authmsg(FORMAT, "%s,%s,c=%s,r=%s", msg1, chl.get(), cbd.get(), szNonce.get());
		HMAC(hashMethod, storedKey, hashSize, (uint8_t *)authmsg.c_str(), authmsg.GetLength(), clientSig, &len);

		uint8_t clientProof[EVP_MAX_MD_SIZE];
		for (int j = 0; j < hashSize; j++)
			clientProof[j] = clientKey[j] ^ clientSig[j];

		ptrA szEncoded(mir_base64_encode(clientProof, hashSize));
		auto *pHash = dest << XCHILD("hash", szEncoded);
		pHash << XATTR("xmlns", "urn:xmpp:scram-upgrade:0");
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// SASL2: common tasks processing methods

void CJabberProto::OnProcessUpgrade(const TiXmlElement *n, ThreadData *info)
{
	TUpgradeTask *pTask;
	auto *szMechanism = n->GetText();
	if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-1"))
		pTask = new TScramTask(info, szMechanism, EVP_sha1(), 500);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-256"))
		pTask = new TScramTask(info, szMechanism, EVP_sha256(), 520);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-384"))
		pTask = new TScramTask(info, szMechanism, EVP_sha384(), 530);
	else if (!mir_strcmp(szMechanism, "UPGR-SCRAM-SHA-512"))
		pTask = new TScramTask(info, szMechanism, EVP_sha512(), 540);
	else {
		debugLogA("Unsupported mechanism for upgrade: %s, skipping", szMechanism);
		return;
	}

	m_arSaslUpgrade.insert(pTask);
}

void CJabberProto::OnProcessContinue(const TiXmlElement *node, ThreadData *info)
{
	if (!node->Attribute("xmlns", JABBER_FEAT_SASL2)) {
		debugLogA("Missing xmlns for continue, ignoring");
		return;
	}

	TUpgradeTask *pTask = nullptr;
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

	pTask->setInitData(XmlGetChildText(node, "additional-data"));
	info->m_saslUpgrade = pTask;

	XmlNode next("next");
	next << XATTR("xmlns", JABBER_FEAT_SASL2) << XATTR("task", pTask->getName());
	info->send(next);
}

void CJabberProto::OnProcessTaskData(const TiXmlElement *node, ThreadData *info)
{
	if (!info->m_saslUpgrade)
		return;

	XmlNode reply("task-data");
	reply << XATTR("xmlns", JABBER_FEAT_SASL2);
	if (info->m_saslUpgrade->perform(node, reply))
		info->send(reply);
	else
		info->send("</stream:stream>"); // bye-bye
}
