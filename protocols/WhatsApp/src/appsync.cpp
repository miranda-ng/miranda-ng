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
				pCollection->hash.init();
				debugLogA("%s: applying snapshot of version %d", pCollection->szName.get(), dwVersion);
				for (auto &it : snapshot.records())
					ParsePatch(pCollection, it, true);
			}
			else debugLogA("%s: skipping snapshot of version %d", pCollection->szName.get(), dwVersion);
		}

		if (auto *pPatchList = coll->getChild("patches")) {
			for (auto &it : pPatchList->getChildren()) {
				proto::SyncdPatch patch;
				patch << it->content;

				dwVersion = patch.version().version();
				if (dwVersion > pCollection->version) {
					debugLogA("%s: applying patch of version %d", pCollection->szName.get(), dwVersion);
					for (auto &jt : patch.mutations())
						ParsePatch(pCollection, jt.record(), jt.operation() == proto::SyncdMutation_SyncdOperation::SyncdMutation_SyncdOperation_SET);
				}
				else debugLogA("%s: skipping patch of version %d", pCollection->szName.get(), dwVersion);
			}
		}

		CMStringA szSetting(FORMAT, "Collection_%s", pszName);
		// setDword(szSetting, dwVersion);

		JSONNode jsonRoot, jsonMap;
		for (auto &it : pCollection->indexValueMap)
			jsonMap << CHAR_PARAM(ptrA(mir_base64_encode(it.first.c_str(), it.first.size())), ptrA(mir_base64_encode(it.second.c_str(), it.second.size())));
		jsonRoot << INT_PARAM("version", dwVersion) << CHAR_PARAM("hash", ptrA(mir_base64_encode(pCollection->hash.hash, sizeof(pCollection->hash.hash))))
			<< JSON_PARAM("indexValueMap", jsonMap);

		string2file(jsonRoot.write(), GetTmpFileName("collection", CMStringA(pszName) + ".json"));
	}
}

static uint8_t sttMutationInfo[] = "WhatsApp Mutation Keys";

void WhatsAppProto::ParsePatch(WACollection *pColl, const ::proto::SyncdRecord &rec, bool bSet)
{
	int id = decodeBigEndian(rec.keyid().id());
	auto &index = rec.index().blob();
	auto &value = rec.value().blob();
	auto &macValue = value.substr(value.size() - 32, value.size());

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

	MBinBuffer decoded = aesDecrypt(EVP_aes_256_cbc(), mutationKeys.encKey, (uint8_t *)value.c_str(), value.c_str() + 16, value.size() - 32);
	if (!decoded.data()) {
		debugLogA("Unable to decode patch with key id=%d", id);
		return;
	}

	proto::SyncActionData data;
	data << decoded;

	debugLogA("Applying patch for %s{%d}: %s", pColl->szName.get(), data.version(), data.Utf8DebugString().c_str());

	if (bSet) {
		JSONNode jsonRoot = JSONNode::parse(data.index().c_str());
		ApplyPatch(jsonRoot, data.value());

		pColl->hash.add(macValue.c_str(), macValue.size());
		pColl->indexValueMap[index] = macValue;
	}
	else {
		auto &prevVal = pColl->indexValueMap.find(index);
		if (prevVal != pColl->indexValueMap.end()) {
			pColl->hash.sub(prevVal->second.c_str(), prevVal->second.size());
			pColl->indexValueMap.erase(prevVal);
		}
	}
}

void WhatsAppProto::ApplyPatch(const JSONNode &index, const proto::SyncActionValue &data)
{
	auto title = index.at((json_index_t)0).as_string();

	if (title == "contact" && data.has_contactaction()) {
		WAJid jid(index.at(1).as_string().c_str());
		auto *pUser = AddUser(jid.toString(), false, jid.isGroup());

		auto &pAction = data.contactaction();
		auto &fullName = pAction.fullname();
		if (!fullName.empty())
			setUString(pUser->hContact, "Nick", fullName.c_str());

		if (pAction.has_firstname()) {
			CMStringA str(pAction.firstname().c_str());
			str.TrimRight();
			setUString(pUser->hContact, "FirstName", str.c_str());
			setUString(pUser->hContact, "LastName", fullName.c_str() + str.GetLength() + 1);
		}
		else {
			size_t idx = fullName.rfind(' ');
			if (idx != fullName.npos) {
				setUString(pUser->hContact, "FirstName", fullName.substr(0, idx).c_str());
				setUString(pUser->hContact, "LastName", fullName.substr(idx+1, fullName.size()).c_str());
			}
			else {
				setUString(pUser->hContact, "FirstName", "");
				setUString(pUser->hContact, "LastName", fullName.c_str());
			}
		}
	}
}
