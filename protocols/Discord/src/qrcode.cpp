/*
Copyright © 2016-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CDiscordRemoteAuth::CDiscordRemoteAuth()
{
	// generate key pair
   pCtx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
   EVP_PKEY_keygen_init(pCtx);
   EVP_PKEY_CTX_set_rsa_keygen_bits(pCtx, 2048);

   pKey = nullptr;
   EVP_PKEY_keygen(pCtx, &pKey);

   pDecCtx = EVP_PKEY_CTX_new(pKey, nullptr);
   EVP_PKEY_decrypt_init(pDecCtx);
   EVP_PKEY_CTX_set_rsa_padding(pDecCtx, RSA_PKCS1_OAEP_PADDING);
   EVP_PKEY_CTX_set_rsa_oaep_md(pDecCtx, EVP_sha256());
   EVP_PKEY_CTX_set_rsa_mgf1_md(pDecCtx, EVP_sha256());
}

CDiscordRemoteAuth::~CDiscordRemoteAuth()
{
   EVP_PKEY_free(pKey);
   EVP_PKEY_CTX_free(pCtx);
   EVP_PKEY_CTX_free(pDecCtx);   
}

MBinBuffer CDiscordRemoteAuth::Decrypt(const JSONNode &node)
{
   size_t cbLen;
   mir_ptr<uint8_t> pEncrypted((uint8_t*)mir_base64_decode(node.as_string().c_str(), &cbLen));
   if (!pEncrypted)
      return MBinBuffer();

   size_t outlen;
   if (EVP_PKEY_decrypt(pDecCtx, nullptr, &outlen, pEncrypted, cbLen) <= 0)
      return MBinBuffer();

   MBinBuffer ret(outlen);
   if (EVP_PKEY_decrypt(pDecCtx, ret.data(), &outlen, pEncrypted, cbLen) <= 0)
      return MBinBuffer();

   ret.assign(ret.data(), outlen);
   return ret;
}

CMStringA CDiscordRemoteAuth::GetPubKey()
{
   auto *bio = BIO_new(BIO_s_mem());
   PEM_write_bio_PUBKEY(bio, pKey);
   BUF_MEM *mem = nullptr;
   BIO_get_mem_ptr(bio, &mem);

   CMStringA szPubKey(mem->data, (int)mem->length);
   szPubKey.Delete(0, szPubKey.Find("\n") + 1);
   szPubKey.Truncate(szPubKey.Find("\n-"));
   szPubKey.Replace("\n", "");
   
   BIO_free(bio);
   return szPubKey;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::RemoteAuthThread(void *)
{
   m_pRemoteAuth = new CDiscordRemoteAuth();

   while (RemoteAuthWorker())
      ;

   delete m_pRemoteAuth;
   m_pRemoteAuth = nullptr;
}

bool CDiscordProto::RemoteAuthWorker()
{
   debugLogA("Entering RemoteAuthWorker()");

   bool bHasCookie = false;
   MHttpHeaders hdrs;
   hdrs.AddHeader("Origin", "https://discord.com");
   if (!m_szWSCookie.IsEmpty()) {
      bHasCookie = true;
      hdrs.AddHeader("Cookie", m_szWSCookie);
   }

   NLHR_PTR pReply(m_ws.connect(m_hGatewayNetlibUser, "wss://remote-auth-gateway.discord.gg/?encoding=json&v=2", &hdrs));
   if (pReply == nullptr) {
      debugLogA("Remote auth connection failed, exiting");
      return false;
   }

   m_szWSCookie = pReply->GetCookies();

   if (pReply->resultCode != 101) {
      // if there's no cookie & Miranda is bounced with error 404, simply apply the cookie and try again
      if (pReply->resultCode == 404 || pReply->resultCode == 403) {
         if (!bHasCookie)
            return true;

         m_szWSCookie.Empty(); // don't use the same cookie twice
      }
      return false;
   }

   m_bConnected = true;
   m_ws.run();
   m_bConnected = false;
   debugLogA("Leaving RemoteAuthWorker()");
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CQRCodeDlg::OnDestroy()
{
   m_proto->m_pQRDlg = nullptr;

   if (!m_bSucceeded)
      m_proto->ShutdownSession();
}

void CQRCodeDlg::SetData(const CMStringA &str)
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

static INT_PTR CALLBACK LaunchDialog(void *param)
{
   auto *ppro = (CDiscordProto *)param;

   if (auto *pDlg = ppro->m_pQRDlg) {
      SetForegroundWindow(pDlg->GetHwnd());
      SetActiveWindow(pDlg->GetHwnd());
   }
   else {
      ppro->m_pQRDlg = new CQRCodeDlg(ppro);
      ppro->m_pQRDlg->Show();
   }
   return 0;
}

void CDiscordProto::RemoteAuthProcess(const JSONNode &node)
{
   auto op = node["op"].as_string();
   if (op == "hello") {
      int interval = node["heartbeat_interval"].as_int();
      m_impl.m_heartBeat.StartSafe(interval);

      JSONNode ret;
      GatewaySend(ret << CHAR_PARAM("op", "init") << CHAR_PARAM("encoded_public_key", m_pRemoteAuth->GetPubKey()));
   }
   else if (op == "nonce_proof") {
      MBinBuffer nonce(m_pRemoteAuth->Decrypt(node["encrypted_nonce"]));
      if (nonce.length()) {
         CMStringA szNonce(ptrA(mir_base64_encode(nonce)));
         szNonce.Replace("/", "_");
         szNonce.Replace("+", "-");
         szNonce.Replace("=", "");

         JSONNode ret;
         GatewaySend(ret << CHAR_PARAM("op", "nonce_proof") << CHAR_PARAM("nonce", szNonce));
      }
      else ShutdownSession();
   }
   else if (op == "pending_remote_init") {
      CallFunctionSync(LaunchDialog, this);

      CMStringA szCode(node["fingerprint"].as_mstring());
      m_pQRDlg->SetData("https://discord.com/ra/" + szCode);
   }
   else if (op == "pending_ticket") {
      MBinBuffer payload(m_pRemoteAuth->Decrypt(node["encrypted_user_payload"]));
      if (payload.length()) {
         std::regex regex("(.+):(.+):(.+):(.+)");
         std::smatch match;
         std::string content((char *)payload.data(), payload.length());
         if (std::regex_search(content, match, regex)) {
            setId(DB_KEY_ID, m_ownId = _atoi64(std::string(match[1]).c_str()));
            setString(DB_KEY_DISCR, std::string(match[2]).c_str());
            setString(DB_KEY_AVHASH, std::string(match[3]).c_str());
            setString(DB_KEY_NICK, std::string(match[4]).c_str());
            return;  // success
         }
      }
      ShutdownSession();
   }
   else if (op == "pending_login") {
      if (auto &ticket = node["ticket"]) {
         JSONNode root;
         root << CHAR_PARAM("ticket", ticket.as_string().c_str());
         
         AsyncHttpRequest req(this, REQUEST_POST, "/users/@me/remote-auth/login", &CDiscordProto::OnReceiveToken, &root);
         NLHR_PTR reply(Netlib_HttpTransaction(m_hNetlibUser, &req));
         if (reply && reply->resultCode == 200) {
            JsonReply json(reply);
            if (auto &token = json.data()["encrypted_token"]) {
               MBinBuffer payload(m_pRemoteAuth->Decrypt(token));
               if (payload.length()) {
                  // success, we have a new token, let's write it down
                  CMStringA szToken((char *)payload.data(), (int)payload.length());
                  setString(DB_KEY_TOKEN, szToken);
                  m_szAccessToken = szToken.Detach();

                  if (m_pQRDlg)
                     m_pQRDlg->SetSuccess();
               }
            }
         }         
      }
      ShutdownSession();
   }
   else if (op == "cancel") {
      ShutdownSession();
   }
}
