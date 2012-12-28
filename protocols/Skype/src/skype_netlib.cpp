#include "skype_proto.h"
#include "base64/base64.h"

void CSkypeProto::InitNetLib()
{
	wchar_t name[128];
	::mir_sntprintf(name, SIZEOF(name), ::TranslateT("%s connection"), this->m_tszUserName);

	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = this->m_szModuleName;
	this->hNetLibUser = (HANDLE)::CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	this->Log(
		L"Setting protocol/module name to '%s/%s'", 
		_A2T(this->m_szProtoName), 
		_A2T(this->m_szModuleName));
}

void CSkypeProto::UninitNetLib()
{
	::Netlib_CloseHandle(this->hNetLibUser);
	this->hNetLibUser = NULL;
}

void CSkypeProto::InitProxy()
{
	if (this->hNetLibUser)
	{
		NETLIBUSERSETTINGS nlus = { sizeof(NETLIBUSERSETTINGS) };
		::CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)this->hNetLibUser, (LPARAM)&nlus);

		if (nlus.useProxy) 
		{
			char address[MAX_PATH];
			::mir_snprintf(address, MAX_PATH, "%s:%d", nlus.szProxyServer, nlus.wProxyPort);

			switch (nlus.proxyType)
			{
			case PROXYTYPE_HTTP:
			case PROXYTYPE_HTTPS:
				this->skype->SetInt(SETUPKEY_HTTPS_PROXY_ENABLE, 1);
				this->skype->SetInt(SETUPKEY_SOCKS_PROXY_ENABLE, 0);
				this->skype->SetStr(SETUPKEY_HTTPS_PROXY_ADDR, address);
				if (nlus.useProxyAuth)
				{
					char encodedPass[MAX_PATH];
					Base64::Encode(nlus.szProxyAuthPassword, encodedPass, MAX_PATH);

					this->skype->SetStr(SETUPKEY_HTTPS_PROXY_USER,	nlus.szProxyAuthUser);
					this->skype->SetStr(SETUPKEY_HTTPS_PROXY_PWD,	encodedPass);
				}
				break;

			case PROXYTYPE_SOCKS4:
			case PROXYTYPE_SOCKS5:
				this->skype->SetInt(SETUPKEY_HTTPS_PROXY_ENABLE, 0);
				this->skype->SetInt(SETUPKEY_SOCKS_PROXY_ENABLE, 1);
				this->skype->SetStr(SETUPKEY_SOCKS_PROXY_ADDR, address);
				if (nlus.useProxyAuth)
				{
					this->skype->SetStr(SETUPKEY_SOCKS_PROXY_USER,	nlus.szProxyAuthUser);
					this->skype->SetStr(SETUPKEY_SOCKS_PROXY_PWD,	nlus.szProxyAuthPassword);
				}
				break;

			default:
				this->skype->Delete(SETUPKEY_HTTPS_PROXY_ENABLE);
				this->skype->Delete(SETUPKEY_HTTPS_PROXY_ADDR);
				this->skype->Delete(SETUPKEY_HTTPS_PROXY_USER);
				this->skype->Delete(SETUPKEY_HTTPS_PROXY_PWD);
				this->skype->Delete(SETUPKEY_SOCKS_PROXY_ENABLE);
				this->skype->Delete(SETUPKEY_SOCKS_PROXY_ADDR);
				this->skype->Delete(SETUPKEY_SOCKS_PROXY_USER);
				this->skype->Delete(SETUPKEY_SOCKS_PROXY_PWD);
				break;
			}
		}
	}
}

void CSkypeProto::Log(const wchar_t *fmt, ...)
{
	va_list va;
	wchar_t msg[1024];

	va_start(va, fmt);
	::mir_vsntprintf(msg, sizeof(msg), fmt, va);
	va_end(va);

	::CallService(MS_NETLIB_LOGW, (WPARAM)this->hNetLibUser, (LPARAM)msg);
}