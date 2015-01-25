/*
 * WALogin.h
 *
 *  Created on: 26/06/2012
 *      Author: Antonio
 */

#ifndef WALOGIN_H_
#define WALOGIN_H_

#include "BinTreeNodeReader.h"
#include "BinTreeNodeWriter.h"
#include "WAConnection.h"
#include <string>

#include "../OpenSSL/rc4.h"
#include "../OpenSSL/hmac.h"

class WAConnection;
class BinTreeNodeReader;
class BinTreeNodeWriter;

class KeyStream {
private:
	RC4_KEY rc4;
	unsigned char key[20], keyMac[20];
	int seq;
	HMAC_CTX hmac;

	void hmacsha1(unsigned char* text, int textLength, unsigned char *out);

public:
	KeyStream();
	~KeyStream();

	void init(unsigned char *_key, unsigned char *_keyMac);

	static void keyFromPasswordAndNonce(const std::string& pass, const std::vector<unsigned char>& nonce, unsigned char *out);
	void decodeMessage(unsigned char* buffer, int macOffset, int offset, const int length);
	void encodeMessage(unsigned char* buffer, int macOffset, int offset, const int length);
};


class WALogin {
private:
	static const std::string NONCE_KEY;
	WAConnection *connection;

	std::vector<unsigned char>* getAuthBlob(const std::vector<unsigned char>& nonce);
	void sendResponse(const std::vector<unsigned char>& challengeData);
	void sendFeatures();
	void sendAuth(const std::vector<unsigned char>& nonce);
	std::vector<unsigned char>* readFeaturesUntilChallengeOrSuccess();
	void parseSuccessNode(ProtocolTreeNode *node);
	std::vector<unsigned char> readSuccess();

public:
	time_t expire_date;
	int account_kind;
	std::string password;

	WALogin(WAConnection* connection, const std::string& password);
	~WALogin();

	std::vector<unsigned char>* login(const std::vector<unsigned char>& blobLength);
};

#endif /* WALOGIN_H_ */
