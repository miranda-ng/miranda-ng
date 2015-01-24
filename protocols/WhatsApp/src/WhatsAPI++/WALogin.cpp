/*
 * WALogin.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "WALogin.h"
#include "ByteArray.h"
#include "ProtocolTreeNode.h"
#include "WAException.h"
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>

#include "../common.h" // #TODO Remove Miranda-dependency

using namespace Utilities;

const std::string WALogin::NONCE_KEY = "nonce=\"";

WALogin::WALogin(WAConnection* connection, BinTreeNodeReader *reader, BinTreeNodeWriter *writer, const std::string& domain, const std::string& user, const std::string& resource, const std::string& password, const std::string& push_name) {
	this->connection = connection;
	this->inn = reader;
	this->out = writer;
	this->domain = domain;
	this->user = user;
	this->resource = resource;
	this->password = password;
	this->push_name = push_name;
	this->supports_receipt_acks = false;
	this->account_kind = -1;
	this->expire_date = 0L;
	this->outputKey = NULL;
}

std::vector<unsigned char>* WALogin::login(const std::vector<unsigned char>& authBlob) {
	this->out->streamStart(this->domain, this->resource);

	_LOGDATA("sent stream start");

	sendFeatures();

	_LOGDATA("sent features");

	sendAuth(authBlob);

	_LOGDATA("send auth, auth blob size %d", authBlob.size());

	this->inn->streamStart();

	_LOGDATA("read stream start");

	return this->readFeaturesUntilChallengeOrSuccess();
}

BinTreeNodeReader* WALogin::getTreeNodeReader()
{
	return this->inn;
}

BinTreeNodeWriter* WALogin::getTreeNodeWriter()
{
	return this->out;
}

void WALogin::sendResponse(const std::vector<unsigned char>& challengeData) {
	std::vector<unsigned char>* authBlob = this->getAuthBlob(challengeData);

	ProtocolTreeNode node("response", NULL, authBlob);

	this->out->write(&node);
}

void WALogin::sendFeatures()
{
	ProtocolTreeNode* child = new ProtocolTreeNode("receipt_acks", NULL);
	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>();
	children->push_back(child);

	std::map<string, string>* attributes = new std::map<string, string>();
	(*attributes)["type"] = "all";
	ProtocolTreeNode* pictureChild = new ProtocolTreeNode("w:profile:picture", attributes);
	 children->push_back(pictureChild);

	// children->push_back(new ProtocolTreeNode("status", NULL));

	ProtocolTreeNode node("stream:features", NULL, NULL, children);
	this->out->write(&node, true);
}

void WALogin::sendAuth(const std::vector<unsigned char>& existingChallenge)
{
	std::vector<unsigned char>* data = NULL;
	if (!existingChallenge.empty()) {
		data = this->getAuthBlob(existingChallenge);
	}

	std::map<string, string>* attributes = new std::map<string, string>();
	(*attributes)["mechanism"] = "WAUTH-2";
	(*attributes)["user"] = this->user;

	ProtocolTreeNode node("auth", attributes, data, NULL);
	this->out->write(&node, true);
}

std::vector<unsigned char>* WALogin::getAuthBlob(const std::vector<unsigned char>& nonce)
{
	unsigned char out[4*20];
	KeyStream::keyFromPasswordAndNonce(this->password, nonce, out);

	if (this->connection->inputKey != NULL)
		delete this->connection->inputKey;
	this->connection->inputKey = new KeyStream(out + 40, out + 60);

	if (this->outputKey != NULL)
		delete this->outputKey;
	this->outputKey = new KeyStream(out, out + 20);

	std::vector<unsigned char>* list = new std::vector<unsigned char>(0);
	for (int i = 0; i < 4; i++)
		list->push_back(0);

	list->insert(list->end(), this->user.begin(), this->user.end());
	list->insert(list->end(), nonce.begin(), nonce.end());

	this->outputKey->encodeMessage(&((*list)[0]), 0, 4, (int)list->size() - 4);
	return list;
}

std::vector<unsigned char>* WALogin::readFeaturesUntilChallengeOrSuccess()
{
	ProtocolTreeNode* root;
	while ((root = this->inn->nextTree()) != NULL) {
		if (ProtocolTreeNode::tagEquals(root, "stream:features")) {
			this->supports_receipt_acks = root->getChild("receipt_acks") != NULL;
			delete root;
			continue;
		}
		if (ProtocolTreeNode::tagEquals(root, "challenge")) {
			// base64_decode(*root->data);
			// _LOGDATA("Challenge data %s (%d)", root->data->c_str(), root->data->length());
			std::vector<unsigned char> challengedata(root->data->begin(), root->data->end());
			delete root;
			this->sendResponse(challengedata);
			_LOGDATA("Send response");
			std::vector<unsigned char> data = this->readSuccess();
			_LOGDATA("Read success");
			return new std::vector<unsigned char>(data.begin(), data.end());
		}
		if (ProtocolTreeNode::tagEquals(root, "success")) {
			// base64_decode(*root->data);
			std::vector<unsigned char>* ret = new std::vector<unsigned char>(root->data->begin(), root->data->end());
			this->parseSuccessNode(root);
			delete root;
			return ret;
		}
	}
	throw WAException("fell out of loop in readFeaturesAndChallenge", WAException::CORRUPT_STREAM_EX, 0);
}

void WALogin::parseSuccessNode(ProtocolTreeNode* node)
{
	const string &expiration = node->getAttributeValue("expiration");
	if (!expiration.empty()) {
		this->expire_date = atol(expiration.c_str());
		if (this->expire_date == 0)
			throw WAException("invalid expire date: " + expiration);
	}

	const string &kind = node->getAttributeValue("kind");
	if (kind == "paid")
		this->account_kind = 1;
	else if (kind == "free")
		this->account_kind = 0;
	else
		this->account_kind = -1;

	if (this->connection->outputKey != NULL)
		delete this->connection->outputKey;
	this->connection->outputKey = this->outputKey;
}

std::vector<unsigned char> WALogin::readSuccess()
{
	ProtocolTreeNode* node = this->inn->nextTree();

	if (ProtocolTreeNode::tagEquals(node, "failure")) {
		delete node;
		throw WAException("Login failure", WAException::LOGIN_FAILURE_EX, WAException::LOGIN_FAILURE_EX_TYPE_PASSWORD);
	}

	ProtocolTreeNode::require(node, "success");
	this->parseSuccessNode(node);

	const string &status = node->getAttributeValue("status");
	if (status == "expired") {
		delete node;
		throw WAException("Account expired on" + std::string(ctime(&this->expire_date)), WAException::LOGIN_FAILURE_EX, WAException::LOGIN_FAILURE_EX_TYPE_EXPIRED, this->expire_date);
	}
	if (status == "active") {
		if (node->getAttributeValue("expiration").empty()) {
			delete node;
			throw WAException("active account with no expiration");
		}
	}
	else this->account_kind = -1;

	std::vector<unsigned char> data = *node->data;
	delete node;
	return data;
}

WALogin::~WALogin()
{}

KeyStream::KeyStream(unsigned char* _key, unsigned char* _keyMac) :
	seq(0)
{
	memcpy(key, _key, 20);
	memcpy(keyMac, _keyMac, 20);

	RC4_set_key(&this->rc4, 20, this->key);

	unsigned char drop[768];
	RC4(&this->rc4, sizeof(drop), drop, drop);

	HMAC_CTX_init(&hmac);
}

KeyStream::~KeyStream()
{
	HMAC_CTX_cleanup(&hmac);
}

void KeyStream::keyFromPasswordAndNonce(const std::string& pass, const std::vector<unsigned char>& nonce, unsigned char *out)
{
	size_t cbSize = nonce.size();

	uint8_t *pNonce = (uint8_t*)_alloca(cbSize + 1);
	memcpy(pNonce, nonce.data(), cbSize);

	for (int i = 0; i < 4; i++) {
		pNonce[cbSize] = i + 1;
		PKCS5_PBKDF2_HMAC_SHA1(pass.data(), (int)pass.size(), pNonce, (int)cbSize+1, 2, 20, out + i*20);
	}
}

void KeyStream::decodeMessage(unsigned char* buffer, int macOffset, int offset, const int length)
{
	unsigned char digest[20];
	this->hmacsha1(buffer + offset, length, digest);

	if (memcmp(&buffer[macOffset], digest, 4))
		throw WAException("invalid MAC", WAException::CORRUPT_STREAM_EX, 0);

	unsigned char* out = (unsigned char*)_alloca(length);
	RC4(&this->rc4, length, buffer + offset, out);
	memcpy(buffer + offset, out, length);
}

void KeyStream::encodeMessage(unsigned char* buffer, int macOffset, int offset, const int length)
{
	unsigned char* out = (unsigned char*)_alloca(length);
	RC4(&this->rc4, length, buffer + offset, out);
	memcpy(buffer + offset, out, length);

	unsigned char digest[20];
	this->hmacsha1(buffer + offset, length, digest);
	memcpy(buffer + macOffset, digest, 4);
}

void KeyStream::hmacsha1(unsigned char* text, int textLength, unsigned char *out)
{
	HMAC_Init(&hmac, this->keyMac, 20, EVP_sha1());
	HMAC_Update(&hmac, text, textLength);

	unsigned char hmacInt[4];
	hmacInt[0] = (this->seq >> 24);
	hmacInt[1] = (this->seq >> 16);
	hmacInt[2] = (this->seq >> 8);
	hmacInt[3] = (this->seq);
	HMAC_Update(&hmac, hmacInt, sizeof(hmacInt));

	unsigned int mdLength;
	HMAC_Final(&hmac, out, &mdLength);

	this->seq++;
}
