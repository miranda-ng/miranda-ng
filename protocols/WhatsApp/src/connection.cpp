#include "common.h"

void WhatsAppProto::stayConnectedLoop(void*)
{
	std::string cc, in, pass;
	DBVARIANT dbv = { 0 };

	if (!getString(WHATSAPP_KEY_CC, &dbv)) {
		cc = dbv.pszVal;
		db_free(&dbv);
	}
	if (cc.empty()) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a country code."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}

	if (!getString(WHATSAPP_KEY_LOGIN, &dbv)) {
		in = dbv.pszVal;
		db_free(&dbv);
	}
	if (in.empty()) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a phone number without country code."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}
	this->phoneNumber = cc + in;
	this->jid = this->phoneNumber + "@s.whatsapp.net";

	if (!getString(WHATSAPP_KEY_NICK, &dbv)) {
		this->nick = dbv.pszVal;
		db_free(&dbv);
	}
	if (this->nick.empty()) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a nickname."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}

	if (!getString(WHATSAPP_KEY_PASS, &dbv)) {
		pass = dbv.pszVal;
		db_free(&dbv);
	}
	if (pass.empty()) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a password."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}

	// -----------------------------

	Mutex writerMutex;
	int desiredStatus;
	bool error = false;

	this->conn = NULL;

	while (true) {
		if (connection != NULL) {
			delete connection;
			connection = NULL;
		}
		if (this->conn != NULL) {
			delete this->conn;
			this->conn = NULL;
		}

		desiredStatus = this->m_iDesiredStatus;
		if (desiredStatus == ID_STATUS_OFFLINE || error) {
			debugLogA("Set status to offline");
			SetAllContactStatuses(ID_STATUS_OFFLINE, true);
			this->ToggleStatusMenuItems(false);
			int prevStatus = this->m_iStatus;
			this->m_iStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, prevStatus);
			break;
		}

		debugLogA("Connecting...");

		try {
			unsigned passLen;
			ptrA passBin((char*)mir_base64_decode(pass.c_str(), &passLen));
			std::string password(passBin, passLen), resource = ACCOUNT_RESOURCE;
			int portNumber;
			if (getByte(WHATSAPP_KEY_SSL, 0))
				portNumber = 443, resource += "-443";
			else
				portNumber = 5222, resource += "-5222";

			this->conn = new WASocketConnection("c.whatsapp.net", portNumber);
			this->connection = new WAConnection(this->phoneNumber, resource, &this->connMutex, this, this);
			this->connection->init(&writerMutex, this->conn);
			{
				WALogin login(connection, password);

				std::vector<unsigned char> *nextChallenge = login.login(*this->challenge);
				delete this->challenge;
				this->challenge = nextChallenge;
				connection->setLogin(&login);
			}
			connection->nick = this->nick;
			connection->setVerboseId(true);
			if (desiredStatus != ID_STATUS_INVISIBLE)
				connection->sendAvailableForChat();

			debugLogA("Set status to online");
			this->m_iStatus = desiredStatus;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_CONNECTING);
			this->ToggleStatusMenuItems(true);

			// ProcessBuddyList(0);

			// #TODO Move out of try block. Exception is expected on disconnect
			while (true) {
				this->lastPongTime = time(NULL);
				if (!connection->read())
					break;
			}
			debugLogA("Exit from read-loop");
		}
		catch (WAException &e) {
			debugLogA("Exception: %s", e.what());
			error = true;
		}
		catch (exception &e) {
			debugLogA("Exception: %s", e.what());
			error = true;
		}
		catch (...) {
			debugLogA("Unknown exception");
			error = true;
		}
	}
	debugLogA("Break out from loop");
}

void WhatsAppProto::sentinelLoop(void*)
{
	int delay = MAX_SILENT_INTERVAL;
	int quietInterval;
	while (WaitForSingleObjectEx(update_loop_lock_, delay * 1000, true) == WAIT_TIMEOUT) {
		if (this->m_iStatus != ID_STATUS_OFFLINE && this->connection != NULL && this->m_iDesiredStatus == this->m_iStatus) {
			// #TODO Quiet after pong or tree read?
			quietInterval = difftime(time(NULL), this->lastPongTime);
			if (quietInterval >= MAX_SILENT_INTERVAL) {
				try {
					debugLogA("send ping");
					this->lastPongTime = time(NULL);
					this->connection->sendPing();
				}
				catch (exception &e) {
					debugLogA("Exception: %s", e.what());
				}
			}
			else delay = MAX_SILENT_INTERVAL - quietInterval;
		}
		else delay = MAX_SILENT_INTERVAL;
	}
	ResetEvent(update_loop_lock_);
	debugLogA("Exiting sentinel loop");
}

void WhatsAppProto::onPing(const std::string& id)
{
	if (this->isOnline()) {
		try {
			debugLogA("Sending pong with id %s", id.c_str());
			this->connection->sendPong(id);
		}
		CODE_BLOCK_CATCH_ALL
	}
}
