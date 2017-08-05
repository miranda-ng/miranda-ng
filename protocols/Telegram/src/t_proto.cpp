/*
Copyright (c) 2015 Miranda NG project (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CTelegramProto::CTelegramProto(const char* protoName, const TCHAR* userName) : PROTO<CTelegramProto>(protoName, userName)
{
	TLS = new MirTLS(this);

	tgl_set_verbosity(TLS, 10);


	InitNetwork();
	InitCallbacks();

	extern struct tgl_timer_methods mtgl_libevent_timers;

	tgl_set_timer_methods(TLS, &mtgl_libevent_timers);

	tgl_set_rsa_key(TLS, "tgl.pub");

	TLS->base_path = Utils_ReplaceVars("%miranda_profilesdir%\\%miranda_profilename%\\TGL_Data\\");
	CreateDirectoryTree(TLS->base_path);

	tgl_set_download_directory(TLS, CMStringA(FORMAT, "%s\\Downloads\\", TLS->base_path));
	CreateDirectoryTree(TLS->downloads_directory);

	tgl_register_app_id(TLS, TELEGRAM_API_ID, TELEGRAM_API_HASH);
	tgl_set_app_version(TLS, g_szMirVer);

	tgl_init(TLS);

	bl_do_dc_option(TLS, 1, "", 0, TELEGRAM_API_SERVER, strlen(TELEGRAM_API_SERVER), 443);
	bl_do_set_working_dc(TLS, 1);
}

CTelegramProto::~CTelegramProto()
{
	tgl_free_all(TLS);
}

DWORD_PTR CTelegramProto::GetCaps(int type, MCONTACT)
{
	switch (type)
	{
	case PFLAGNUM_1:
		return PF1_IM | PF1_AUTHREQ | PF1_CHAT | PF1_BASICSEARCH | PF1_MODEMSG | PF1_FILE;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_3:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_FORCEADDED | PF4_NOAUTHDENYREASON | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE | PF4_OFFLINEFILES;
	case PFLAG_UNIQUEIDTEXT:
	case PFLAG_UNIQUEIDSETTING:
		return (DWORD_PTR)"ID";
	}
	return 0;
}

MCONTACT CTelegramProto::AddToList(int, PROTOSEARCHRESULT *psr)
{
	return 0;
}

MCONTACT CTelegramProto::AddToListByEvent(int, int, MEVENT hDbEvent)
{
	return 0;
}

int CTelegramProto::Authorize(MEVENT hDbEvent)
{
	return 0;
}

int CTelegramProto::AuthDeny(MEVENT hDbEvent, const TCHAR*)
{
	return 0;

}

int CTelegramProto::AuthRecv(MCONTACT, PROTORECVEVENT* pre)
{
	return 0;
}

int CTelegramProto::AuthRequest(MCONTACT hContact, const TCHAR *szMessage)
{
	return 0;
}

int CTelegramProto::GetInfo(MCONTACT hContact, int)
{
	return 0;
}

HANDLE CTelegramProto::SearchBasic(const TCHAR* id)
{
	return 0;
}

int CTelegramProto::SendMsg(MCONTACT hContact, int flags, const char *msg)
{
	return 0;
}

void LoginThread(void* p)
{
	tgl_login(((CTelegramProto*)p)->TLS);
}

int CTelegramProto::SetStatus(int iNewStatus)
{
	if (iNewStatus == ID_STATUS_ONLINE) 
		mir_forkthread(LoginThread, this);  
	return 0;
}

int CTelegramProto::UserIsTyping(MCONTACT hContact, int type)
{
	return 0;
}

int CTelegramProto::OnEvent(PROTOEVENTTYPE iEventType, WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int CTelegramProto::OnPreShutdown(WPARAM, LPARAM)
{
	SaveState();
	return 0;
}


void CTelegramProto::TGLGetValue(tgl_value_type type, const char *prompt, int num_values, char **result)
{
	switch (type)
	{
	case  tgl_phone_number:
		*result = getStringA("ID");
		if (*result)
			break;
	default:
		ENTER_STRING es = { sizeof(es) };
		es.type = ESF_MULTILINE;
		es.caption = mir_a2t(prompt);
		EnterString(&es);
		*result = mir_t2a(es.ptszResult);
		mir_free((void*)es.caption);
	};
}