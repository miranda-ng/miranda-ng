#include "stdafx.h"

CToxCallDlgBase::CToxCallDlgBase(CToxProto *proto, int idDialog, MCONTACT hContact) :
	CToxDlgBase(proto, idDialog, false), hContact(hContact)
{
}

void CToxCallDlgBase::OnInitDialog()
{
	WindowList_Add(m_proto->hAudioDialogs, m_hwnd, hContact);
}

void CToxCallDlgBase::OnClose()
{
	WindowList_Remove(m_proto->hAudioDialogs, m_hwnd);
}

INT_PTR CToxCallDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CALL_END)
		if (wParam == hContact)
			Close();

	return CToxDlgBase::DlgProc(msg, wParam, lParam);
}

void CToxCallDlgBase::SetIcon(const char *name)
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, name);
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)IcoLib_GetIcon(iconName, false));
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIcon(iconName, true));
}

void CToxCallDlgBase::SetTitle(const TCHAR *title)
{
	SetWindowText(m_hwnd, title);
}

//////////////////////////////////////////////////////////////////////////////////////////////

CToxIncomingCall::CToxIncomingCall(CToxProto *proto, MCONTACT hContact) :
	CToxCallDlgBase(proto, IDD_CALL_RECEIVE, hContact),
	from(this, IDC_FROM), date(this, IDC_DATE),
	answer(this, IDOK), reject(this, IDCANCEL)
{
	answer.OnClick = Callback(this, &CToxIncomingCall::OnAnswer);
}

void CToxIncomingCall::OnInitDialog()
{
	CToxCallDlgBase::OnInitDialog();
	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "IncomingCallWindow_");

	TCHAR *nick = pcli->pfnGetContactDisplayName(hContact, 0);
	from.SetText(nick);

	TCHAR title[MAX_PATH];
	mir_sntprintf(title, TranslateT("Incoming call from %s"), nick);
	SetTitle(title);
	SetIcon("audio_ring");
}

void CToxIncomingCall::OnClose()
{
	toxav_reject(m_proto->toxThread->toxAv, m_proto->calls[hContact], NULL);
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "IncomingCallWindow_");
	CToxCallDlgBase::OnClose();
}

void CToxIncomingCall::OnAnswer(CCtrlBase*)
{
	ToxAvCSettings *cSettings = m_proto->GetAudioCSettings();
	if (cSettings == NULL)
		return;

	if (toxav_answer(m_proto->toxThread->toxAv, m_proto->calls[hContact], cSettings) == TOX_ERROR)
		m_proto->debugLogA(__FUNCTION__": failed to start call");
}

//////////////////////////////////////////////////////////////////////////////////////////////

CToxOutgoingCall::CToxOutgoingCall(CToxProto *proto, MCONTACT hContact) :
	CToxCallDlgBase(proto, IDD_CALL_SEND, hContact),
	to(this, IDC_FROM), call(this, IDOK), cancel(this, IDCANCEL)
{
	m_autoClose = CLOSE_ON_CANCEL;
	call.OnClick = Callback(this, &CToxOutgoingCall::OnCall);
	cancel.OnClick = Callback(this, &CToxOutgoingCall::OnCancel);
}

void CToxOutgoingCall::OnInitDialog()
{
	CToxCallDlgBase::OnInitDialog();
	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "OutgoingCallWindow_");

	TCHAR *nick = pcli->pfnGetContactDisplayName(hContact, 0);
	to.SetText(nick);

	TCHAR title[MAX_PATH];
	mir_sntprintf(title, TranslateT("Outgoing call to %s"), nick);
	SetTitle(title);
	SetIcon("audio_end");
}

void CToxOutgoingCall::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "OutgoingCallWindow_");
	CToxCallDlgBase::OnClose();
}

void CToxOutgoingCall::OnCall(CCtrlBase*)
{
	ToxAvCSettings *cSettings = m_proto->GetAudioCSettings();
	if (cSettings == NULL)
	{
		Close();
		return;
	}

	int friendNumber = m_proto->GetToxFriendNumber(hContact);
	if (friendNumber == UINT32_MAX)
	{
		mir_free(cSettings);
		Close();
		return;
	}

	int32_t callId;
	if (toxav_call(m_proto->toxThread->toxAv, &callId, friendNumber, cSettings, 10) == TOX_ERROR)
	{
		mir_free(cSettings);
		m_proto->debugLogA(__FUNCTION__": failed to start outgoing call");
		return;
	}
	mir_free(cSettings);
	m_proto->calls[hContact] = callId;

	char *message = NULL;
	TCHAR title[MAX_PATH];
	if (GetWindowText(m_hwnd, title, _countof(title)))
		message = mir_utf8encodeT(title);
	else
		message = mir_utf8encode("Outgoing call");
	m_proto->AddEventToDb(hContact, DB_EVENT_CALL, time(NULL), DBEF_UTF, (PBYTE)message, mir_strlen(message));

	call.Enable(FALSE);
	SetIcon("audio_call");
}

void CToxOutgoingCall::OnCancel(CCtrlBase*)
{
	if (!call.Enabled())
		toxav_cancel(m_proto->toxThread->toxAv, m_proto->calls[hContact], 0, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////

CToxCallDialog::CToxCallDialog(CToxProto *proto, MCONTACT hContact) :
	CToxCallDlgBase(proto, IDD_CALL, hContact), end(this, IDCANCEL)
{
}

void CToxCallDialog::OnInitDialog()
{
	CToxCallDlgBase::OnInitDialog();
	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CallWindow_");
	SetIcon("audio_start");
}

void CToxCallDialog::OnClose()
{
	toxav_hangup(m_proto->toxThread->toxAv, m_proto->calls[hContact]);
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CallWindow_");
	CToxCallDlgBase::OnClose();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ToxAvCSettings* CToxProto::GetAudioCSettings()
{
	ToxAvCSettings *cSettings = (ToxAvCSettings*)mir_calloc(sizeof(ToxAvCSettings));
	cSettings->audio_frame_duration = 20;

	DWORD deviceId = getDword("AudioInputDeviceID", WAVE_MAPPER);

	WAVEINCAPS wic;
	MMRESULT error = waveInGetDevCaps(deviceId, &wic, sizeof(WAVEINCAPS));
	if (error != MMSYSERR_NOERROR)
	{
		debugLogA(__FUNCTION__": failed to get input device caps (%d)", error);

		TCHAR errorMessage[MAX_PATH];
		waveInGetErrorText(error, errorMessage, _countof(errorMessage));
		CToxProto::ShowNotification(
			TranslateT("Unable to find input audio device"),
			errorMessage);

		mir_free(cSettings);
		return NULL;
	}

	cSettings->audio_channels = wic.wChannels;
	if ((wic.dwFormats & WAVE_FORMAT_48S16) || (wic.dwFormats & WAVE_FORMAT_48M16))
	{
		cSettings->audio_bitrate = 16 * 1000;
		cSettings->audio_sample_rate = 48000;
	}
	else if ((wic.dwFormats & WAVE_FORMAT_48S08) || (wic.dwFormats & WAVE_FORMAT_48M08))
	{
		cSettings->audio_bitrate = 8 * 1000;
		cSettings->audio_sample_rate = 48000;
	}
	else if ((wic.dwFormats & WAVE_FORMAT_4S16) || (wic.dwFormats & WAVE_FORMAT_4M16))
	{
		cSettings->audio_bitrate = 16 * 1000;
		cSettings->audio_sample_rate = 24000;
	}
	else if ((wic.dwFormats & WAVE_FORMAT_4S08) || (wic.dwFormats & WAVE_FORMAT_4S08))
	{
		cSettings->audio_bitrate = 8 * 1000;
		cSettings->audio_sample_rate = 24000;
	}
	else if ((wic.dwFormats & WAVE_FORMAT_2M16) || (wic.dwFormats & WAVE_FORMAT_2S16))
	{
		cSettings->audio_bitrate = 16 * 1000;
		cSettings->audio_sample_rate = 16000;
	}
	else if ((wic.dwFormats & WAVE_FORMAT_2M08) || (wic.dwFormats & WAVE_FORMAT_2S08))
	{
		cSettings->audio_bitrate = 8 * 1000;
		cSettings->audio_sample_rate = 16000;
	}
	else if ((wic.dwFormats & WAVE_FORMAT_1M16) || (wic.dwFormats & WAVE_FORMAT_1S16))
	{
		cSettings->audio_bitrate = 16 * 1000;
		cSettings->audio_sample_rate = 8000;
	}
	else if ((wic.dwFormats & WAVE_FORMAT_1M08) || (wic.dwFormats & WAVE_FORMAT_1S08))
	{
		cSettings->audio_bitrate = 8 * 1000;
		cSettings->audio_sample_rate = 8000;
	}
	else
	{
		debugLogA(__FUNCTION__": failed to parse input device caps");
		mir_free(cSettings);
		return NULL;
	}

	return cSettings;
}

/* INCOMING CALL */

// incoming call flow
void CToxProto::OnAvInvite(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxThread->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		toxav_reject(proto->toxThread->toxAv, callId, NULL);
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to find contact");
		toxav_reject(proto->toxThread->toxAv, callId, NULL);
		return;
	}

	ToxAvCSettings cSettings;
	if (toxav_get_peer_csettings(proto->toxThread->toxAv, callId, 0, &cSettings) != av_ErrorNone)
	{
		proto->debugLogA(__FUNCTION__": failed to get codec settings");
		toxav_reject(proto->toxThread->toxAv, callId, NULL);
		return;
	}

	if (cSettings.call_type != av_TypeAudio)
	{
		proto->debugLogA(__FUNCTION__": video call is unsupported");
		toxav_reject(proto->toxThread->toxAv, callId, Translate("Video call is unsupported"));
		return;
	}

	TCHAR message[MAX_PATH];
	mir_sntprintf(message, TranslateT("Incoming call from %s"), pcli->pfnGetContactDisplayName(hContact, 0));
	T2Utf szMessage(message);

	PROTORECVEVENT recv = { 0 };
	recv.timestamp = time(NULL);
	recv.lParam = callId;
	recv.szMessage = szMessage;
	ProtoChainRecv(hContact, PSR_AUDIO, hContact, (LPARAM)&recv);
}

// save event to db
INT_PTR CToxProto::OnRecvAudioCall(WPARAM hContact, LPARAM lParam)
{
	PROTORECVEVENT *pre = (PROTORECVEVENT*)lParam;

	calls[hContact] = pre->lParam;

	MEVENT hEvent = AddEventToDb(hContact, DB_EVENT_CALL, pre->timestamp, DBEF_UTF, (PBYTE)pre->szMessage, mir_strlen(pre->szMessage));

	CLISTEVENT cle = { sizeof(cle) };
	cle.flags |= CLEF_TCHAR;
	cle.hContact = hContact;
	cle.hDbEvent = hEvent;
	cle.lParam = DB_EVENT_CALL;
	cle.hIcon = IcoLib_GetIconByHandle(GetIconHandle("audio_ring"));

	TCHAR szTooltip[MAX_PATH];
	mir_sntprintf(szTooltip, TranslateT("Incoming call from %s"), pcli->pfnGetContactDisplayName(hContact, 0));
	cle.ptszTooltip = szTooltip;

	char szService[MAX_PATH];
	mir_snprintf(szService, "%s/Audio/Ring", GetContactProto(hContact));
	cle.pszService = szService;

	CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);

	return hEvent;
}

// react on clist event click
INT_PTR CToxProto::OnAudioRing(WPARAM, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT*)lParam;
	CDlgBase *incomingCallDlg = new CToxIncomingCall(this, cle->hContact);
	incomingCallDlg->Show();

	return 0;
}

void CToxProto::OnAvCancel(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxThread->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to find contact");
		return;
	}

	CLISTEVENT *cle = NULL;
	while ((cle = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, hContact, 0)))
	{
		if (cle->lParam == DB_EVENT_CALL)
		{
			CallService(MS_CLIST_REMOVEEVENT, hContact, cle->hDbEvent);
			break;
		}
	}

	char *message = mir_utf8encodeT(TranslateT("Call canceled"));
	proto->AddEventToDb(hContact, DB_EVENT_CALL, time(NULL), DBEF_UTF, (PBYTE)message, mir_strlen(message));

	WindowList_Broadcast(proto->hAudioDialogs, WM_CALL_END, hContact, 0);
}

/* OUTGOING CALL */

// outcoming audio flow
INT_PTR CToxProto::OnSendAudioCall(WPARAM hContact, LPARAM)
{
	CDlgBase *outgoingCallDlg = new CToxOutgoingCall(this, hContact);
	outgoingCallDlg->Show();

	return 0;
}

void CToxProto::OnAvReject(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxThread->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to find contact");
		return;
	}

	char *message = mir_utf8encodeT(TranslateT("Call canceled"));
	proto->AddEventToDb(hContact, DB_EVENT_CALL, time(NULL), DBEF_UTF, (PBYTE)message, mir_strlen(message));

	WindowList_Broadcast(proto->hAudioDialogs, WM_CALL_END, hContact, 0);
}

void CToxProto::OnAvCallTimeout(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	int friendNumber = toxav_get_peer_id(proto->toxThread->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to find contact");
		return;
	}

	char *message = mir_utf8encodeT(TranslateT("Call canceled"));
	proto->AddEventToDb(hContact, DB_EVENT_CALL, time(NULL), DBEF_UTF, (PBYTE)message, mir_strlen(message));

	WindowList_Broadcast(proto->hAudioDialogs, WM_CALL_END, hContact, 0);
}

/* --- */

static void CALLBACK WaveOutCallback(HWAVEOUT hOutDevice, UINT uMsg, DWORD/* dwInstance*/, DWORD dwParam1, DWORD/* dwParam2*/)
{
	if (uMsg == WOM_DONE)
	{
		WAVEHDR *header = (WAVEHDR*)dwParam1;
		if (header->dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(hOutDevice, header, sizeof(WAVEHDR));
		mir_free(header->lpData);
		mir_free(header);
	}
}

static void CALLBACK ToxShowDialogApcProc(void *arg)
{
	CDlgBase *callDlg = (CDlgBase*)arg;
	callDlg->Show();
}

void CToxProto::OnAvStart(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxAvCSettings cSettings;
	int cSettingsError = toxav_get_peer_csettings(proto->toxThread->toxAv, callId, 0, &cSettings);
	if (cSettingsError != av_ErrorNone)
	{
		proto->debugLogA(__FUNCTION__": failed to get codec settings (%d)", cSettingsError);
		toxav_hangup(proto->toxThread->toxAv, callId);
		return;
	}

	if (cSettings.call_type != av_TypeAudio)
	{
		proto->debugLogA(__FUNCTION__": video call is unsupported");
		toxav_hangup(proto->toxThread->toxAv, callId);
		return;
	}

	WAVEFORMATEX wfx = { 0 };
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = cSettings.audio_channels;
	wfx.wBitsPerSample = cSettings.audio_bitrate / 1000;
	wfx.nSamplesPerSec = cSettings.audio_sample_rate;
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	DWORD deviceId = proto->getDword("AudioOutputDeviceID", WAVE_MAPPER);
	MMRESULT error = waveOutOpen(&proto->hOutDevice, deviceId, &wfx, (DWORD_PTR)WaveOutCallback, (DWORD_PTR)proto, CALLBACK_FUNCTION);
	if (error != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to open audio device (%d)", error);
		toxav_hangup(proto->toxThread->toxAv, callId);

		TCHAR errorMessage[MAX_PATH];
		waveInGetErrorText(error, errorMessage, _countof(errorMessage));
		CToxProto::ShowNotification(
			TranslateT("Unable to find output audio device"),
			errorMessage);

		return;
	}

	int friendNumber = toxav_get_peer_id(proto->toxThread->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		toxav_hangup(proto->toxThread->toxAv, callId);
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to find contact");
		toxav_hangup(proto->toxThread->toxAv, callId);
		return;
	}

	if (toxav_prepare_transmission(proto->toxThread->toxAv, callId, false) == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to prepare audio transmition");
		toxav_hangup(proto->toxThread->toxAv, callId);
		return;
	}

	char *message = mir_utf8encodeT(TranslateT("Call started"));
	proto->AddEventToDb(hContact, DB_EVENT_CALL, time(NULL), DBEF_UTF, (PBYTE)message, mir_strlen(message));


	WindowList_Broadcast(proto->hAudioDialogs, WM_CALL_END, hContact, 0);
	CDlgBase *callDlg = new CToxCallDialog(proto, hContact);
	CallFunctionAsync(ToxShowDialogApcProc, callDlg);
}

void CToxProto::OnAvEnd(void*, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	waveOutReset(proto->hOutDevice);
	waveOutClose(proto->hOutDevice);
	toxav_kill_transmission(proto->toxThread->toxAv, callId);

	int friendNumber = toxav_get_peer_id(proto->toxThread->toxAv, callId, 0);
	if (friendNumber == TOX_ERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to get friend number");
		return;
	}

	MCONTACT hContact = proto->GetContact(friendNumber);
	if (hContact == NULL)
	{
		proto->debugLogA(__FUNCTION__": failed to find contact");
		return;
	}

	char *message = mir_utf8encodeT(TranslateT("Call ended"));
	proto->AddEventToDb(hContact, DB_EVENT_CALL, time(NULL), DBEF_UTF, (PBYTE)message, mir_strlen(message));

	WindowList_Broadcast(proto->hAudioDialogs, WM_CALL_END, hContact, 0);
}

void CToxProto::OnAvPeerTimeout(void *av, int32_t callId, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxAvCallState callState = toxav_get_call_state(proto->toxThread->toxAv, callId);
	switch (callState)
	{
	case av_CallStarting:
		proto->OnAvCancel(av, callId, arg);
		return;

	case av_CallActive:
		proto->OnAvEnd(av, callId, arg);
		return;

	default:
		proto->debugLogA(__FUNCTION__": failed to handle callState");
		break;
	}
}

//////

void CToxProto::OnFriendAudio(void*, int32_t, const int16_t *PCM, uint16_t size, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	WAVEHDR *header = (WAVEHDR*)mir_calloc(sizeof(WAVEHDR));
	header->dwBufferLength = size * sizeof(int16_t);
	header->lpData = (LPSTR)mir_alloc(header->dwBufferLength);
	memcpy(header->lpData, (PBYTE)PCM, header->dwBufferLength);

	MMRESULT error = waveOutPrepareHeader(proto->hOutDevice, header, sizeof(WAVEHDR));
	if (error != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to prepare audio buffer (%d)", error);
		return;
	}

	error = waveOutWrite(proto->hOutDevice, header, sizeof(WAVEHDR));
	if (error != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to play audio samples (%d)", error);
		return;
	}
}