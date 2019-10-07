/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019 George Hazan

*/

#include "stdafx.h"

class CWhatsAppQRDlg : public CProtoDlgBase<WhatsAppProto>
{
	QRcode *m_qr;

public:
	CWhatsAppQRDlg(WhatsAppProto *ppro, const CMStringA &szData) :
		CProtoDlgBase<WhatsAppProto>(ppro, IDD_SHOWQR)
	{
		m_qr = QRcode_encodeString(szData, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
	}

	~CWhatsAppQRDlg()
	{
		QRcode_free(m_qr);
	}

	bool OnInitDialog() override
	{
		HWND hwndRc = GetDlgItem(m_hwnd, IDC_QRPIC);
		RECT rc;
		GetClientRect(hwndRc, &rc);

		::SetForegroundWindow(m_hwnd);

		int scale = 8; // (rc.bottom - rc.top) / m_qr->width;
		int rowLen = m_qr->width * scale * 3;
		if (rowLen % 4)
			rowLen = (rowLen / 4 + 1) * 4;
		int dataLen = rowLen * m_qr->width * scale;

		mir_ptr<BYTE> pData((BYTE *)mir_alloc(dataLen));
		if (pData == nullptr)
			return false;

		memset(pData, 0xFF, dataLen); // white background by default

		const BYTE *s = m_qr->data;
		for (int y = 0; y < m_qr->width; y++) {
			BYTE *d = pData.get() + rowLen * y * scale;
			for (int x = 0; x < m_qr->width; x++) {
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
		bih.biWidth = m_qr->width * scale;
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
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int random_func(uint8_t *pData, size_t size, void *)
{
	Utils_GetRandom(pData, size);
	return 0;
}

bool WhatsAppProto::ShowQrCode()
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

	CMStringA szQrData(FORMAT, "%s,%s,%s", m_szJid.c_str(), szPubKey.c_str(), m_szClientId.c_str());
	CWhatsAppQRDlg(this, szQrData).DoModal();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ServerThread(void *)
{
	m_pConn = new WAConnection();

	ptrA szClientToken(getStringA(DBKEY_CLIENT_SECRET));
	if (szClientToken == nullptr) {
		if (!ShowQrCode()) {
			delete m_pConn;
			return;
		}
	}
}