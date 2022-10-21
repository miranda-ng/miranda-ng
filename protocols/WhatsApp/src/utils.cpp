/*

WhatsApp plugin for Miranda NG
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

bool WAJid::isUser() const
{	return server == "s.whatsapp.net";
}

bool WAJid::isGroup() const
{	return server == "g.us";
}

bool WAJid::isBroadcast() const
{
	return server == "broadcast";
}

bool WAJid::isStatusBroadcast() const
{
	return isBroadcast() && user == "status";
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

static uint8_t sttLtHashInfo[] = "WhatsApp Patch Integrity";

void LT_HASH::add(const void *pData, size_t len)
{
	uint16_t tmp[_countof(hash)];
	HKDF(EVP_sha256(), (BYTE *)"", 0, (BYTE*)pData, len, sttLtHashInfo, sizeof(sttLtHashInfo) - 1, (BYTE *)tmp, sizeof(tmp));

	for (int i = 0; i < _countof(hash); i++)
		hash[i] += tmp[i];
}

void LT_HASH::sub(const void *pData, size_t len)
{
	uint16_t tmp[_countof(hash)];
	HKDF(EVP_sha256(), (BYTE *)"", 0, (BYTE *)pData, len, sttLtHashInfo, sizeof(sttLtHashInfo) - 1, (BYTE *)tmp, sizeof(tmp));

	for (int i = 0; i < _countof(hash); i++)
		hash[i] -= tmp[i];
}

/////////////////////////////////////////////////////////////////////////////////////////

WAUser* WhatsAppProto::FindUser(const char *szId)
{
	mir_cslock lck(m_csUsers);
	auto *tmp = (WAUser *)_alloca(sizeof(WAUser));
	tmp->szId = (char*)szId;
	return m_arUsers.find(tmp);
}

WAUser* WhatsAppProto::AddUser(const char *szId, bool bTemporary, bool isChat)
{
	auto *pUser = FindUser(szId);
	if (pUser != nullptr)
		return pUser;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	if (isChat) {
		setByte(hContact, "ChatRoom", 1);
		setString(hContact, "ChatRoomID", szId);
	}
	else {
		setString(hContact, DBKEY_ID, szId);
		if (m_wszDefaultGroup)
			Clist_SetGroup(hContact, m_wszDefaultGroup);
	}

	if (bTemporary)
		Contact::RemoveFromList(hContact);

	pUser = new WAUser(hContact, mir_strdup(szId));
	pUser->bIsGroupChat = isChat;

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

CMStringA WhatsAppProto::GenerateMessageId()
{
	return CMStringA(FORMAT, "%d.%d-%d", m_wMsgPrefix[0], m_wMsgPrefix[1], m_iPacketId++);
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

	if (pHandler != nullptr) {
		CMStringA id(GenerateMessageId());
		node.addAttr("id", id);

		mir_cslock lck(m_csPacketQueue);
		m_arPacketQueue.insert(new WARequest(id, pHandler));
	}

	CMStringA szText;
	node.print(szText);
	debugLogA("Sending binary node:\n%s", szText.c_str());

	WAWriter writer;
	writer.writeNode(&node);

	MBinBuffer encData = m_noise->encrypt(writer.body.data(), writer.body.length());
	MBinBuffer payload = m_noise->encodeFrame(encData.data(), encData.length());
	WebSocket_SendBinary(m_hServerConn, payload.data(), payload.length());
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

std::string decodeBinStr(const std::string &buf)
{
	size_t cbLen;
	void *pData = mir_base64_decode(buf.c_str(), &cbLen);
	if (pData == nullptr)
		return "";

	std::string res((char *)pData, cbLen);
	mir_free(pData);
	return res;
}

uint32_t decodeBigEndian(const std::string &buf)
{
	uint32_t ret = 0;
	for (auto &cc : buf) {
		ret <<= 8;
		ret += (uint8_t)cc;
	}

	return ret;
}

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
// Popups

void WhatsAppProto::InitPopups(void)
{
	g_plugin.addPopupOption(CMStringW(FORMAT, TranslateT("%s error notifications"), m_tszUserName), m_bUsePopups);

	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	wchar_t desc[256];
	mir_snwprintf(desc, L"%s/%s", m_tszUserName, TranslateT("Errors"));

	POPUPCLASS ppc = {};
	ppc.flags = PCF_UNICODE;
	ppc.pszName = name;
	ppc.pszDescription.w = desc;
	ppc.hIcon = IcoLib_GetIconByHandle(m_hProtoIcon);
	ppc.colorBack = RGB(191, 0, 0); //Red
	ppc.colorText = RGB(255, 245, 225); //Yellow
	ppc.iSeconds = 60;
	m_hPopupClass = Popup_RegisterClass(&ppc);

	IcoLib_ReleaseIcon(ppc.hIcon);
}

void WhatsAppProto::Popup(MCONTACT hContact, const wchar_t *szMsg, const wchar_t *szTitle)
{
	if (!m_bUsePopups)
		return;

	char name[256];
	mir_snprintf(name, "%s_%s", m_szModuleName, "Error");

	CMStringW wszTitle(szTitle);
	if (hContact == 0) {
		wszTitle.Insert(0, L": ");
		wszTitle.Insert(0, m_tszUserName);
	}

	POPUPDATACLASS ppd = {};
	ppd.szTitle.w = wszTitle;
	ppd.szText.w = szMsg;
	ppd.pszClassName = name;
	ppd.hContact = hContact;
	Popup_AddClass(&ppd);
}

/////////////////////////////////////////////////////////////////////////////////////////

MBinBuffer WhatsAppProto::unzip(const MBinBuffer &src)
{
	z_stream strm = {};
	inflateInit(&strm);

	strm.avail_in = (uInt)src.length();
	strm.next_in = (Bytef *)src.data();

	MBinBuffer res;
	Bytef buf[2048];

	while (strm.avail_in > 0) {
		strm.avail_out = sizeof(buf);
		strm.next_out = buf;

		int ret = inflate(&strm, Z_NO_FLUSH);
		switch (ret) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;
			__fallthrough;

		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&strm);
			return res;
		}

		res.append(buf, sizeof(buf) - strm.avail_out);
	}

	inflateEnd(&strm);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

void bin2file(const MBinBuffer &buf, const wchar_t *pwszFileName)
{
	int fileId = _wopen(pwszFileName, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
	if (fileId != -1) {
		write(fileId, buf.data(), (unsigned)buf.length());
		close(fileId);
	}
}

void string2file(const std::string &str, const wchar_t *pwszFileName)
{
	int fileId = _wopen(pwszFileName, _O_WRONLY | _O_TRUNC | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);
	if (fileId != -1) {
		write(fileId, str.c_str(), (unsigned)str.size());
		close(fileId);
	}
}

CMStringA file2string(const wchar_t *pwszFileName)
{
	CMStringA res;
		
	int fileId = _wopen(pwszFileName, _O_RDONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
	if (fileId != -1) {
		res.Truncate(filelength(fileId));
		read(fileId, res.GetBuffer(), res.GetLength());
		close(fileId);
	}
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringA directPath2url(const char *pszDirectPath)
{
	return CMStringA("https://mmg.whatsapp.net") + pszDirectPath;
}

MBinBuffer WhatsAppProto::DownloadEncryptedFile(const char *url, const std::string &mediaKeys, const char *pszMediaType)
{
	NETLIBHTTPHEADER headers[1] = {{"Origin", "https://web.whatsapp.com"}};

	NETLIBHTTPREQUEST req = {};
	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	req.szUrl = (char*)url;
	req.headersCount = _countof(headers);
	req.headers = headers;

	MBinBuffer ret;
	auto *pResp = Netlib_HttpTransaction(m_hNetlibUser, &req);
	if (pResp) {
		if (pResp->resultCode == 200) {
			CMStringA pszHkdfString = pszMediaType;
			pszHkdfString.SetAt(_toupper(pszHkdfString[0]), 0);
			pszHkdfString = "WhatsApp " + pszHkdfString + " Keys";

			// 0 - 15: iv
			// 16 - 47: cipherKey
			// 48 - 111: macKey
			uint8_t out[112];
			HKDF(EVP_sha256(), (BYTE *)"", 0, (BYTE *)mediaKeys.c_str(), (int)mediaKeys.size(), (BYTE *)pszHkdfString.c_str(), pszHkdfString.GetLength(), out, sizeof(out));

			ret = aesDecrypt(EVP_aes_256_cbc(), out + 16, out, pResp->pData, pResp->dataLength);
		}
	}

	return ret;
}

CMStringW WhatsAppProto::GetTmpFileName(const char *pszClass, const char *pszAddition)
{
	CMStringW ret(VARSW(L"%miranda_userdata%"));
	ret.AppendFormat(L"/%S/%S_%S", m_szModuleName, pszClass, pszAddition);
	return ret;
}
