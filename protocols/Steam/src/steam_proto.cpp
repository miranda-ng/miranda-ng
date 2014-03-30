#include "steam.h"

CSteamProto::CSteamProto(const char* protoName, const TCHAR* userName) :
	PROTO<CSteamProto>(protoName, userName)
{
	CreateProtoService(PS_CREATEACCMGRUI, &CSteamProto::OnAccountManagerInit);
}

CSteamProto::~CSteamProto()
{
}

MCONTACT __cdecl CSteamProto::AddToList(int flags, PROTOSEARCHRESULT* psr)
{
	return 0;
}

MCONTACT __cdecl CSteamProto::AddToListByEvent(int flags, int iContact, HANDLE hDbEvent)
{
	return 0;
}

int __cdecl CSteamProto::Authorize(HANDLE hDbEvent)
{
	return 0;
}

int __cdecl CSteamProto::AuthDeny(HANDLE hDbEvent, const TCHAR* szReason)
{
	return 0;
}

int __cdecl CSteamProto::AuthRecv(MCONTACT hContact, PROTORECVEVENT* pre)
{
	return 0;
}

int __cdecl CSteamProto::AuthRequest(MCONTACT hContact, const TCHAR* szMessage)
{
	return 0;
}

HANDLE __cdecl CSteamProto::FileAllow(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szPath ) 
{ 
	return 0;
}

int __cdecl CSteamProto::FileCancel(MCONTACT hContact, HANDLE hTransfer ) 
{
	return 0;
}

int __cdecl CSteamProto::FileDeny(MCONTACT hContact, HANDLE hTransfer, const TCHAR* szReason )
{
	return 0;
}

int __cdecl CSteamProto::FileResume( HANDLE hTransfer, int* action, const TCHAR** szFilename )
{
	return 0;
}

DWORD_PTR __cdecl CSteamProto:: GetCaps(int type, MCONTACT hContact)
{
	switch(type)
	{
	case PFLAGNUM_1:
		return PF1_IM;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAG_UNIQUEIDTEXT:
		return (DWORD_PTR)::Translate("Username");
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"Username";
	default:
		return 0;
	}
}

int __cdecl CSteamProto::GetInfo(MCONTACT hContact, int infoType ) { return 0; }

HANDLE __cdecl CSteamProto::SearchBasic(const TCHAR* id)
{
	return 0;
}

HANDLE __cdecl CSteamProto::SearchByEmail(const TCHAR* email)
{
	return 0;
}

HANDLE __cdecl CSteamProto::SearchByName(const TCHAR* nick, const TCHAR* firstName, const TCHAR* lastName)
{
	return 0;
}

HWND __cdecl CSteamProto::SearchAdvanced( HWND owner ) { return 0; }

HWND __cdecl CSteamProto::CreateExtendedSearchUI( HWND owner ){ return 0; }

int __cdecl CSteamProto::RecvContacts(MCONTACT hContact, PROTORECVEVENT* pre) 
{
	return 0;
}

int __cdecl CSteamProto::RecvFile(MCONTACT hContact, PROTORECVFILET* pre) 
{
	return 0;
}

int __cdecl CSteamProto::RecvMsg(MCONTACT hContact, PROTORECVEVENT* pre)
{
 return 0;
}

int __cdecl CSteamProto::RecvUrl(MCONTACT hContact, PROTORECVEVENT *) { return 0; }

int __cdecl CSteamProto::SendContacts(MCONTACT hContact, int flags, int nContacts, MCONTACT *hContactsList)
{
	return 0;
}

HANDLE __cdecl CSteamProto::SendFile(MCONTACT hContact, const TCHAR *szDescription, TCHAR **ppszFiles)
{
	return 0;
}

int __cdecl CSteamProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	return 0;
}

int __cdecl CSteamProto::SendUrl(MCONTACT hContact, int flags, const char *url) { return 0; }

int __cdecl CSteamProto::SetApparentMode(MCONTACT hContact, int mode) { return 0; }

NETLIBHTTPREQUEST *CSteamProto::LoginRequest(const char *username, const char *password, const char *timestamp, const char *captchagid, const char *captcha_text, const char *emailauth, const char *emailsteamid)
{
	HttpRequest request = HttpRequest(m_hNetlibUser, REQUEST_POST, "https://steamcommunity.com/login/dologin/");
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("Accept-Encoding", "deflate, gzip");

	CMStringA param;
	param.AppendFormat("username=%s", mir_urlEncode(username));
	param.AppendFormat("&password=%s", mir_urlEncode(password));
	param.AppendFormat("&captchagid=%s", captchagid);
	param.AppendFormat("&captcha_text=%s", mir_urlEncode(captcha_text));
	param.AppendFormat("&emailauth=%s", mir_urlEncode(emailauth));
	param.AppendFormat("&emailsteamid=%s", emailsteamid);
	param.AppendFormat("&rsatimestamp=%s", timestamp);

	request.SetData(param.GetBuffer(), param.GetLength());

	return request.Send();
}

#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/engine.h>

bool CSteamProto::Login()
{
	ptrA username(getStringA("Username"));

	HttpRequest request(m_hNetlibUser, REQUEST_GET, "https://steamcommunity.com/login/getrsakey");
	request.AddParameter("username", username);

	mir_ptr<NETLIBHTTPREQUEST> response(request.Send());

	if (!response || response->resultCode != HTTP_STATUS_OK)
		return false;

	JSONNODE *root = json_parse(response->pData), *node;
	if (!root)
		return false;

	node = json_get(root, "success");
	if (!json_as_bool(node))
		return false;

	node = json_get(root, "timestamp");
	ptrA timestamp = ptrA(mir_t2a(json_as_string(node)));

	node = json_get(root, "publickey_mod");
	const char *mod = mir_t2a(json_as_string(node));

	node = json_get(root, "publickey_exp");
	const char *exp = mir_t2a(json_as_string(node));

	const char *password = getStringA("Password");

	BIGNUM *modulus = BN_new();
	if (!BN_hex2bn(&modulus, mod))
	return false;

	BIGNUM *exponent = BN_new();
	if (!BN_hex2bn(&exponent, exp))
		return false;

	RSA *rsa = RSA_new();
	rsa->n = modulus;
	rsa->e = exponent;

	int size = RSA_size(rsa);
	BYTE *ePassword = (BYTE*)mir_calloc(size);
	if (RSA_public_encrypt((int)strlen(password), (const unsigned char*)password, ePassword, rsa, RSA_PKCS1_PADDING) < 0)
		return false;

	char *sPassword = mir_base64_encode(ePassword, size);

	bool captcha_needed, emailauth_needed;
	CMStringA captchagid("-1"), captcha_text, emailauth, emailsteamid;
	do
	{
		response = Authorize(username, sPassword, timestamp, captchagid.GetBuffer(), captcha_text.GetBuffer(), emailauth.GetBuffer(), emailsteamid.GetBuffer());
		if (!response || response->resultCode != HTTP_STATUS_OK)
			return false;

		root = json_parse(response->pData);

		node = json_get(root, "emailauth_needed");
		emailauth_needed = json_as_bool(node);
		if (emailauth_needed)
		{
			GuardParam guard;

			node = json_get(root, "emailsteamid");
			emailsteamid = ptrA(mir_t2a(json_as_string(node)));

			node = json_get(root, "emaildomain");
			strcpy(guard.domain, mir_u2a(json_as_string(node)));

			if (DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_GUARD),
				NULL,
				CSteamProto::GuardProc,
				(LPARAM)&guard) != 1)
				return false;

			emailauth = guard.code;
		}

		node = json_get(root, "captcha_needed");
		captcha_needed = json_as_bool(node);
		if (captcha_needed)
		{
			node = json_get(root, "captcha_gid");
			captchagid = ptrA(mir_t2a(json_as_string(node)));

			CMStringA url = CMStringA("https://steamcommunity.com/public/captcha.php?gid=") + captchagid;
			CallService(MS_UTILS_OPENURL, 0, (LPARAM)url.GetBuffer());

			//request = HttpRequest(m_hNetlibUser, REQUEST_GET, "https://steamcommunity.com/public/captcha.php?gid=");
			//request.AddUrlPart(gid);

			//response = request.Send();

			//if (!response || response->resultCode != HTTP_STATUS_OK)
			//	return false;

			CaptchaParam captcha;
			captcha.size = response->dataLength;
			captcha.data = (BYTE*)mir_alloc(response->dataLength);
			memcpy(captcha.data, response->pData, captcha.size);

			if (DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_CAPTCHA),
				NULL,
				CSteamProto::CaptchaProc,
				(LPARAM)&captcha) != 1)
				return false;

			captcha_text = captcha.text;
		}
		
		node = json_get(root, "success");
		if (!json_as_bool(node) && !emailauth_needed && !captcha_needed)
			return false;

	} while (emailauth_needed || captcha_needed);

	node = json_get(root, "success");
	if (!json_as_bool(node))
		return false;

	// {"success":true, "login_complete" : true, "transfer_url" : "https:\/\/store.steampowered.com\/\/login\/transfer", "transfer_parameters" : {"steamid":"*", "token" : "*", "remember_login" : false, "webcookie" : "*"}}

	return true;
}

bool CSteamProto::Logout()
{
	return true;
}

int CSteamProto::SetStatus(int new_status)
{
	if (new_status == m_iDesiredStatus)
		return 0;

	int old_status = m_iStatus;
	m_iDesiredStatus = new_status;

	if (new_status == ID_STATUS_OFFLINE)
	{
		Logout();
		m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;

		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

		if (!Miranda_Terminated())
		{
			/*this->SetAllContactStatus(ID_STATUS_OFFLINE);
			this->CloseAllChatSessions();*/
		}

		return 0;
	}
	else
	{
		if (old_status == ID_STATUS_OFFLINE/* && !this->IsOnline()*/)
		{
			this->m_iStatus = ID_STATUS_CONNECTING;
			/*if (!Login())
				return 0;*/
		}
		else
		{
			/*if ( this->account->IsOnline())
			{
				SetServerStatus(new_status);
				return 0;
			}*/

			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

			return 0;
		}
	}

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)old_status, m_iStatus);

	return 0;
}

HANDLE __cdecl CSteamProto::GetAwayMsg(MCONTACT hContact) { return 0; }
int __cdecl CSteamProto::RecvAwayMsg(MCONTACT hContact, int mode, PROTORECVEVENT *evt) { return 0; }
int __cdecl CSteamProto::SetAwayMsg(int m_iStatus, const TCHAR *msg) { return 0; }

int __cdecl CSteamProto::UserIsTyping(MCONTACT hContact, int type)
{
	return 0;
}

int __cdecl CSteamProto::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch (eventType)
	{
	case EV_PROTO_ONLOAD:
		return this->OnModulesLoaded(wParam, lParam);

	case EV_PROTO_ONEXIT:
		return this->OnPreShutdown(wParam, lParam);

	case EV_PROTO_ONOPTIONS:
		return this->OnOptionsInit(wParam, lParam);

	/*case EV_PROTO_ONCONTACTDELETED:
		return this->OnContactDeleted(wParam, lParam);*/

	/*case EV_PROTO_ONMENU:
		this->OnInitStatusMenu();
		break;*/
	}

	return 1;
}