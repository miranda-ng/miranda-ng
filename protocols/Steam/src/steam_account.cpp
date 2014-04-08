#include "common.h"

WORD CSteamProto::SteamToMirandaStatus(int state)
{
	switch (state)
	{
	case 0: //Offline
		return ID_STATUS_OFFLINE;
	case 2: //Busy
		return ID_STATUS_DND;
	case 3: //Away
		return ID_STATUS_AWAY;
		/*case 4: //Snoozing
		prim = PURPLE_STATUS_EXTENDED_AWAY;
		break;
		case 5: //Looking to trade
		return "trade";
		case 6: //Looking to play
		return "play";*/
		//case 1: //Online
	default:
		return ID_STATUS_ONLINE;
	}
}

int CSteamProto::MirandaToSteamState(int status)
{
	switch (status)
	{
	case ID_STATUS_OFFLINE:
		return 0; //Offline
	case ID_STATUS_DND:
		return 2; //Busy
	case ID_STATUS_AWAY:
		return 3; //Away
		/*case 4: //Snoozing
		prim = PURPLE_STATUS_EXTENDED_AWAY;
		break;
		case 5: //Looking to trade
		return "trade";
		case 6: //Looking to play
		return "play";*/
		//case 1: //Online
	default:
		return ID_STATUS_ONLINE;
	}
}

bool CSteamProto::IsOnline()
{
	return m_iStatus > ID_STATUS_OFFLINE && m_hPollingThread;
}

bool CSteamProto::IsMe(const char *steamId)
{
	ptrA mySteamId(getStringA("SteamID"));
	if (!lstrcmpA(steamId, mySteamId))
		return true;

	return false;
}

void CSteamProto::SetServerStatusThread(void *arg)
{
	WORD status = *((WORD*)arg);

	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));

	int state = CSteamProto::MirandaToSteamState(status);

	// change status
	if (m_iStatus == state)
		return;

	int oldStatus = m_iStatus;
	m_iStatus = state;

	SteamWebApi::MessageApi::SendResult sendResult;
	SteamWebApi::MessageApi::SendStatus(m_hNetlibUser, token, sessionId, state, &sendResult);

	if (sendResult.IsSuccess())
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);
	else
		m_iStatus = oldStatus;
}

void CSteamProto::Authorize(SteamWebApi::AuthorizationApi::AuthResult *authResult)
{
	ptrW username(getWStringA("Username"));
	ptrA base64RsaEncryptedPassword;

	const wchar_t *nickname = getWStringA("Nick");
	if (lstrlen(nickname) == 0 && username)
		setWString("Nick", username);

	return;

	// get rsa public key
	SteamWebApi::RsaKeyApi::RsaKey rsaKey;
	SteamWebApi::RsaKeyApi::GetRsaKey(m_hNetlibUser, username, &rsaKey);
	if (!rsaKey.IsSuccess())
		return;

	ptrA password(getStringA("Password"));

	const char *pszModulus = rsaKey.GetModulus();
	DWORD cchModulus = strlen(pszModulus);

	// convert hex string to byte array
	DWORD cbLen = 0, dwSkip = 0, dwFlags = 0;
	if (!CryptStringToBinaryA(pszModulus, cchModulus, CRYPT_STRING_HEX, NULL, &cbLen, &dwSkip, &dwFlags))
	{
		debugLogA("CSteamProto::Authorize: Cannot get size of rsa modulus in binary (%lu)", GetLastError());
		return;
	}

	// Allocate a new buffer.
	BYTE *pbBuffer = (BYTE*)malloc(cbLen);
	if (!CryptStringToBinaryA(pszModulus, cchModulus, CRYPT_STRING_HEX, pbBuffer, &cbLen, &dwSkip, &dwFlags))
	{
		debugLogA("CSteamProto::Authorize: Cannot convert rsa modulus to binary (%lu)", GetLastError());
		return;
	}

	// reverse byte array, because of microsoft
	for (int i = 0; i < cbLen / 2; ++i)
	{
		BYTE temp = pbBuffer[cbLen - i - 1];
		pbBuffer[cbLen - i - 1] = pbBuffer[i];
		pbBuffer[i] = temp;
	}

	HCRYPTPROV hCSP = 0;
	if (!CryptAcquireContext(&hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_SILENT) &&
		!CryptAcquireContext(&hCSP, NULL, NULL, PROV_RSA_AES, CRYPT_SILENT | CRYPT_NEWKEYSET))
	{
		debugLogA("CSteamProto::Authorize: Cannot create rsa context (%lu)", GetLastError());
		return;
	}

	// Move the key into the key container.
	DWORD cbKeyBlob = sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY) + cbLen;
	BYTE *pKeyBlob = (BYTE*)malloc(cbKeyBlob);

	// Fill in the data.
	PUBLICKEYSTRUC *pPublicKey = (PUBLICKEYSTRUC*)pKeyBlob;
	pPublicKey->bType = PUBLICKEYBLOB; 
	pPublicKey->bVersion = CUR_BLOB_VERSION;  // Always use this value.
	pPublicKey->reserved = 0;                 // Must be zero.
	pPublicKey->aiKeyAlg = CALG_RSA_KEYX;     // RSA public-key key exchange. 

	// The next block of data is the RSAPUBKEY structure.
	RSAPUBKEY *pRsaPubKey = (RSAPUBKEY*)(pKeyBlob + sizeof(PUBLICKEYSTRUC));
	pRsaPubKey->magic = 0x31415352; // RSA1 // Use public key
	pRsaPubKey->bitlen = cbLen * 8;  // Number of bits in the modulus.
	pRsaPubKey->pubexp = 0x10001; // "010001" // Exponent.

	// Copy the modulus into the blob. Put the modulus directly after the
	// RSAPUBKEY structure in the blob.
	BYTE *pKey = (BYTE*)(((BYTE *)pRsaPubKey) + sizeof(RSAPUBKEY));
	//pKeyBlob + sizeof(BLOBHEADER)+ sizeof(RSAPUBKEY); 
	memcpy(pKey, pbBuffer, cbLen);

	// Now import public key
	HCRYPTKEY phKey = 0;
	if (!CryptImportKey(hCSP, pKeyBlob, cbKeyBlob, 0, 0, &phKey))
	{
		debugLogA("CSteamProto::Authorize: Cannot import rsa key to context (%lu)", GetLastError());
		return;
	}

	// encrypt password
	DWORD encryptedSize = 0;
	DWORD pwdLength = strlen(password);
	if (!CryptEncrypt(phKey, 0, TRUE, 0, NULL, &encryptedSize, pwdLength))
	{
		debugLogA("CSteamProto::Authorize: Cannot get rsa encrypt length (%lu)", GetLastError());
		return;
	}

	BYTE *encryptedPassword = (BYTE*)mir_calloc(encryptedSize);
	memcpy(encryptedPassword, password, pwdLength);
	if (!CryptEncrypt(phKey, 0, TRUE, 0, encryptedPassword, &pwdLength, encryptedSize))
	{
		debugLogA("CSteamProto::Authorize: Cannot rsa encrypt password (%lu)", GetLastError());
		return;
	}

	// reverse byte array again
	for (int i = 0; i < encryptedSize / 2; ++i)
	{
		BYTE temp = encryptedPassword[encryptedSize - i - 1];
		encryptedPassword[encryptedSize - i - 1] = encryptedPassword[i];
		encryptedPassword[i] = temp;
	}

	// save rsa encrypted password to db
	db_set_blob(NULL, this->m_szModuleName, "EncryptedPassword", encryptedPassword, encryptedSize);

	CryptDestroyKey(phKey);
	free(pKeyBlob);
	CryptReleaseContext(hCSP, 0);
	free(pbBuffer);

	base64RsaEncryptedPassword = mir_base64_encode(encryptedPassword, encryptedSize);
	mir_free(encryptedPassword);

	// try to authorize
	SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, base64RsaEncryptedPassword, rsaKey.GetTimestamp(), authResult);
	if (authResult->IsEmailAuthNeeded() || authResult->IsCaptchaNeeded())
	{
		do
		{
			if (authResult->IsEmailAuthNeeded())
			{
				GuardParam guard;

				lstrcpyA(guard.domain, authResult->GetEmailDomain());

				if (DialogBoxParam(
					g_hInstance,
					MAKEINTRESOURCE(IDD_GUARD),
					NULL,
					CSteamProto::GuardProc,
					(LPARAM)&guard) != 1)
					break;

				authResult->SetAuthCode(guard.code);
			}
				
			if (authResult->IsCaptchaNeeded())
			{
				// todo: show captcha dialog
			}

			// try to authorize with emailauthcode or captcha taxt
			SteamWebApi::AuthorizationApi::Authorize(m_hNetlibUser, username, base64RsaEncryptedPassword, rsaKey.GetTimestamp(), authResult);
		} while (authResult->IsEmailAuthNeeded() || authResult->IsCaptchaNeeded());
	}
}

void CSteamProto::LogInThread(void* param)
{
	if (this->IsOnline())
		return;

	CMStringA token(getStringA("TokenSecret"));
	if (token.IsEmpty())
	{
		SteamWebApi::AuthorizationApi::AuthResult authResult;
		Authorize(&authResult);
		// if some error
		if (!authResult.IsSuccess())
		{
			// todo: dosplay error message from authResult.GetMessage()
			//ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_BADUSERID);
			debugLogA("CSteamProto::Authorize: Error (%s)", authResult.GetMessage());

			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
			return;
		}

		token = authResult.GetToken();
		setString("TokenSecret", token);
		setString("Cookie", authResult.GetCookie());
		setString("SteamID", authResult.GetSteamid());
	}

	SteamWebApi::LoginApi::LoginResult loginResult;
	SteamWebApi::LoginApi::Logon(m_hNetlibUser, token, &loginResult);
	// if some error
	if (!loginResult.IsSuccess())
	{
		debugLogA("CSteamProto::login: Error");
		// set status to offline
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, ID_STATUS_OFFLINE);
		return;
	}
	else
	{
		setString("SessionID", loginResult.GetSessionId());
		setDword("MessageID", loginResult.GetMessageId());
		// set selected status
		m_iStatus = m_iDesiredStatus;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)ID_STATUS_CONNECTING, m_iDesiredStatus);
	}

	// get contact list
	SteamWebApi::FriendListApi::FriendList friendList;
	SteamWebApi::FriendListApi::Load(m_hNetlibUser, token, loginResult.GetSteamId(), &friendList);
	
	if (friendList.IsSuccess())
	{
		for (int i = 0; i < friendList.GetCount(); i++)
		{
			if (!FindContact(friendList[i]))
			{
				SteamWebApi::FriendApi::Friend rFriend;
				SteamWebApi::FriendApi::LoadSummaries(m_hNetlibUser, token, friendList[i], &rFriend);
				if (!rFriend.IsSuccess()) continue;
				AddContact(rFriend);
			}
		}
	}

	// start pooling thread
	if (m_hPollingThread == NULL && !m_bTerminated)
	{
		m_bTerminated = false;
		m_hPollingThread = ForkThreadEx(&CSteamProto::PollingThread, NULL, NULL);
	}
}

void CSteamProto::LogOutThread(void*)
{
	ptrA token(getStringA("TokenSecret"));
	ptrA sessionId(getStringA("SessionID"));

	SteamWebApi::LoginApi::Logoff(m_hNetlibUser, token, sessionId);

	delSetting("SessionID");
}