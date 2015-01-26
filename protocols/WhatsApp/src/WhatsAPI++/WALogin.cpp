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

WALogin::WALogin(WAConnection* connection, const std::string& password)
{
	this->connection = connection;
	this->password = password;
	this->account_kind = -1;
	this->expire_date = 0L;
}

std::vector<unsigned char>* WALogin::login(const std::vector<unsigned char>& authBlob)
{
	connection->out->streamStart(connection->domain, connection->resource);

	connection->logData("sent stream start");

	sendFeatures();

	connection->logData("sent features");

	sendAuth(authBlob);

	connection->logData("send auth, auth blob size %d", authBlob.size());

	connection->in->streamStart();

	connection->logData("read stream start");

	return this->readFeaturesUntilChallengeOrSuccess();
}

void WALogin::sendResponse(const std::vector<unsigned char>& challengeData)
{
	std::vector<unsigned char>* authBlob = this->getAuthBlob(challengeData);
	connection->out->write(ProtocolTreeNode("response", authBlob));
}

void WALogin::sendFeatures()
{
	ProtocolTreeNode* child = new ProtocolTreeNode("receipt_acks");
	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>();
	children->push_back(child);

	ProtocolTreeNode* pictureChild = new ProtocolTreeNode("w:profile:picture") << XATTR("type", "all");
	children->push_back(pictureChild);

	connection->out->write(ProtocolTreeNode("stream:features", NULL, children), true);
}

void WALogin::sendAuth(const std::vector<unsigned char>& existingChallenge)
{
	std::vector<unsigned char>* data = NULL;
	if (!existingChallenge.empty())
		data = this->getAuthBlob(existingChallenge);

	connection->out->write(ProtocolTreeNode("auth", data) << 
		XATTR("mechanism", "WAUTH-2") << XATTR("user", connection->user), true);
}

std::vector<unsigned char>* WALogin::getAuthBlob(const std::vector<unsigned char>& nonce)
{
	unsigned char out[4*20];
	KeyStream::keyFromPasswordAndNonce(this->password, nonce, out);

	this->connection->inputKey.init(out + 40, out + 60);
	this->connection->outputKey.init(out, out + 20);

	std::vector<unsigned char>* list = new std::vector<unsigned char>(0);
	for (int i = 0; i < 4; i++)
		list->push_back(0);

	list->insert(list->end(), connection->user.begin(), connection->user.end());
	list->insert(list->end(), nonce.begin(), nonce.end());

	this->connection->outputKey.encodeMessage(&((*list)[0]), 0, 4, (int)list->size() - 4);
	return list;
}

std::vector<unsigned char>* WALogin::readFeaturesUntilChallengeOrSuccess()
{
	while (ProtocolTreeNode *root = connection->in->nextTree()) {
		#ifdef _DEBUG
			{
				string tmp = root->toString();
				connection->logData(tmp.c_str());
			}
		#endif
		if (ProtocolTreeNode::tagEquals(root, "stream:features")) {
			connection->supports_receipt_acks = root->getChild("receipt_acks") != NULL;
			delete root;
			continue;
		}
		if (ProtocolTreeNode::tagEquals(root, "challenge")) {
			std::vector<unsigned char> challengedata(root->data->begin(), root->data->end());
			delete root;
			this->sendResponse(challengedata);
			connection->logData("Send response");
			std::vector<unsigned char> data = this->readSuccess();
			connection->logData("Read success");
			return new std::vector<unsigned char>(data.begin(), data.end());
		}
		if (ProtocolTreeNode::tagEquals(root, "success")) {
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
	connection->out->setSecure();

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
}

std::vector<unsigned char> WALogin::readSuccess()
{
	ProtocolTreeNode *node = connection->in->nextTree();

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
