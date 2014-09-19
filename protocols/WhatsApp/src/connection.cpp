#include "common.h"

void WhatsAppProto::ChangeStatus(void*)
{
	if (m_iDesiredStatus != ID_STATUS_OFFLINE && m_iStatus == ID_STATUS_OFFLINE) {
		ResetEvent(update_loop_lock_);
		ForkThread(&WhatsAppProto::sentinelLoop, this);
		ForkThread(&WhatsAppProto::stayConnectedLoop, this);
	}
	else if (m_iStatus == ID_STATUS_INVISIBLE && m_iDesiredStatus == ID_STATUS_ONLINE) {
		if (this->connection != NULL) {
			this->connection->sendAvailableForChat();
			m_iStatus = ID_STATUS_ONLINE;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_INVISIBLE);
		}
	}
	else if (m_iStatus == ID_STATUS_ONLINE && m_iDesiredStatus == ID_STATUS_INVISIBLE) {
		if (this->connection != NULL) {
			this->connection->sendClose();
			m_iStatus = ID_STATUS_INVISIBLE;
			SetAllContactStatuses(ID_STATUS_OFFLINE, true);
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_ONLINE);
		}
	}
	else if (m_iDesiredStatus == ID_STATUS_OFFLINE) {
		if (this->conn != NULL) {
			SetEvent(update_loop_lock_);
			this->conn->forceShutdown();
			debugLogA("Forced shutdown");
		}
	}
}

void WhatsAppProto::stayConnectedLoop(void*)
{
	std::string cc, in, pass;
	DBVARIANT dbv = { 0 };

	if (!getString(WHATSAPP_KEY_CC, &dbv)) {
		cc = dbv.pszVal;
		db_free(&dbv);
	}
	if (cc.empty()) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a country-code."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}

	if (!getString(WHATSAPP_KEY_LOGIN, &dbv)) {
		in = dbv.pszVal;
		db_free(&dbv);
	}
	if (in.empty()) {
		NotifyEvent(m_tszUserName, TranslateT("Please enter a phone-number without country code."), NULL, WHATSAPP_EVENT_CLIENT);
		return;
	}
	this->phoneNumber = cc + in;
	this->jid = this->phoneNumber + "@s.whatsapp.net";

	if (!getString(WHATSAPP_KEY_NICK, &dbv)) {
		this->nick = dbv.pszVal;
		db_free(&dbv);
	}
	if (nick.empty()) {
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
	WALogin* login = NULL;
	int desiredStatus;
	bool error = false;

	this->conn = NULL;

	while (true) {
		if (connection != NULL) {
			if (connection->getLogin() == NULL && login != NULL) {
				delete login;
				login = NULL;
			}
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
		this->m_iStatus = ID_STATUS_CONNECTING;
		ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_OFFLINE, m_iStatus);

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

			connection = new WAConnection(&this->connMutex, this, this);
			login = new WALogin(connection, new BinTreeNodeReader(connection, conn, WAConnection::dictionary, WAConnection::DICTIONARY_LEN),
				new BinTreeNodeWriter(connection, conn, WAConnection::dictionary, WAConnection::DICTIONARY_LEN, &writerMutex),
				"s.whatsapp.net", this->phoneNumber, resource, password, nick);

			std::vector<unsigned char>* nextChallenge = login->login(*this->challenge);
			delete this->challenge;
			this->challenge = nextChallenge;
			connection->setLogin(login);
			connection->setVerboseId(true); // ?
			if (desiredStatus != ID_STATUS_INVISIBLE) {
				connection->sendAvailableForChat();
			}

			debugLogA("Set status to online");
			this->m_iStatus = desiredStatus;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_CONNECTING);
			this->ToggleStatusMenuItems(true);

			ForkThread(&WhatsAppProto::ProcessBuddyList, this);

			// #TODO Move out of try block. Exception is expected on disconnect
			bool cont = true;
			while (cont == true) {
				this->lastPongTime = time(NULL);
				cont = connection->read();
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
