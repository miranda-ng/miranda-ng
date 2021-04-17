/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019 George Hazan

*/

#include "stdafx.h"

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
	setString(hContact, DBKEY_ID, szId);
	pUser = new WAUser(hContact, mir_strdup(szId));
	if (bTemporary)
		Contact_RemoveFromList(hContact);
	if (m_wszDefaultGroup)
		Clist_SetGroup(hContact, m_wszDefaultGroup);

	mir_cslock lck(m_csUsers);
	m_arUsers.insert(pUser);
	return pUser;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool WhatsAppProto::decryptBinaryMessage(size_t cbSize, const void *buf, MBinBuffer &res)
{
	if (cbSize <= 32)
		return false;

	// validate message first
	{
		unsigned int md_len = 32;
		BYTE md[32];
		HMAC(EVP_sha256(), mac_key.data(), (int)mac_key.length(), (unsigned char *)buf+32, (int)cbSize-32, md, &md_len);
		if (memcmp(buf, md, sizeof(md))) {
			debugLogA("Message cannot be decrypted, check your keys");
			return false;
		}
	}
	
	// okay, let's decrypt this thing
	{
		BYTE iv[16];
		memcpy(iv, (char*)buf + 32, sizeof(iv));
		res.assign((char*)buf + 48, cbSize - 48);
		res.append((char*)mac_key.data(), 32); // reserve 32 more bytes for temp data

		int dec_len = 0, final_len = 0;
		EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (BYTE*)enc_key.data(), iv);
		EVP_DecryptUpdate(ctx, (BYTE*)res.data(), &dec_len, (BYTE*)buf + 48, (int)cbSize - 48);
		EVP_DecryptFinal_ex(ctx, (BYTE*)res.data() + dec_len, &final_len);
		EVP_CIPHER_CTX_free(ctx);
	}
	return true;
}
