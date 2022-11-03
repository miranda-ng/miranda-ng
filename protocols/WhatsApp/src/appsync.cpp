/*

WhatsApp plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::InitSync()
{
	m_arCollections.insert(new WACollection("regular"));
	m_arCollections.insert(new WACollection("regular_high"));
	m_arCollections.insert(new WACollection("regular_low"));
	m_arCollections.insert(new WACollection("critical_block"));
	m_arCollections.insert(new WACollection("critical_unblock_low"));

	for (auto &it : m_arCollections) {
		CMStringW wszPath(GetTmpFileName("collection", it->szName));
		wszPath.Append(L".json");
		if (_waccess(wszPath, 0))
			continue;

		JSONNode root = JSONNode::parse(file2string(wszPath));
		it->version = root["version"].as_int();

		auto szHash = decodeBinStr(root["hash"].as_string());
		if (szHash.size() == sizeof(it->hash.hash))
			memcpy(it->hash.hash, szHash.c_str(), sizeof(it->hash.hash));

		for (auto &val : root["indexValueMap"])
			it->indexValueMap[decodeBinStr(val.name())] = decodeBinStr(val.as_string());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::OnServerSync(const WANode &node)
{
	OBJLIST<WACollection> task(1);

	for (auto &it : node.getChildren())
		if (it->title == "collection")
			task.insert(new WACollection(it->getAttr("name"), it->getAttrInt("version")));

	ResyncServer(task);
	SendAck(node);
}

void WhatsAppProto::ResyncAll()
{
	ResyncServer(m_arCollections);
}

void WhatsAppProto::ResyncServer(const OBJLIST<WACollection> &task)
{
	WANodeIq iq(IQ::SET, "w:sync:app:state");

	auto *pList = iq.addChild("sync");
	for (auto &it : task) {
		auto *pCollection = m_arCollections.find(it);
		if (pCollection == nullptr)
			m_arCollections.insert(pCollection = new WACollection(it->szName, 0));

		if (!pCollection->version || pCollection->version < it->version) {
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
			proto::ExternalBlobReference body(pSnapshot->content);
			if (!body->directpath || !body->has_mediakey) {
				debugLogA("Invalid snapshot data, skipping");
				continue;
			}

			MBinBuffer buf = DownloadEncryptedFile(directPath2url(body->directpath), body->mediakey, "App State");
			if (!buf.data()) {
				debugLogA("Invalid downloaded snapshot data, skipping");
				continue;
			}

			proto::SyncdSnapshot snapshot(unpadBuffer16(buf));
			if (!snapshot) {
				debugLogA("%s: unable to decode snapshot, skipping");
				continue;
			}

			dwVersion = snapshot->version->version;
			if (dwVersion > pCollection->version) {
				pCollection->hash.init();
				debugLogA("%s: applying snapshot of version %d", pCollection->szName.get(), dwVersion);
				for (int i=0; i < snapshot->n_records; i++)
					ParsePatch(pCollection, snapshot->records[i], true);
			}
			else debugLogA("%s: skipping snapshot of version %d", pCollection->szName.get(), dwVersion);
		}

		if (auto *pPatchList = coll->getChild("patches")) {
			for (auto &it : pPatchList->getChildren()) {
				proto::SyncdPatch patch(it->content);
				if (!patch) {
					debugLogA("%s: unable to decode patch, skipping");
					continue;
				}

				dwVersion = patch->version->version;
				if (dwVersion > pCollection->version) {
					debugLogA("%s: applying patch of version %d", pCollection->szName.get(), dwVersion);
					for (int i = 0; i < patch->n_mutations; i++) {
						auto &jt = *patch->mutations[i];
						ParsePatch(pCollection, jt.record, jt.operation == WA__SYNCD_MUTATION__SYNCD_OPERATION__SET);
					}
				}
				else debugLogA("%s: skipping patch of version %d", pCollection->szName.get(), dwVersion);
			}
		}

		JSONNode jsonRoot, jsonMap;
		for (auto &it : pCollection->indexValueMap)
			jsonMap << CHAR_PARAM(ptrA(mir_base64_encode(it.first.c_str(), it.first.size())), ptrA(mir_base64_encode(it.second.c_str(), it.second.size())));
		jsonRoot << INT_PARAM("version", dwVersion) << CHAR_PARAM("hash", ptrA(mir_base64_encode(pCollection->hash.hash, sizeof(pCollection->hash.hash))))
			<< JSON_PARAM("indexValueMap", jsonMap);

		string2file(jsonRoot.write(), GetTmpFileName("collection", CMStringA(pszName) + ".json"));
	}
}

static uint8_t sttMutationInfo[] = "WhatsApp Mutation Keys";

void WhatsAppProto::ParsePatch(WACollection *pColl, const Wa__SyncdRecord *rec, bool bSet)
{
	int id = decodeBigEndian(rec->keyid->id);
	auto &indexBlob = rec->index->blob;
	auto &value = rec->value->blob;
	
	auto *macValue = value.data + value.len - 32;
	std::string index((char *)indexBlob.data, indexBlob.len);

	MBinBuffer key(getBlob(CMStringA(FORMAT, "AppSyncKey%d", id)));
	if (!key.data()) {
		debugLogA("No key with id=%d to decode a patch");
		return;
	}

	struct
	{
		uint8_t indexKey[32];
		uint8_t encKey[32];
		uint8_t macKey[32];
		uint8_t snapshotMacKey[32];
		uint8_t patchMacKey[32];

	} mutationKeys;

	HKDF(EVP_sha256(), (BYTE *)"", 0, key.data(), key.length(), sttMutationInfo, sizeof(sttMutationInfo) - 1, (BYTE *)&mutationKeys, sizeof(mutationKeys));

	MBinBuffer decoded = aesDecrypt(EVP_aes_256_cbc(), mutationKeys.encKey, value.data, value.data + 16, value.len - 32);
	if (!decoded.data()) {
		debugLogA("Unable to decode patch with key id=%d", id);
		return;
	}

	proto::SyncActionData data(unpadBuffer16(decoded));
	if (!data) {
		debugLogA("Unable to decode action data with id=%d", id);
		return;
	}
	
	JSONNode jsonRoot = JSONNode::parse((char *)data->index.data);

	if (bSet) {
		ApplyPatch(jsonRoot, data->value);

		pColl->hash.add(macValue, 32);
		pColl->indexValueMap[index] = std::string((char*)macValue, 32);
	}
	else {
		debugLogA("Removing data with index: %s", jsonRoot.write().c_str());

		auto &prevVal = pColl->indexValueMap.find(index);
		if (prevVal != pColl->indexValueMap.end()) {
			pColl->hash.sub(prevVal->second.c_str(), prevVal->second.size());
			pColl->indexValueMap.erase(prevVal);
		}
	}
}

void WhatsAppProto::ApplyPatch(const JSONNode &index, const Wa__SyncActionValue *data)
{
	debugLogA("Applying patch for %s: %s", index.write().c_str(), protobuf_c_text_to_string(data).c_str());

	auto title = index.at((json_index_t)0).as_string();

	if (title == "contact" && data->contactaction) {
		auto *pUser = AddUser(index.at(1).as_string().c_str(), false);

		auto *pAction = data->contactaction;
		auto &fullName = pAction->fullname;
		if (fullName)
			setUString(pUser->hContact, "Nick", fullName);

		if (pAction->firstname) {
			CMStringA str(pAction->firstname);
			str.TrimRight();
			setUString(pUser->hContact, "FirstName", str.c_str());
			setUString(pUser->hContact, "LastName", fullName + str.GetLength() + 1);
		}
		else {
			auto *p = strrchr(fullName, ' ');
			if (p != 0) {
				*p = 0;
				setUString(pUser->hContact, "FirstName", fullName);
				setUString(pUser->hContact, "LastName", p+1);
			}
			else {
				setUString(pUser->hContact, "FirstName", "");
				setUString(pUser->hContact, "LastName", fullName);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ProcessHistorySync(const Wa__HistorySync *pSync)
{
	debugLogA("Got history sync: %s", protobuf_c_text_to_string(pSync).c_str());

	switch (pSync->synctype) {
	case WA__HISTORY_SYNC__HISTORY_SYNC_TYPE__INITIAL_BOOTSTRAP:
	case WA__HISTORY_SYNC__HISTORY_SYNC_TYPE__RECENT:
		for (int i = 0; i < pSync->n_conversations; i++) {
			auto *pChat = pSync->conversations[i];

			auto *pUser = AddUser(pChat->id, false);
			for (int j = 0; j < pChat->n_messages; j++) {
				auto *pMessage = pChat->messages[j];
				if (!pMessage->message)
					continue;

				MEVENT hEvent = db_event_getById(m_szModuleName, pMessage->message->key->id);
				if (hEvent) {
					debugLogA("Event %s is already processed", pMessage->message->key->id);
					continue;
				}

				CMStringA szMessageText(GetMessageText(pMessage->message->message));
				if (!szMessageText.IsEmpty()) {
					PROTORECVEVENT pre = {};
					pre.timestamp = pMessage->message->messagetimestamp;
					pre.szMessage = szMessageText.GetBuffer();
					pre.szMsgId = pMessage->message->key->id;
					pre.flags = PREF_CREATEREAD;
					if (pMessage->message->key->fromme)
						pre.flags |= PREF_SENT;
					ProtoChainRecvMsg(pUser->hContact, &pre);
				}
			}
		}
		break;

	case WA__HISTORY_SYNC__HISTORY_SYNC_TYPE__PUSH_NAME:
		for (int i = 0; i < pSync->n_pushnames; i++) {
			auto *pName = pSync->pushnames[i];
			if (auto *pUser = FindUser(pName->id))
				setUString(pUser->hContact, "Nick", pName->pushname);
		}
		break;

	case WA__HISTORY_SYNC__HISTORY_SYNC_TYPE__INITIAL_STATUS_V3:
		for (int i = 0; i < pSync->n_statusv3messages;  i++) {
			auto *pStatus = pSync->statusv3messages[i];
			// TODO
		}
		break;
	}
}
