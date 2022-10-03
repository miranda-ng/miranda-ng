/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

static int sttEnumPrekeys(const char *szSetting, void *param)
{
	std::vector<int> *list = (std::vector<int> *)param;
	if (!memcmp(szSetting, "PreKey", 6) && !strstr(szSetting, "Public"))
		list->push_back(atoi(szSetting + 6));
	return 0;
}

void WhatsAppProto::OnIqCountPrekeys(const WANode &node)
{
	std::vector<int> ids;
	db_enum_settings(0, sttEnumPrekeys, m_szModuleName, &ids);

	int iCount = node.getChild("count")->getAttrInt("value");
	if (iCount >= ids.size()) {
		debugLogA("Prekeys are already uploaded");
		return;
	}

	WANode iq("iq");
	iq << CHAR_PARAM("xmlns", "encrypt") << CHAR_PARAM("type", "set") << CHAR_PARAM("to", S_WHATSAPP_NET) << CHAR_PARAM("id", generateMessageId());

	auto regId = encodeBigEndian(getDword(DBKEY_REG_ID));
	iq.addChild("registration")->content.append(regId.c_str(), regId.size());

	iq.addChild("type")->content.append(KEY_BUNDLE_TYPE, 1);
	iq.addChild("identity")->content.append(m_noise->signedIdentity.pub);

	auto *n = iq.addChild("list");
	for (auto &keyId : ids) {
		auto *nKey = n->addChild("key");
			
		auto encId = encodeBigEndian(keyId, 3);
		nKey->addChild("id")->content.append(encId.c_str(), encId.size());
		nKey->addChild("value")->content.append(getBlob(CMStringA(FORMAT, "PreKey%dPublic", keyId)));
	}

	auto *skey = iq.addChild("skey");

	auto encId = encodeBigEndian(m_noise->preKey.keyid, 3);
	skey->addChild("id")->content.append(encId.c_str(), encId.size());
	skey->addChild("value")->content.append(m_noise->preKey.pub);
	skey->addChild("signature")->content.append(m_noise->preKey.signature);

	WSSendNode(iq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnStreamError(const WANode &node)
{
	m_bTerminated = true;

	if (auto *pszCode = node.getAttr("code")) {
		switch (atoi(pszCode)) {
		case 401:
			debugLogA("Connection logged out from another device, exiting");
			break;

		case 408:
			debugLogA("Connection lost, exiting");
			break;

		case 411:
			debugLogA("Conflict between two devices, exiting");
			break;

		case 428:
			debugLogA("Connection forcibly closed by the server, exiting");
			break;

		case 440:
			debugLogA("Connection replaced from another device, exiting");
			break;

		case 515:
			debugLogA("Server required to restart immediately, leaving thread");
			m_bRespawn = true;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnSuccess(const WANode &)
{
	OnLoggedIn();

	WANode iq("iq");
	iq << CHAR_PARAM("to", S_WHATSAPP_NET) << CHAR_PARAM("xmlns", "passive") << CHAR_PARAM("type", "set");
	iq.addChild("active");
	WSSendNode(iq);
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnIqResult(const WANode &node)
{
	if (auto *pszId = node.getAttr("id"))
		for (auto &it: m_arPacketQueue) 
			if (it->szPacketId == pszId)
				(this->*it->pHandler)(node);
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnIqPairDevice(const WANode &node)
{
	WANode reply("iq");
	reply << CHAR_PARAM("to", S_WHATSAPP_NET) << CHAR_PARAM("type", "result") << CHAR_PARAM("id", node.getAttr("id"));
	WSSendNode(reply);

	if (auto *pRef = node.getChild("pair-device")->getChild("ref")) {
		ShowQrCode(pRef->getBody());
	}
	else {
		debugLogA("OnIqPairDevice: got reply without ref, exiting");
		ShutdownSession();
	}
}

void WhatsAppProto::OnIqPairSuccess(const WANode &node)
{
	CloseQrDialog();

	auto *pRoot = node.getChild("pair-success");

	try {
		if (auto *pPlatform = pRoot->getChild("platform"))
			debugLogA("Got response from platform: %s", pPlatform->getBody().c_str());

		if (auto *pBiz = pRoot->getChild("biz"))
			if (auto *pszName = pBiz->getAttr("name"))
				setUString("Nick", pszName);

		if (auto *pDevice = pRoot->getChild("device")) {
			if (auto *pszJid = pDevice->getAttr("jid")) {
				WAJid jid(pszJid);
				m_szJid = jid.user + "@" + jid.server;
				setUString(DBKEY_JID, m_szJid);
				setDword(DBKEY_DEVICE_ID, jid.device);
			}
		}
		else throw "OnIqPairSuccess: got reply without device info, exiting";

		if (auto *pIdentity = pRoot->getChild("device-identity")) {
			proto::ADVSignedDeviceIdentityHMAC payload;
			if (!payload.ParseFromArray(pIdentity->content.data(), (int)pIdentity->content.length()))
				throw "OnIqPairSuccess: got reply with invalid identity, exiting";

			auto &hmac = payload.hmac();
			auto &details = payload.details();
			{
				// check details signature using HMAC
				uint8_t signature[32];
				unsigned int out_len = sizeof(signature);
				MBinBuffer secret(getBlob(DBKEY_SECRET_KEY));
				HMAC(EVP_sha256(), secret.data(), (int)secret.length(), (BYTE *)details.c_str(), (int)details.size(), signature, &out_len);
				if (memcmp(hmac.c_str(), signature, sizeof(signature)))
					throw "OnIqPairSuccess: got reply with invalid details signature, exiting";
			}

			proto::ADVSignedDeviceIdentity account;
			if (!account.ParseFromString(details))
				throw "OnIqPairSuccess: got reply with invalid account, exiting";

			auto &deviceDetails = account.details();
			auto &accountSignature = account.accountsignature();
			auto &accountSignatureKey = account.accountsignaturekey();
			{
				MBinBuffer buf;
				buf.append("\x06\x00", 2);
				buf.append(deviceDetails.c_str(), deviceDetails.size());
				buf.append(m_noise->signedIdentity.pub.data(), m_noise->signedIdentity.pub.length());

				ec_public_key key = {};
				memcpy(key.data, accountSignatureKey.c_str(), sizeof(key.data));
				if (1 != curve_verify_signature(&key, (BYTE *)buf.data(), buf.length(), (BYTE *)accountSignature.c_str(), accountSignature.size()))
					throw "OnIqPairSuccess: got reply with invalid account signature, exiting";
			}
			debugLogA("Received valid account signature");
			{
				MBinBuffer buf;
				buf.append("\x06\x01", 2);
				buf.append(deviceDetails.c_str(), deviceDetails.size());
				buf.append(m_noise->signedIdentity.pub.data(), m_noise->signedIdentity.pub.length());
				buf.append(accountSignatureKey.c_str(), accountSignatureKey.size());

				signal_buffer *result;
				ec_private_key key = {};
				memcpy(key.data, m_noise->signedIdentity.priv.data(), m_noise->signedIdentity.priv.length());
				if (curve_calculate_signature(g_plugin.pCtx, &result, &key, (BYTE *)buf.data(), buf.length()) != 0)
					throw "OnIqPairSuccess: cannot calculate account signature, exiting";

				account.set_devicesignature(result->data, result->len);
				signal_buffer_free(result);
			}

			setDword("SignalDeviceId", 0);
			{
				MBinBuffer key;
				if (accountSignatureKey.size() == 32)
					key.append(KEY_BUNDLE_TYPE, 1);
				key.append(accountSignatureKey.c_str(), accountSignatureKey.size());
				db_set_blob(0, m_szModuleName, "SignalIdentifierKey", key.data(), (int)key.length());
			}

			account.clear_accountsignaturekey();

			MBinBuffer accountEnc(account.ByteSize());
			account.SerializeToArray(accountEnc.data(), (int)accountEnc.length());
			db_set_blob(0, m_szModuleName, "WAAccount", accountEnc.data(), (int)accountEnc.length());

			proto::ADVDeviceIdentity deviceIdentity;
			deviceIdentity.ParseFromString(deviceDetails);

			WANode reply("iq");
			reply << CHAR_PARAM("to", S_WHATSAPP_NET) << CHAR_PARAM("type", "result") << CHAR_PARAM("id", node.getAttr("id"));

			WANode *nodePair = reply.addChild("pair-device-sign");

			WANode *nodeDeviceIdentity = nodePair->addChild("device-identity");
			nodeDeviceIdentity->addAttr("key-index", deviceIdentity.keyindex());
			nodeDeviceIdentity->content.append(accountEnc.data(), accountEnc.length());
			WSSendNode(reply);
		}
		else throw "OnIqPairSuccess: got reply without identity, exiting";
	}
	catch (const char *pErrMsg) {
		debugLogA(pErrMsg);
		ShutdownSession();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnProcessHandshake(const void *pData, int cbLen)
{
	proto::HandshakeMessage msg;
	if (!msg.ParseFromArray(pData, cbLen)) {
		debugLogA("Error parsing data, exiting");

LBL_Error:
		ShutdownSession();
		return;
	}

	auto &static_ = msg.serverhello().static_();
	auto &payload_ = msg.serverhello().payload();
	auto &ephemeral_ = msg.serverhello().ephemeral();

	m_noise->updateHash(ephemeral_.c_str(), ephemeral_.size());
	m_noise->mixIntoKey(m_noise->ephemeral.priv.data(), ephemeral_.c_str());

	MBinBuffer decryptedStatic = m_noise->decrypt(static_.c_str(), static_.size());
	m_noise->mixIntoKey(m_noise->ephemeral.priv.data(), decryptedStatic.data());

	MBinBuffer decryptedCert = m_noise->decrypt(payload_.c_str(), payload_.size());

	proto::CertChain cert; cert.ParseFromArray(decryptedCert.data(), (int)decryptedCert.length());
	proto::CertChain::NoiseCertificate::Details details; details.ParseFromString(cert.intermediate().details());
	if (details.issuerserial() != 0) {
		debugLogA("Invalid certificate serial number, exiting");
		goto LBL_Error;
	}

	MBinBuffer encryptedPub = m_noise->encrypt(m_noise->noiseKeys.pub.data(), m_noise->noiseKeys.pub.length());
	m_noise->mixIntoKey(m_noise->noiseKeys.priv.data(), ephemeral_.c_str());

	// create reply
	proto::ClientPayload node;

	MFileVersion v;
	Miranda_GetFileVersion(&v);

	// not received our jid from server? generate registration packet then
	if (m_szJid.IsEmpty()) {
		uint8_t appVersion[16];
		mir_md5_hash((BYTE *)APP_VERSION, sizeof(APP_VERSION) - 1, appVersion);

		auto *pAppVersion = new proto::DeviceProps_AppVersion();
		pAppVersion->set_primary(v[0]);
		pAppVersion->set_secondary(v[1]);
		pAppVersion->set_tertiary(v[2]);
		pAppVersion->set_quaternary(v[3]);

		proto::DeviceProps pCompanion;
		pCompanion.set_os("Miranda");
		pCompanion.set_allocated_version(pAppVersion);
		pCompanion.set_platformtype(proto::DeviceProps_PlatformType_DESKTOP);
		pCompanion.set_requirefullsync(false);

		MBinBuffer buf(pCompanion.ByteSize());
		pCompanion.SerializeToArray(buf.data(), (int)buf.length());

		auto *pPairingData = new proto::ClientPayload_DevicePairingRegistrationData();
		pPairingData->set_deviceprops(buf.data(), buf.length());
		pPairingData->set_buildhash(appVersion, sizeof(appVersion));
		pPairingData->set_eregid(encodeBigEndian(getDword(DBKEY_REG_ID)));
		pPairingData->set_ekeytype(KEY_BUNDLE_TYPE);
		pPairingData->set_eident(m_noise->signedIdentity.pub.data(), m_noise->signedIdentity.pub.length());
		pPairingData->set_eskeyid(encodeBigEndian(m_noise->preKey.keyid));
		pPairingData->set_eskeyval(m_noise->preKey.pub.data(), m_noise->preKey.pub.length());
		pPairingData->set_eskeysig(m_noise->preKey.signature.data(), m_noise->preKey.signature.length());
		node.set_allocated_devicepairingdata(pPairingData);

		node.set_passive(false);
	}
	// generate login packet
	else {
		WAJid jid(m_szJid);
		node.set_username(_atoi64(jid.user));
		node.set_device(getDword(DBKEY_DEVICE_ID));
		node.set_passive(true);
	}

	auto *pUserVersion = new proto::ClientPayload_UserAgent_AppVersion();
	pUserVersion->set_primary(2);
	pUserVersion->set_secondary(2230);
	pUserVersion->set_tertiary(15);

	auto *pUserAgent = new proto::ClientPayload_UserAgent();
	pUserAgent->set_allocated_appversion(pUserVersion);
	pUserAgent->set_platform(proto::ClientPayload_UserAgent_Platform_WEB);
	pUserAgent->set_releasechannel(proto::ClientPayload_UserAgent_ReleaseChannel_RELEASE);
	pUserAgent->set_mcc("000");
	pUserAgent->set_mnc("000");
	pUserAgent->set_osversion("0.1");
	pUserAgent->set_osbuildnumber("0.1");
	pUserAgent->set_manufacturer("");
	pUserAgent->set_device("Desktop");
	pUserAgent->set_localelanguageiso6391("en");
	pUserAgent->set_localecountryiso31661alpha2("US");

	auto *pWebInfo = new proto::ClientPayload_WebInfo();
	pWebInfo->set_websubplatform(proto::ClientPayload_WebInfo_WebSubPlatform_WEB_BROWSER);

	node.set_connecttype(proto::ClientPayload_ConnectType_WIFI_UNKNOWN);
	node.set_connectreason(proto::ClientPayload_ConnectReason_USER_ACTIVATED);
	node.set_allocated_useragent(pUserAgent);
	node.set_allocated_webinfo(pWebInfo);

	MBinBuffer payload(node.ByteSize());
	node.SerializeToArray(payload.data(), (int)payload.length());

	MBinBuffer payloadEnc = m_noise->encrypt(payload.data(), payload.length());

	auto *pFinish = new proto::HandshakeMessage_ClientFinish();
	pFinish->set_payload(payloadEnc.data(), payloadEnc.length());
	pFinish->set_static_(encryptedPub.data(), encryptedPub.length());

	proto::HandshakeMessage handshake;
	handshake.set_allocated_clientfinish(pFinish);
	WSSend(handshake);

	m_noise->finish();
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::InitPersistentHandlers()
{
	m_arPersistent.insert(new WAPersistentHandler("iq", "set", "md", "pair-device", &WhatsAppProto::OnIqPairDevice));
	m_arPersistent.insert(new WAPersistentHandler("iq", "set", "md", "pair-success", &WhatsAppProto::OnIqPairSuccess));
	m_arPersistent.insert(new WAPersistentHandler(0, "result", 0, 0, &WhatsAppProto::OnIqResult));

	m_arPersistent.insert(new WAPersistentHandler("stream:error", 0, 0, 0, &WhatsAppProto::OnStreamError));
	m_arPersistent.insert(new WAPersistentHandler("success", 0, 0, 0, &WhatsAppProto::OnSuccess));
}
