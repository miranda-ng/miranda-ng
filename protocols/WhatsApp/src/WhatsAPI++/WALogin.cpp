/*
 * WALogin.cpp
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#include "WALogin.h"
#include "ByteArray.h"
#include "ProtocolTreeNode.h"
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>

#include "../common.h" // #TODO Remove Miranda-dependency

using namespace Utilities;

WALogin::WALogin(WAConnection* connection, const std::string &password)
{
	m_pConnection = connection;
	m_szPassword = password;
	m_iAccountKind = -1;
	m_tExpireDate = 0L;
}

std::vector<unsigned char> WALogin::login(const std::vector<unsigned char>& authBlob)
{
	m_pConnection->out.streamStart(m_pConnection->domain, m_pConnection->resource);

	m_pConnection->logData("sent stream start");

	sendFeatures();

	m_pConnection->logData("sent features");

	sendAuth(authBlob);

	m_pConnection->logData("send auth, auth blob size %d", authBlob.size());

	m_pConnection->in.streamStart();

	m_pConnection->logData("read stream start");

	return this->readFeaturesUntilChallengeOrSuccess();
}

void WALogin::sendResponse(const std::vector<unsigned char>& challengeData)
{
	std::vector<unsigned char>* authBlob = this->getAuthBlob(challengeData);
	m_pConnection->out.write(ProtocolTreeNode("response", authBlob));
}

void WALogin::sendFeatures()
{
	ProtocolTreeNode* child = new ProtocolTreeNode("receipt_acks");
	std::vector<ProtocolTreeNode*>* children = new std::vector<ProtocolTreeNode*>();
	children->push_back(child);

	ProtocolTreeNode* pictureChild = new ProtocolTreeNode("w:profile:picture") << XATTR("type", "all");
	children->push_back(pictureChild);

	m_pConnection->out.write(ProtocolTreeNode("stream:features", NULL, children));
}

void WALogin::sendAuth(const std::vector<unsigned char>& existingChallenge)
{
	std::vector<unsigned char>* data = NULL;
	if (!existingChallenge.empty())
		data = getAuthBlob(existingChallenge);

	m_pConnection->out.write(ProtocolTreeNode("auth", data) << 
		XATTR("mechanism", "WAUTH-2") << XATTR("user", m_pConnection->user));
}

std::vector<unsigned char>* WALogin::getAuthBlob(const std::vector<unsigned char>& nonce)
{
	unsigned char out[4*20];
	KeyStream::keyFromPasswordAndNonce(m_szPassword, nonce, out);

	m_pConnection->inputKey.init(out + 40, out + 60);
	m_pConnection->outputKey.init(out, out + 20);

	std::vector<unsigned char>* list = new std::vector<unsigned char>(0);
	for (int i = 0; i < 4; i++)
		list->push_back(0);

	list->insert(list->end(), m_pConnection->user.begin(), m_pConnection->user.end());
	list->insert(list->end(), nonce.begin(), nonce.end());

	m_pConnection->outputKey.encodeMessage(&((*list)[0]), 0, 4, (int)list->size() - 4);
	return list;
}

std::vector<unsigned char> WALogin::readFeaturesUntilChallengeOrSuccess()
{
	while (ProtocolTreeNode *root = m_pConnection->in.nextTree()) {
		string tmp = root->toString();
		m_pConnection->logData(tmp.c_str());

		if (ProtocolTreeNode::tagEquals(root, "stream:features")) {
			m_pConnection->supports_receipt_acks = root->getChild("receipt_acks") != NULL;
			delete root;
			continue;
		}
		if (ProtocolTreeNode::tagEquals(root, "challenge")) {
			std::vector<unsigned char> challengedata(root->data->begin(), root->data->end());
			delete root;
			this->sendResponse(challengedata);
			m_pConnection->logData("Send response");
			std::vector<unsigned char> data = this->readSuccess();
			m_pConnection->logData("Read success");
			return std::vector<unsigned char>(data.begin(), data.end());
		}
		if (ProtocolTreeNode::tagEquals(root, "success")) {
			std::vector<unsigned char> ret(root->data->begin(), root->data->end());
			this->parseSuccessNode(root);
			delete root;
			return ret;
		}
	}
	throw WAException("fell out of loop in readFeaturesAndChallenge", WAException::CORRUPT_STREAM_EX, 0);
}

void WALogin::parseSuccessNode(ProtocolTreeNode* node)
{
	m_pConnection->out.setSecure();

	const string &expiration = node->getAttributeValue("expiration");
	if (!expiration.empty()) {
		m_tExpireDate = atol(expiration.c_str());
		if (m_tExpireDate == 0)
			throw WAException("invalid expire date: " + expiration);
	}

	const string &kind = node->getAttributeValue("kind");
	if (kind == "paid")
		m_iAccountKind = 1;
	else if (kind == "free")
		m_iAccountKind = 0;
	else
		m_iAccountKind = -1;
}

std::vector<unsigned char> WALogin::readSuccess()
{
	ProtocolTreeNode *node = m_pConnection->in.nextTree();

	if (ProtocolTreeNode::tagEquals(node, "failure")) {
		delete node;
		throw WAException("Login failure", WAException::LOGIN_FAILURE_EX, WAException::LOGIN_FAILURE_EX_TYPE_PASSWORD);
	}

	ProtocolTreeNode::require(node, "success");
	this->parseSuccessNode(node);

	const string &status = node->getAttributeValue("status");
	if (status == "expired") {
		delete node;
		throw WAException("Account expired on" + std::string(ctime(&m_tExpireDate)), WAException::LOGIN_FAILURE_EX, WAException::LOGIN_FAILURE_EX_TYPE_EXPIRED, m_tExpireDate);
	}
	if (status == "active") {
		if (node->getAttributeValue("expiration").empty()) {
			delete node;
			throw WAException("active account with no expiration");
		}
	}
	else m_iAccountKind = -1;

	std::vector<unsigned char> data = *node->data;
	delete node;
	return data;
}

WALogin::~WALogin()
{}
