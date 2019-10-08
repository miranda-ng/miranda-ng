/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019 George Hazan

*/

#include "stdafx.h"

class CWhatsAppQRDlg : public CProtoDlgBase<WhatsAppProto>
{
public:
	CWhatsAppQRDlg(WhatsAppProto *ppro) :
		CProtoDlgBase<WhatsAppProto>(ppro, IDD_SHOWQR)
	{}

	void SetData(const CMStringA &str)
	{
		auto *pQR = QRcode_encodeString(str, 0, QR_ECLEVEL_L, QR_MODE_8, 1);

		HWND hwndRc = GetDlgItem(m_hwnd, IDC_QRPIC);
		RECT rc;
		GetClientRect(hwndRc, &rc);

		::SetForegroundWindow(m_hwnd);

		int scale = 8; // (rc.bottom - rc.top) / pQR->width;
		int rowLen = pQR->width * scale * 3;
		if (rowLen % 4)
			rowLen = (rowLen / 4 + 1) * 4;
		int dataLen = rowLen * pQR->width * scale;

		mir_ptr<BYTE> pData((BYTE *)mir_alloc(dataLen));
		if (pData == nullptr) {
			QRcode_free(pQR);
			return;
		}

		memset(pData, 0xFF, dataLen); // white background by default

		const BYTE *s = pQR->data;
		for (int y = 0; y < pQR->width; y++) {
			BYTE *d = pData.get() + rowLen * y * scale;
			for (int x = 0; x < pQR->width; x++) {
				if (*s & 1)
					for (int i = 0; i < scale; i++)
						for (int j = 0; j < scale; j++) {
							d[j * 3 + i * rowLen] = 0;
							d[1 + j * 3 + i * rowLen] = 0;
							d[2 + j * 3 + i * rowLen] = 0;
						}

				d += scale * 3;
				s++;
			}
		}

		BITMAPFILEHEADER fih = {};
		fih.bfType = 0x4d42;  // "BM"
		fih.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataLen;
		fih.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER bih = {};
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = pQR->width * scale;
		bih.biHeight = -bih.biWidth;
		bih.biPlanes = 1;
		bih.biBitCount = 24;
		bih.biCompression = BI_RGB;

		wchar_t wszTempPath[MAX_PATH], wszTempFile[MAX_PATH];
		GetTempPathW(_countof(wszTempPath), wszTempPath);
		GetTempFileNameW(wszTempPath, L"wa_", TRUE, wszTempFile);
		FILE *f = _wfopen(wszTempFile, L"wb");
		fwrite(&fih, sizeof(BITMAPFILEHEADER), 1, f);
		fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, f);
		fwrite(pData, sizeof(unsigned char), dataLen, f);
  		fclose(f);

		SendMessage(hwndRc, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Image_Load(wszTempFile));
		
		DeleteFileW(wszTempFile);
		QRcode_free(pQR);
	}
};

static INT_PTR __stdcall sttShowDialog(void *param)
{
	WhatsAppProto *ppro = (WhatsAppProto *)param;

	if (ppro->m_pQRDlg == nullptr) {
		ppro->m_pQRDlg = new CWhatsAppQRDlg(ppro);
		ppro->m_pQRDlg->Show();
	}
	else {
		SetForegroundWindow(ppro->m_pQRDlg->GetHwnd());
		SetActiveWindow(ppro->m_pQRDlg->GetHwnd());
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int random_func(uint8_t *pData, size_t size, void *)
{
	Utils_GetRandom(pData, size);
	return 0;
}

bool WhatsAppProto::ShowQrCode(const CMStringA &ref)
{
	CMStringA szPubKey(getMStringA(DBKEY_PUBKEY));
	if (szPubKey.IsEmpty()) {
		// generate new pair of private & public keys for this account
		signal_context *pTmpCtx;
		signal_context_create(&pTmpCtx, nullptr);

		signal_crypto_provider prov;
		memset(&prov, 0xFF, sizeof(prov));
		prov.random_func = random_func;
		signal_context_set_crypto_provider(pTmpCtx, &prov);

		ec_key_pair *pKeys;
		if (curve_generate_key_pair(pTmpCtx, &pKeys)) {
			signal_context_destroy(pTmpCtx);
			return false;
		}

		auto *pPubKey = ec_key_pair_get_public(pKeys);
		signal_buffer *pBuf;
		ec_public_key_serialize(&pBuf, pPubKey);
		szPubKey = ptrA(mir_base64_encode(&pBuf->data, pBuf->len));
		signal_buffer_free(pBuf);
		setString(DBKEY_PUBKEY, szPubKey);

		auto *pPrivKey = ec_key_pair_get_private(pKeys);
		ec_private_key_serialize(&pBuf, pPrivKey);
		CMStringA szPrivKey(ptrA(mir_base64_encode(&pBuf->data, pBuf->len)));
		signal_buffer_free(pBuf);
		setString(DBKEY_PRIVATEKEY, szPrivKey);
	}

	CallFunctionSync(sttShowDialog, this);

	CMStringA szQrData(FORMAT, "%s,%s,%s", ref.c_str(), szPubKey.c_str(), m_szClientId.c_str());
	m_pQRDlg->SetData(szQrData);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////
// sends a piece of JSON to a server via a websocket, masked

int WhatsAppProto::WSSend(const CMStringA &str, WA_PKT_HANDLER pHandler)
{
	if (m_hServerConn == nullptr)
		return -1;

	int pktId = ++m_iPktNumber;

	CMStringA buf;
	buf.Format("%d.--%d,", (int)m_iLoginTime, pktId);
	if (!str.IsEmpty()) {
		buf.AppendChar(',');
		buf += str;
	}

	if (pHandler != nullptr) {
		auto *pReq = new WARequest;
		pReq->issued = time(0);
		pReq->pHandler = pHandler;
		pReq->pktId = pktId;

		mir_cslock lck(m_csPacketQueue);
		m_arPacketQueue.insert(pReq);
	}

	debugLogA("Sending packet #%d: %s", pktId, buf.c_str());
	WebSocket_Send(m_hServerConn, buf.c_str(), buf.GetLength());
	return pktId;
}

void WhatsAppProto::OnLoggedIn()
{
	debugLogA("CDiscordProto::OnLoggedIn");
	m_bOnline = true;
	// SetServerStatus(m_iDesiredStatus);
}

void WhatsAppProto::OnLoggedOut(void)
{
	debugLogA("CDiscordProto::OnLoggedOut");
	m_bOnline = false;
	m_bTerminated = true;
	m_iPktNumber = 0;

	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)m_iStatus, ID_STATUS_OFFLINE);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

	setAllContactStatuses(ID_STATUS_OFFLINE, false);
}

void WhatsAppProto::RestoreSession()
{
}

void WhatsAppProto::ShutdownSession()
{
	if (m_bTerminated)
		return;

	debugLogA("WhatsAppProto::ShutdownSession");

	// shutdown all resources
	if (m_hServerConn)
		Netlib_Shutdown(m_hServerConn);

	OnLoggedOut();
}

void WhatsAppProto::StartSession()
{
	CMStringA payload(FORMAT, "[\"admin\",\"init\",[0,3,4940],[\"Windows\",\"Chrome\",\"10\"],\"%s\",true]", m_szClientId.c_str());
	WSSend(payload, &WhatsAppProto::OnStartSession);
}

void WhatsAppProto::OnStartSession(const JSONNode &root)
{
	int status = root["status"].as_int();
	if (status != 200) {
		debugLogA("Session start failed with error %d", status);
		ShutdownSession();
		return;
	}

	CMStringA ref = root["ref"].as_mstring();
	ShowQrCode(ref);
}

//////////////////////////////////////////////////////////////////////////////////////
// gateway worker thread

void WhatsAppProto::ServerThread(void *)
{
	while (ServerThreadWorker())
		;
	ShutdownSession();
}

bool WhatsAppProto::ServerThreadWorker()
{
	NETLIBHTTPHEADER hdrs[] =
	{
		{ "Origin", "https://web.whatsapp.com" },
		{ "Sec-WebSocket-Key", "k/hwJLznKpk3p2hxyYGzWA==" },
		{ 0, 0 }
	};

	m_hServerConn = WebSocket_Connect(m_hNetlibUser, "web.whatsapp.com/ws", hdrs);
	if (m_hServerConn == nullptr) {
		debugLogA("Server connection failed, exiting");
		return false;
	}

	debugLogA("Server connection succeeded");

	m_iLoginTime = time(0);
	m_szClientToken = getMStringA(DBKEY_CLIENT_TOKEN);
	if (m_szClientToken.IsEmpty())
		StartSession();
	else
		RestoreSession();

	bool bExit = false;
	int offset = 0;
	MBinBuffer netbuf;

	while (!bExit) {
		if (m_bTerminated)
			break;

		unsigned char buf[2048];
		int bufSize = Netlib_Recv(m_hServerConn, (char *)buf + offset, _countof(buf) - offset, MSG_NODUMP);
		if (bufSize == 0) {
			debugLogA("Gateway connection gracefully closed");
			bExit = !m_bTerminated;
			break;
		}
		if (bufSize < 0) {
			debugLogA("Gateway connection error, exiting");
			break;
		}

		WSHeader hdr;
		if (!WebSocket_InitHeader(hdr, buf, bufSize)) {
			offset += bufSize;
			continue;
		}
		offset = 0;

		debugLogA("Got packet: buffer = %d, opcode = %d, headerSize = %d, final = %d, masked = %d", bufSize, hdr.opCode, hdr.headerSize, hdr.bIsFinal, hdr.bIsMasked);

		// we have some additional data, not only opcode
		if ((size_t)bufSize > hdr.headerSize) {
			size_t currPacketSize = bufSize - hdr.headerSize;
			netbuf.append(buf, bufSize);
			while (currPacketSize < hdr.payloadSize) {
				int result = Netlib_Recv(m_hServerConn, (char *)buf, _countof(buf), MSG_NODUMP);
				if (result == 0) {
					debugLogA("Gateway connection gracefully closed");
					bExit = !m_bTerminated;
					break;
				}
				if (result < 0) {
					debugLogA("Gateway connection error, exiting");
					break;
				}
				currPacketSize += result;
				netbuf.append(buf, result);
			}
		}

		// read all payloads from the current buffer, one by one
		size_t prevSize = 0;
		while (true) {
			switch (hdr.opCode) {
			case 0: // text packet
			case 1: // binary packet
			case 2: // continuation
				if (hdr.bIsFinal) {
					// process a packet here
					CMStringA szJson(netbuf.data() + hdr.headerSize, (int)hdr.payloadSize);
					debugLogA("JSON received:\n%s", szJson.c_str());

					int pos = szJson.Find(',');
					if (pos != -1) {
						CMStringA szPrefix = szJson.Left(pos);
						szJson.Delete(0, pos+1);

						JSONNode root = JSONNode::parse(szJson);
						if (root) {
							int sessId, pktId;
							if (sscanf(szPrefix, "%d.--%d,", &sessId, &pktId) == 2) {
								auto *pReq = m_arPacketQueue.find((WARequest *)&pktId);
								if (pReq != nullptr) {
									(this->*pReq->pHandler)(root);
								}
							}
							else ProcessPacket(root);
						}
					}
				}
				break;

			case 8: // close
				debugLogA("server required to exit");
				bExit = true; // simply reconnect, don't exit
				break;

			case 9: // ping
				debugLogA("ping received");
				Netlib_Send(m_hServerConn, (char *)buf + hdr.headerSize, bufSize - int(hdr.headerSize), 0);
				break;
			}

			if (hdr.bIsFinal)
				netbuf.remove(hdr.headerSize + hdr.payloadSize);

			if (netbuf.length() == 0)
				break;

			// if we have not enough data for header, continue reading
			if (!WebSocket_InitHeader(hdr, netbuf.data(), netbuf.length()))
				break;

			// if we have not enough data for data, continue reading
			if (hdr.headerSize + hdr.payloadSize > netbuf.length())
				break;

			debugLogA("Got inner packet: buffer = %d, opcode = %d, headerSize = %d, payloadSize = %d, final = %d, masked = %d", netbuf.length(), hdr.opCode, hdr.headerSize, hdr.payloadSize, hdr.bIsFinal, hdr.bIsMasked);
			if (prevSize == netbuf.length()) {
				netbuf.remove(prevSize);
				debugLogA("dropping current packet, exiting");
				break;
			}

			prevSize = netbuf.length();
		}
	}

	Netlib_CloseHandle(m_hServerConn);
	m_hServerConn = nullptr;
	return bExit;
}

void WhatsAppProto::ProcessPacket(const JSONNode &root)
{
	CMStringA szType = root[(size_t)0].as_mstring();
	const JSONNode &content = root[1];

	if (szType == "Conn")
		ProcessConn(content);
}

void WhatsAppProto::ProcessConn(const JSONNode &root)
{
	if (m_pQRDlg) {
		m_pQRDlg->Close();
		m_pQRDlg = nullptr;
	}

	m_szJid = root["wid"].as_mstring();
	setString(DBKEY_ID, m_szJid);

	CMStringA szSecret(root["secret"].as_mstring());
	size_t secretLen;
	void *pSecret = mir_base64_decode(szSecret, &secretLen);

	setWString(DBKEY_NICK, root["pushname"].as_mstring());
	setWString(DBKEY_CLIENT_TOKEN, root["clientToken"].as_mstring());
	setWString(DBKEY_SERVER_TOKEN, root["serverToken"].as_mstring());
	setWString(DBKEY_BROWSER_TOKEN, root["browserToken"].as_mstring());

	OnLoggedIn();
}
