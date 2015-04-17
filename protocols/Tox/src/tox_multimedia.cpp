#include "common.h"

/* AUDIO RECEIVING */

CToxAudioCall::CToxAudioCall(CToxProto *proto, int callId) :
CToxDlgBase(proto, IDD_AUDIO, false), callId(callId),
ok(this, IDOK), cancel(this, IDCANCEL)
{
	m_autoClose = CLOSE_ON_CANCEL;
	ok.OnClick = Callback(this, &CToxAudioCall::OnOk);
	cancel.OnClick = Callback(this, &CToxAudioCall::OnCancel);
}

void CToxAudioCall::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, "audio_start");
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon(iconName, 16));
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIcon(iconName, 32));

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "AudioCallWindow");
}

void CToxAudioCall::OnOk(CCtrlBase*)
{
	ok.Enable(FALSE);
	ToxAvCSettings cSettings;// = av_DefaultSettings;
	//cSettings.call_type = av_TypeAudio;
	toxav_get_peer_csettings(m_proto->toxAv, callId, 0, &cSettings);

	toxav_answer(m_proto->toxAv, callId, &cSettings);
}

void CToxAudioCall::OnCancel(CCtrlBase*)
{
	if (ok.Enabled())
		toxav_reject(m_proto->toxAv, callId, NULL);
	else
		toxav_stop_call(m_proto->toxAv, callId);
}

void CToxAudioCall::OnClose()
{
	WindowList_Remove(m_proto->hAudioDialogs, m_hwnd);
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "AudioCallWindow");
}

INT_PTR CToxAudioCall::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_AUDIO_END)
		Close();

	return CToxDlgBase::DlgProc(msg, wParam, lParam);
}

// incoming call flow
void CToxProto::OnAvInvite(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to get contact");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	ToxAvCSettings dest;
	if (toxav_get_peer_csettings(proto->toxAv, callId, 0, &dest) != av_ErrorNone)
	{
		proto->debugLogA(__FUNCTION__": failed to get codec settings");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	if (dest.call_type != av_TypeAudio)
	{
		proto->debugLogA(__FUNCTION__": video call is unsupported");
		toxav_reject(proto->toxAv, callId, Translate("Video call is unsupported"));
		return;
	}

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = time(NULL);
	recv.lParam = callId;
	recv.flags = PREF_UTF;
	recv.szMessage = mir_utf8encodeT(TranslateT("Incoming audio call"));
	ProtoChainRecv(hContact, PSR_AUDIO, hContact, (LPARAM)&recv);
}

// save event to db
INT_PTR CToxProto::OnRecvAudioCall(WPARAM hContact, LPARAM lParam)
{
	PROTORECVEVENT *pre = (PROTORECVEVENT*)lParam;

	calls[hContact] = pre->lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = DBEF_UTF;
	dbei.eventType = DB_EVENT_AUDIO_RING;
	dbei.cbBlob = (DWORD)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;
	MEVENT hEvent = db_event_add(hContact, &dbei);

	CLISTEVENT cle = { sizeof(cle) };
	cle.flags |= CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = hEvent;
	cle.hIcon = Skin_GetIconByHandle(GetIconHandle("audio_ring"));

	TCHAR szTooltip[256];
	mir_sntprintf(szTooltip, SIZEOF(szTooltip), _T("%s %s %s"), TranslateT("Incoming audio call"), TranslateT("from"), pcli->pfnGetContactDisplayName(hContact, 0));
	cle.ptszTooltip = szTooltip;

	char szService[256];
	mir_snprintf(szService, SIZEOF(szService), "%s/Audio/Ring", GetContactProto(hContact));
	cle.pszService = szService;

	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);

	return hEvent;
}

// 
INT_PTR CToxProto::OnAudioRing(WPARAM hContact, LPARAM lParam)
{
	CToxAudioCall *audioCall = new CToxAudioCall(this, calls[hContact]);
	audioCall->Show();
	WindowList_Add(hAudioDialogs, audioCall->GetHwnd(), hContact);
	return 0;
}

void CToxProto::OnAvRinging(void*, int32_t callId, void *arg)
{
}

void CToxProto::OnAvStart(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to get contact");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = proto->m_szModuleName;
	dbei.timestamp = time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = DB_EVENT_AUDIO_START;
	dbei.pBlob = (PBYTE)mir_utf8encodeT(TranslateT("Audio call start"));;
	dbei.cbBlob = (DWORD)mir_strlen((char*)dbei.pBlob) + 1;
	db_event_add(hContact, &dbei);

	if (toxav_prepare_transmission(proto->toxAv, callId, false) == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to prepare transmition");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}
}

void CToxProto::OnAvEnd(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to get contact");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = proto->m_szModuleName;
	dbei.timestamp = time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = DB_EVENT_AUDIO_END;
	dbei.pBlob = (PBYTE)mir_utf8encodeT(TranslateT("Audio call end"));;
	dbei.cbBlob = (DWORD)mir_strlen((char*)dbei.pBlob) + 1;
	db_event_add(hContact, &dbei);

	WindowList_Broadcast(proto->hAudioDialogs, WM_AUDIO_END, 0, 0);
}

void CToxProto::OnAvReject(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to get contact");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = proto->m_szModuleName;
	dbei.timestamp = time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = DB_EVENT_AUDIO_END;
	dbei.pBlob = (PBYTE)mir_utf8encodeT(TranslateT("Audio call end"));;
	dbei.cbBlob = (DWORD)mir_strlen((char*)dbei.pBlob) + 1;
	db_event_add(hContact, &dbei);
}

void CToxProto::OnAvCancel(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to get contact");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = proto->m_szModuleName;
	dbei.timestamp = time(NULL);
	dbei.flags = DBEF_UTF;
	dbei.eventType = DB_EVENT_AUDIO_END;
	dbei.pBlob = (PBYTE)mir_utf8encodeT(TranslateT("Audio call end"));;
	dbei.cbBlob = (DWORD)mir_strlen((char*)dbei.pBlob) + 1;
	db_event_add(hContact, &dbei);

	WindowList_Broadcast(proto->hAudioDialogs, WM_AUDIO_END, 0, 0);
}

void CToxProto::OnAvCsChange(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvRequestTimeout(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvPeerTimeout(void*, int32_t callId, void *arg) { }

void CToxProto::OnFriendAudio(void*, int32_t callId, const int16_t *PCM, uint16_t size, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxAvCSettings dest;
	int err = toxav_get_peer_csettings(proto->toxAv, callId, 0, &dest);
	if (err != av_ErrorNone)
	{
		proto->debugLogA(__FUNCTION__": failed to get codec settings");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	if (dest.call_type != av_TypeAudio)
	{
		proto->debugLogA(__FUNCTION__": failed to play video");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	WAVEFORMATEX wfx = { 0 };
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = dest.audio_channels;
	wfx.wBitsPerSample = dest.audio_bitrate / 1000;
	wfx.nSamplesPerSec = dest.audio_sample_rate;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	DWORD deviceId = proto->getDword("AudioOutputDeviceID", TOX_ERROR);
	if (deviceId == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get device id");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	HWAVEOUT hDevice;
	MMRESULT result = waveOutOpen(&hDevice, deviceId, &wfx, 0, 0, CALLBACK_NULL);
	if (result != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to open audio device");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	WAVEHDR header = { 0 };
	header.lpData = (LPSTR)PCM;
	header.dwBufferLength = size;

	result = waveOutPrepareHeader(hDevice, &header, sizeof(WAVEHDR));
	if (result != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to prepare audio device header");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	result = waveOutWrite(hDevice, &header, sizeof(WAVEHDR));
	if (result != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to write to audio device");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	while (waveOutUnprepareHeader(hDevice, &header, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
		Sleep(10);

	waveOutClose(hDevice);
}