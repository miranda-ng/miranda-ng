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

WAJid::WAJid(const char *pszJid, int _id)
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
	else device = _id;

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
	if (szId == nullptr)
		return nullptr;

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

	pUser = new WAUser(hContact, mir_strdup(szId));
	pUser->bIsGroupChat = WAJid(szId).isGroup();

	if (pUser->bIsGroupChat) {
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

int WhatsAppProto::WSSend(const ProtobufCMessage &msg)
{
	if (m_hServerConn == nullptr)
		return -1;

	MBinBuffer buf(proto::Serialize(&msg));
	Netlib_Dump(m_hServerConn, buf.data(), buf.length(), true, 0);

	MBinBuffer payload = m_noise->encodeFrame(buf.data(), buf.length());
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

MBinBuffer decodeBufStr(const std::string &buf)
{
	MBinBuffer res;
	size_t cbLen;
	void *pData = mir_base64_decode(buf.c_str(), &cbLen);
	if (pData == nullptr)
		return res;

	res.assign(pData, cbLen);
	mir_free(pData);
	return res;
}

uint32_t decodeBigEndian(const ProtobufCBinaryData &buf)
{
	uint32_t ret = 0;
	for (int i = 0; i < buf.len; i++) {
		ret <<= 8;
		ret += buf.data[i];
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

void generateIV(uint8_t *iv, uint32_t &pVar)
{
	auto counter = encodeBigEndian(pVar);
	memset(iv, 0, 8);
	memcpy(iv + 8, counter.c_str(), sizeof(uint32_t));
	
	pVar++;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Padding

void padBuffer16(MBinBuffer &buf)
{
	uint8_t c = buf.length() % 16;
	if (c == 0)
		c = 16;

	for (uint8_t i = 0; i < c; i++)
		buf.append(&c, 1);
}

MBinBuffer unpadBuffer16(const MBinBuffer &buf)
{
	size_t len = buf.length();
	auto p = buf.data() + len - 1;
	if (*p <= 0x10) {
		MBinBuffer res;
		res.assign(buf.data(), len - *p);
		return res;
	}

	return buf;
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
		if (ret == Z_STREAM_END)
			break;
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

void WhatsAppProto::GetMessageContent(
	CMStringA &txt, 
	const char *szType,
	const char *szMimeType,
	const char *szUrl,
	const char *szDirectPath,
	const ProtobufCBinaryData &pMediaKey,
	const char *szCaption)
{
	if (szCaption) {
		if (m_bUseBbcodes)
			txt.Append("[b]");
		txt.Append(szCaption);
		if (m_bUseBbcodes)
			txt.Append("[/b]");
		txt.Append("\n");
	}

	CMStringA url = szUrl;
	int idx = url.ReverseFind('/');
	if (idx != -1)
		url.Delete(0, idx+1);
	idx = url.ReverseFind('.');
	if (idx != -1)
		url.Truncate(idx);
	if (szMimeType)
		url.Append(_T2A(ProtoGetAvatarExtension(ProtoGetAvatarFormatByMimeType(szMimeType))));

	char *szMediaType = NEWSTR_ALLOCA(szType);
	szMediaType[0] = toupper(szMediaType[0]);

	MBinBuffer buf = DownloadEncryptedFile(directPath2url(szDirectPath), pMediaKey, szMediaType);
	if (!buf.isEmpty()) {
		CMStringW pwszFileName(GetTmpFileName(szType, url));
		bin2file(buf, pwszFileName);

		pwszFileName.Replace(L"\\", L"/");
		txt.AppendFormat("file://%s", T2Utf(pwszFileName).get());
	}
}

CMStringA WhatsAppProto::GetMessageText(const Wa__Message *pMessage)
{
	CMStringA szMessageText;

	if (pMessage) {
		if (auto *pExt = pMessage->extendedtextmessage) {
			if (pExt->title) {
				if (m_bUseBbcodes)
					szMessageText.Append("[b]");
				szMessageText.Append(pExt->title);
				if (m_bUseBbcodes)
					szMessageText.Append("[/b]");
				szMessageText.Append("\n");
			}

			if (pExt->contextinfo && pExt->contextinfo->quotedmessage)
				szMessageText.AppendFormat("> %s\n\n", pExt->contextinfo->quotedmessage->conversation);

			if (pExt->text)
				szMessageText.Append(pExt->text);
		}
		else if (auto *pAudio = pMessage->audiomessage) {
			GetMessageContent(szMessageText, "audio", pAudio->url, pAudio->directpath, pAudio->mimetype, pAudio->mediakey);
		}
		else if (auto *pVideo = pMessage->videomessage) {
			GetMessageContent(szMessageText, "video", pVideo->url, pVideo->directpath, pVideo->mimetype, pVideo->mediakey, pVideo->caption);
		}
		else if (auto *pImage = pMessage->imagemessage) {
			GetMessageContent(szMessageText, "image", pImage->url, pImage->directpath, pImage->mimetype, pImage->mediakey, pImage->caption);
		}
		else if (mir_strlen(pMessage->conversation))
			szMessageText = pMessage->conversation;
	}

	return szMessageText;
}

/////////////////////////////////////////////////////////////////////////////////////////

void proto::CleanBinary(ProtobufCBinaryData &field)
{
	if (field.data) {
		free(field.data);
		field.data = nullptr;
	}
	field.len = 0;
}

ProtobufCBinaryData proto::SetBinary(const void *pData, size_t len)
{
	ProtobufCBinaryData res;
	if (pData == nullptr) {
		res.data = nullptr;
		res.len = 0;
	}
	else {
		res.data = (uint8_t *)malloc(res.len = len);
		memcpy(res.data, pData, len);
	}
	return res;
}

MBinBuffer proto::Serialize(const ProtobufCMessage *msg)
{
	MBinBuffer res(protobuf_c_message_get_packed_size(msg));
	protobuf_c_message_pack(msg, res.data());
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

CMStringA directPath2url(const char *pszDirectPath)
{
	return CMStringA("https://mmg.whatsapp.net") + pszDirectPath;
}

WAMediaKeys::WAMediaKeys(const uint8_t *pKey, size_t keyLen, const char *pszMediaType)
{
	CMStringA pszHkdfString(FORMAT, "WhatsApp %s Keys", pszMediaType);

	HKDF(EVP_sha256(), (BYTE *)"", 0, pKey, (int)keyLen, (BYTE *)pszHkdfString.c_str(), pszHkdfString.GetLength(), (BYTE *)this, sizeof(*this));
}

MBinBuffer WhatsAppProto::DownloadEncryptedFile(const char *url, const ProtobufCBinaryData &mediaKeys, const char *pszMediaType)
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
			WAMediaKeys out(mediaKeys.data, mediaKeys.len, pszMediaType);
			ret = aesDecrypt(EVP_aes_256_cbc(), out.cipherKey, out.iv, pResp->pData, pResp->dataLength);
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
