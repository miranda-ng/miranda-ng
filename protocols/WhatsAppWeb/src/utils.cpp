/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

WAJid::WAJid(const char *pszUser, const char *pszServer, int iDevice, int iAgent) :
	user(pszUser ? pszUser : ""),
	server(pszServer ? pszServer : ""),
	device(iDevice),
	agent(iAgent)
{}

WAJid::WAJid(const char *pszJid)
{
	if (pszJid == nullptr)
		pszJid = "";

	auto *tmp = NEWSTR_ALLOCA(pszJid);
	auto *p = strrchr(tmp, '@');
	if (p) {
		*p = 0;
		server = p + 1;
	}

	if (p = strrchr(tmp, ':')) {
		*p = 0;
		device = atoi(p + 1);
	}
	else device = 0;

	if (p = strrchr(tmp, '_')) {
		*p = 0;
		agent = atoi(p + 1);
	}
	else agent = 0;

	user = tmp;
}

CMStringA WAJid::toString() const
{
	CMStringA ret(user);
	if (agent > 0)
		ret.AppendFormat("_%d", agent);
	if (device > 0)
		ret.AppendFormat(":%d", device);
	ret.AppendFormat("@%s", server.c_str());
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

WAUser* WhatsAppProto::FindUser(const char *szId)
{
	mir_cslock lck(m_csUsers);
	auto *tmp = (WAUser *)_alloca(sizeof(WAUser));
	tmp->szId = (char*)szId;
	return m_arUsers.find(tmp);
}

WAUser* WhatsAppProto::AddUser(const char *szId, bool bTemporary)
{
	auto *pUser = FindUser(szId);
	if (pUser != nullptr)
		return pUser;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);
	setString(hContact, DBKEY_JID, szId);
	pUser = new WAUser(hContact, mir_strdup(szId));
	if (bTemporary)
		Contact::RemoveFromList(hContact);
	if (m_wszDefaultGroup)
		Clist_SetGroup(hContact, m_wszDefaultGroup);

	mir_cslock lck(m_csUsers);
	m_arUsers.insert(pUser);
	return pUser;
}

/////////////////////////////////////////////////////////////////////////////////////////

WA_PKT_HANDLER WhatsAppProto::FindPersistentHandler(const WANode &pNode)
{
	auto *pChild = pNode.getFirstChild();
	CMStringA szChild = (pChild) ? pChild->title : "";
	CMStringA szTitle = pNode.title;
	CMStringA szType = pNode.getAttr("type");
	CMStringA szXmlns = pNode.getAttr("xmlns");

	for (auto &it : m_arPersistent) {
		if (it->pszTitle && szTitle != it->pszTitle)
			continue;
		if (it->pszType && szType != it->pszType)
			continue;
		if (it->pszXmlns && szXmlns != it->pszXmlns)
			continue;
		if (it->pszChild && szChild != it->pszChild)
			continue;
		return it->pHandler;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringA WhatsAppProto::generateMessageId()
{
	return CMStringA(FORMAT, "%d.%d-%d", m_wMsgPrefix[0], m_wMsgPrefix[1], m_iPacketId++);
}

/////////////////////////////////////////////////////////////////////////////////////////

MBinBuffer WhatsAppProto::getBlob(const char *szSetting)
{
	MBinBuffer buf;
	DBVARIANT dbv = { DBVT_BLOB };
	if (!db_get(0, m_szModuleName, szSetting, &dbv)) {
		buf.assign(dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
	}
	return buf;
}

/////////////////////////////////////////////////////////////////////////////////////////
// sends a piece of JSON to a server via a websocket, masked

int WhatsAppProto::WSSend(const MessageLite &msg)
{
	if (m_hServerConn == nullptr)
		return -1;

	int cbLen = msg.ByteSize();
	ptrA protoBuf((char *)mir_alloc(cbLen));
	msg.SerializeToArray(protoBuf, cbLen);

	Netlib_Dump(m_hServerConn, protoBuf, cbLen, true, 0);

	MBinBuffer payload = m_noise->encodeFrame(protoBuf, cbLen);
	WebSocket_SendBinary(m_hServerConn, payload.data(), payload.length());
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static char zeroData[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int WhatsAppProto::WSSendNode(WANode &node, WA_PKT_HANDLER pHandler)
{
	if (m_hServerConn == nullptr)
		return 0;

	CMStringA szText;
	node.print(szText);
	debugLogA("Sending binary node: %s", szText.c_str());

	WAWriter writer;
	writer.writeNode(&node);

	if (pHandler != nullptr) {
		mir_cslock lck(m_csPacketQueue);
		m_arPacketQueue.insert(new WARequest(node.getAttr("id"), pHandler));
	}

	MBinBuffer encData = m_noise->encrypt(writer.body.data(), writer.body.length());
	MBinBuffer payload = m_noise->encodeFrame(encData.data(), encData.length());
	WebSocket_SendBinary(m_hServerConn, payload.data(), payload.length());
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

std::string encodeBigEndian(uint32_t num, size_t len)
{
	std::string res;
	for (int i = 0; i < len; i++) {
		char c = num & 0xFF;
		res = c + res;
		num >>= 8;
	}
	return res;
}

void generateIV(uint8_t *iv, int &pVar)
{
	auto counter = encodeBigEndian(pVar);
	memset(iv, 0, sizeof(iv));
	memcpy(iv + 8, counter.c_str(), sizeof(int));
	
	pVar++;
}

/////////////////////////////////////////////////////////////////////////////////////////

static unsigned char *HKDF_Extract(const EVP_MD *evp_md,
	const unsigned char *salt, size_t salt_len,
	const unsigned char *key, size_t key_len,
	unsigned char *prk, size_t *prk_len)
{
	unsigned int tmp_len;

	if (!HMAC(evp_md, salt, (int)salt_len, key, (int)key_len, prk, &tmp_len))
		return NULL;

	*prk_len = tmp_len;
	return prk;
}

static unsigned char *HKDF_Expand(const EVP_MD *evp_md,
	const unsigned char *prk, size_t prk_len,
	const unsigned char *info, size_t info_len,
	unsigned char *okm, size_t okm_len)
{
	HMAC_CTX *hmac;
	unsigned char *ret = NULL;

	unsigned int i;

	unsigned char prev[EVP_MAX_MD_SIZE];

	size_t done_len = 0, dig_len = EVP_MD_size(evp_md);

	size_t n = okm_len / dig_len;
	if (okm_len % dig_len)
		n++;

	if (n > 255 || okm == NULL)
		return NULL;

	if ((hmac = HMAC_CTX_new()) == NULL)
		return NULL;

	if (!HMAC_Init_ex(hmac, prk, (int)prk_len, evp_md, NULL))
		goto err;

	for (i = 1; i <= n; i++) {
		size_t copy_len;
		const unsigned char ctr = i;

		if (i > 1) {
			if (!HMAC_Init_ex(hmac, NULL, 0, NULL, NULL))
				goto err;

			if (!HMAC_Update(hmac, prev, dig_len))
				goto err;
		}

		if (!HMAC_Update(hmac, info, info_len))
			goto err;

		if (!HMAC_Update(hmac, &ctr, 1))
			goto err;

		if (!HMAC_Final(hmac, prev, NULL))
			goto err;

		copy_len = (done_len + dig_len > okm_len) ?
			okm_len - done_len :
			dig_len;

		memcpy(okm + done_len, prev, copy_len);

		done_len += copy_len;
	}
	ret = okm;

err:
	OPENSSL_cleanse(prev, sizeof(prev));
	HMAC_CTX_free(hmac);
	return ret;
}

unsigned char* HKDF(const EVP_MD *evp_md,
	const unsigned char *salt, size_t salt_len,
	const unsigned char *key, size_t key_len,
	const unsigned char *info, size_t info_len,
	unsigned char *okm, size_t okm_len)
{
	unsigned char prk[EVP_MAX_MD_SIZE];
	unsigned char *ret;
	size_t prk_len;

	if (!HKDF_Extract(evp_md, salt, salt_len, key, key_len, prk, &prk_len))
		return NULL;

	ret = HKDF_Expand(evp_md, prk, prk_len, info, info_len, okm, okm_len);
	OPENSSL_cleanse(prk, sizeof(prk));

	return ret;
}
