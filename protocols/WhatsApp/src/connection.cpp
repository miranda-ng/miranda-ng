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
	m_szPhoneNumber = cc + in;
	m_szJid = m_szPhoneNumber + "@s.whatsapp.net";

	if (!getString(WHATSAPP_KEY_NICK, &dbv)) {
		m_szNick = dbv.pszVal;
		db_free(&dbv);
	}
	if (m_szNick.empty()) {
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
	bool error = false;

	this->conn = NULL;

	while (true) {
		if (m_pConnection != NULL) {
			delete m_pConnection;
			m_pConnection = NULL;
		}
		if (this->conn != NULL) {
			delete this->conn;
			this->conn = NULL;
		}

		if (m_iDesiredStatus == ID_STATUS_OFFLINE || error) {
			debugLogA("Set status to offline");
			SetAllContactStatuses(ID_STATUS_OFFLINE, true);
			this->ToggleStatusMenuItems(false);
			int prevStatus = m_iStatus;
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
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
			m_pConnection = new WAConnection(m_szPhoneNumber, resource, &this->connMutex, &writerMutex, this->conn, this, this);
			{
				WALogin login(m_pConnection, password);

				m_Challenge = login.login(m_Challenge);
				m_pConnection->setLogin(&login);
			}
			m_pConnection->nick = m_szNick;
			m_pConnection->setVerboseId(true);
			if (m_iDesiredStatus != ID_STATUS_INVISIBLE)
				m_pConnection->sendAvailableForChat();

			debugLogA("Set status to online");
			m_iStatus = m_iDesiredStatus;
			ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_CONNECTING);
			this->ToggleStatusMenuItems(true);

			// ProcessBuddyList(0);

			// #TODO Move out of try block. Exception is expected on disconnect
			while (true) {
				this->lastPongTime = time(NULL);
				if (!m_pConnection->read())
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
	while (WaitForSingleObjectEx(update_loop_lock_, 1000, true) == WAIT_TIMEOUT) {
		if (m_iStatus != ID_STATUS_OFFLINE && m_pConnection != NULL && m_iDesiredStatus == m_iStatus) {
			// #TODO Quiet after pong or tree read?
			int quietInterval = difftime(time(NULL), this->lastPongTime);
			if (quietInterval >= MAX_SILENT_INTERVAL) {
				try {
					debugLogA("send ping");
					this->lastPongTime = time(NULL);
					m_pConnection->sendPing();
				}
				catch (exception &e) {
					debugLogA("Exception: %s", e.what());
				}
			}
		}
	}
	ResetEvent(update_loop_lock_);
	debugLogA("Exiting sentinel loop");
}

void WhatsAppProto::onPing(const std::string& id)
{
	if (this->isOnline()) {
		try {
			debugLogA("Sending pong with id %s", id.c_str());
			m_pConnection->sendPong(id);
		}
		CODE_BLOCK_CATCH_ALL
	}
}
