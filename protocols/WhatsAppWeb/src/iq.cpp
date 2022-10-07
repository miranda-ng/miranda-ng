/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::OnAccountSync(const WANode &node)
{
	m_arDevices.destroy();

	for (auto &it : node.getChild("devices")->getChildren())
		if (it->title == "device")
			m_arDevices.insert(new WADevice(it->getAttr("jid"), it->getAttrInt("key-index")));
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnIqBlockList(const WANode &node)
{
	for (auto &it : node.getChild("list")->getChildren()) {
		auto *pUser = AddUser(it->getAttr("jid"), false);
		Contact::Hide(pUser->hContact);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnIqCountPrekeys(const WANode &node)
{
	int iCount = node.getChild("count")->getAttrInt("value");
	if (iCount < 5)
		UploadMorePrekeys();
}

void WhatsAppProto::UploadMorePrekeys()
{
	WANodeIq iq(IQ::SET, "encrypt");

	auto regId = encodeBigEndian(getDword(DBKEY_REG_ID));
	iq.addChild("registration")->content.append(regId.c_str(), regId.size());

	iq.addChild("type")->content.append(KEY_BUNDLE_TYPE, 1);
	iq.addChild("identity")->content.append(m_signalStore.signedIdentity.pub);

	const int PORTION = 10;
	m_signalStore.generatePrekeys(PORTION);

	int iStart = getDword(DBKEY_PREKEY_UPLOAD_ID, 1);
	auto *n = iq.addChild("list");
	for (int i = 0; i < PORTION; i++) {
		auto *nKey = n->addChild("key");

		int keyId = iStart + i;
		auto encId = encodeBigEndian(keyId, 3);
		nKey->addChild("id")->content.append(encId.c_str(), encId.size());
		nKey->addChild("value")->content.append(getBlob(CMStringA(FORMAT, "PreKey%dPublic", keyId)));
	}
	setDword(DBKEY_PREKEY_UPLOAD_ID, iStart + PORTION);

	auto *skey = iq.addChild("skey");

	auto encId = encodeBigEndian(m_signalStore.preKey.keyid, 3);
	skey->addChild("id")->content.append(encId.c_str(), encId.size());
	skey->addChild("value")->content.append(m_signalStore.preKey.pub);
	skey->addChild("signature")->content.append(m_signalStore.preKey.signature);

	WSSendNode(iq, &WhatsAppProto::OnIqDoNothing);
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnIqDoNothing(const WANode &)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnNotifyEncrypt(const WANode &node)
{
	auto *pszFrom = node.getAttr("from");
	if (!mir_strcmp(pszFrom, S_WHATSAPP_NET)) {

	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnReceiveInfo(const WANode &node)
{
	if (auto *pChild = node.getFirstChild()) {
		if (pChild->title == "offline")
			debugLogA("Processed %d offline events", pChild->getAttrInt("count"));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnReceiveMessage(const WANode &node)
{
	auto *msgId = node.getAttr("id");
	auto *msgType = node.getAttr("type");
	auto *msgFrom = node.getAttr("from");
	auto *recepient = node.getAttr("recepient");
	auto *participant = node.getAttr("participant");

	if (msgType == nullptr || msgFrom == nullptr || msgId == nullptr) {
		debugLogA("bad message received: <%s> <%s> <%s>", msgType, msgFrom, msgId);
		return;
	}

	WAMSG type;
	WAJid jid(msgFrom);
	CMStringA szAuthor, szChatId;

	// message from one user to another
	if (jid.isUser()) {
		if (recepient) {
			if (m_szJid != msgFrom) {
				debugLogA("strange message: with recepient, but not from me");
				return;
			}
			szChatId = recepient;
		}
		else szChatId = msgFrom;

		type = WAMSG::PrivateChat;
		szAuthor = msgFrom;
	}
	else if (jid.isGroup()) {
		if (!participant) {
			debugLogA("strange message: from group, but without participant");
			return;
		}

		type = WAMSG::GroupChat;
		szAuthor = participant;
		szChatId = msgFrom;
	}
	else if (jid.isBroadcast()) {
		if (!participant) {
			debugLogA("strange message: from group, but without participant");
			return;
		}

		bool bIsMe = m_szJid == participant;
		if (jid.isStatusBroadcast())
			type = (bIsMe) ? WAMSG::DirectStatus : WAMSG::OtherStatus;
		else
			type = (bIsMe) ? WAMSG::PeerBroadcast : WAMSG::OtherBroadcast;
		szChatId = msgFrom;
		szAuthor = participant;
	}
	else {
		debugLogA("invalid message type");
		return;
	}

	CMStringA szSender = (type == WAMSG::PrivateChat) ? szAuthor : szChatId;
	bool bFromMe = (m_szJid == msgFrom);
	if (!bFromMe && participant)
		bFromMe = m_szJid == participant;

	auto *pKey = new proto::MessageKey();
	pKey->set_remotejid(szChatId);
	pKey->set_id(msgId);
	pKey->set_fromme(bFromMe);
	if (participant)
		pKey->set_participant(participant);

	proto::WebMessageInfo msg;
	msg.set_allocated_key(pKey);
	msg.set_messagetimestamp(_atoi64(node.getAttr("t")));
	msg.set_pushname(node.getAttr("notify"));
	if (bFromMe)
		msg.set_status(proto::WebMessageInfo_Status_SERVER_ACK);

	int iDecryptable = 0;

	for (auto &it: node.getChildren()) {
		if (it->title == "verified_name") {
			proto::VerifiedNameCertificate cert;
			cert << it->content;
			
			proto::VerifiedNameCertificate::Details details;
			details.ParseFromString(cert.details());

			msg.set_verifiedbizname(details.verifiedname());
			continue;
		}
		
		if (it->title != "enc" || it->content.length() == 0)
			continue;

		MBinBuffer msgBody;
		auto *pszType = it->getAttr("type");
		try {
			if (!mir_strcmp(pszType, "pkmsg") || !mir_strcmp(pszType, "msg")) {
				CMStringA szUser = (WAJid(szSender).isUser()) ? szSender : szAuthor;
				msgBody = m_signalStore.decryptSignalProto(szUser, pszType, it->content);
			}
			else if (!mir_strcmp(pszType, "skmsg")) {
				msgBody = m_signalStore.decryptGroupSignalProto(szSender, szAuthor, it->content);
			}
			else throw "Invalid e2e type";

			iDecryptable++;

			proto::Message encMsg;
			encMsg << msgBody;
			if (encMsg.devicesentmessage().has_message())
				encMsg = encMsg.devicesentmessage().message();

			if (encMsg.has_senderkeydistributionmessage())
				m_signalStore.processSenderKeyMessage(encMsg.senderkeydistributionmessage());

			msg.set_allocated_message(new proto::Message(encMsg));
		}
		catch (const char *pszError) {
			debugLogA("Message cannot be parsed: %s", pszError);
			msg.set_messagestubtype(proto::WebMessageInfo_StubType::WebMessageInfo_StubType_CIPHERTEXT);
		}

		if (!iDecryptable) {
			debugLogA("Nothing to decrypt");
			msg.set_messagestubtype(proto::WebMessageInfo_StubType::WebMessageInfo_StubType_CIPHERTEXT);
		}
	}
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
	WSSendNode(WANodeIq(IQ::RESULT) << CHAR_PARAM("id", node.getAttr("id")));

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
				buf.append(m_signalStore.signedIdentity.pub);

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
				buf.append(m_signalStore.signedIdentity.pub);
				buf.append(accountSignatureKey.c_str(), accountSignatureKey.size());

				signal_buffer *result;
				ec_private_key key = {};
				memcpy(key.data, m_signalStore.signedIdentity.priv.data(), m_signalStore.signedIdentity.priv.length());
				if (curve_calculate_signature(m_signalStore.CTX(), &result, &key, (BYTE *)buf.data(), buf.length()) != 0)
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

			WANodeIq reply(IQ::RESULT); reply << CHAR_PARAM("id", node.getAttr("id"));

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
		pPairingData->set_eident(m_signalStore.signedIdentity.pub.data(), m_signalStore.signedIdentity.pub.length());
		pPairingData->set_eskeyid(encodeBigEndian(m_signalStore.preKey.keyid));
		pPairingData->set_eskeyval(m_signalStore.preKey.pub.data(), m_signalStore.preKey.pub.length());
		pPairingData->set_eskeysig(m_signalStore.preKey.signature.data(), m_signalStore.preKey.signature.length());
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

void WhatsAppProto::OnServerSync(const WANode &node)
{
	OBJLIST<WACollection> task(1);

	for (auto &it : node.getChildren())
		if (it->title == "collection")
			task.insert(new WACollection(it->getAttr("name"), it->getAttrInt("version")));

	ResyncServer(task);
}

void WhatsAppProto::ResyncServer(const OBJLIST<WACollection> &task)
{
	WANodeIq iq(IQ::SET, "w:sync:app:state");
	
	auto *pList = iq.addChild("sync");
	for (auto &it : task) {
		auto *pCollection = m_arCollections.find(it);
		if (pCollection == nullptr)
			m_arCollections.insert(pCollection = new WACollection(it->szName, 0));

		if (pCollection->version < it->version) {
			auto *pNode = pList->addChild("collection");
			*pNode << CHAR_PARAM("name", it->szName) << INT_PARAM("version", pCollection->version) 
				<< CHAR_PARAM("return_snapshot", (!pCollection->version) ? "true" : "false");
		}
	}

	if (pList->getFirstChild() != nullptr)
		WSSendNode(iq, &WhatsAppProto::OnIqServerSync);
}

void WhatsAppProto::OnIqServerSync(const WANode &node)
{
	for (auto &coll : node.getChild("sync")->getChildren()) {
		if (coll->title != "collection")
			continue;

		auto *pszName = coll->getAttr("name");
		
		auto *pCollection = FindCollection(pszName);
		if (pCollection == nullptr) {
			pCollection = new WACollection(pszName, 0);
			m_arCollections.insert(pCollection);
		}

		int dwVersion = 0;

		CMStringW wszSnapshotPath(GetTmpFileName("collection", pszName));
		if (auto *pSnapshot = coll->getChild("snapshot")) {
			proto::ExternalBlobReference body;
			body << pSnapshot->content;
			if (!body.has_directpath() || !body.has_mediakey()) {
				debugLogA("Invalid snapshot data, skipping");
				continue;
			}
			
			MBinBuffer buf = DownloadEncryptedFile(directPath2url(body.directpath().c_str()), body.mediakey(), "App State");
			if (!buf.data()) {
				debugLogA("Invalid downloaded snapshot data, skipping");
				continue;
			}

			proto::SyncdSnapshot snapshot;
			snapshot << buf;

			dwVersion = snapshot.version().version();
			if (dwVersion > pCollection->version) {
				auto &hash = snapshot.mac();
				pCollection->hash.assign(hash.c_str(), hash.size());
				
				for (auto &it : snapshot.records())
					pCollection->parseRecord(it, true);
			}
		}

		if (auto *pPatchList = coll->getChild("patches")) {
			for (auto &it : pPatchList->getChildren()) {
				proto::SyncdPatch patch;
				patch << it->content;

				dwVersion = patch.version().version();
				if (dwVersion > pCollection->version)
					for (auto &jt : patch.mutations())
						pCollection->parseRecord(jt.record(), jt.operation() == proto::SyncdMutation_SyncdOperation::SyncdMutation_SyncdOperation_SET);
			}
		}

		CMStringA szSetting(FORMAT, "Collection_%s", pszName);
		// setDword(szSetting, dwVersion);

		JSONNode jsonRoot, jsonMap; 
		for (auto &it : pCollection->indexValueMap)
			jsonMap << CHAR_PARAM(ptrA(mir_base64_encode(it.first.c_str(), it.first.size())), ptrA(mir_base64_encode(it.second.c_str(), it.second.size())));
		jsonRoot << INT_PARAM("version", dwVersion) << JSON_PARAM("indexValueMap", jsonMap);
		
		// string2file(jsonRoot.write(), GetTmpFileName("collection", CMStringA(pszName) + ".json"));
	}
}

void WACollection::parseRecord(const ::proto::SyncdRecord &rec, bool bSet)
{
	// auto &id = rec.keyid().id();
	auto &index = rec.index().blob();
	auto &value = rec.value().blob();

	if (bSet) {
		indexValueMap[index] = value.substr(0, value.size() - 32);
	}
	else indexValueMap.erase(index);
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnSuccess(const WANode &)
{
	OnLoggedIn();

	WSSendNode(WANodeIq(IQ::SET, "passive") << XCHILD("active"), &WhatsAppProto::OnIqDoNothing);
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::InitPersistentHandlers()
{
	m_arPersistent.insert(new WAPersistentHandler("iq", "set", "md", "pair-device", &WhatsAppProto::OnIqPairDevice));
	m_arPersistent.insert(new WAPersistentHandler("iq", "set", "md", "pair-success", &WhatsAppProto::OnIqPairSuccess));

	m_arPersistent.insert(new WAPersistentHandler("notification", "encrypt", 0, 0, &WhatsAppProto::OnNotifyEncrypt));
	m_arPersistent.insert(new WAPersistentHandler("notification", "account_sync", 0, 0, &WhatsAppProto::OnAccountSync));
	m_arPersistent.insert(new WAPersistentHandler("notification", "server_sync", 0, 0, &WhatsAppProto::OnServerSync));

	m_arPersistent.insert(new WAPersistentHandler("ib", 0, 0, 0, &WhatsAppProto::OnReceiveInfo));
	m_arPersistent.insert(new WAPersistentHandler("message", 0, 0, 0, &WhatsAppProto::OnReceiveMessage));
	m_arPersistent.insert(new WAPersistentHandler("stream:error", 0, 0, 0, &WhatsAppProto::OnStreamError));
	m_arPersistent.insert(new WAPersistentHandler("success", 0, 0, 0, &WhatsAppProto::OnSuccess));

	m_arPersistent.insert(new WAPersistentHandler(0, "result", 0, 0, &WhatsAppProto::OnIqResult));
}
