#include "common.h"

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
	recv.flags = PREF_UTF;
	recv.szMessage = mir_utf8encodeT(TranslateT("Incoming call"));
	ProtoChainRecv(hContact, PSR_AUDIO, hContact, (LPARAM)&recv);
}

INT_PTR CToxProto::OnRecvAudioCall(WPARAM hContact, LPARAM lParam)
{
	PROTORECVEVENT *pre = (PROTORECVEVENT*)lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_szModuleName;
	dbei.timestamp = pre->timestamp;
	dbei.flags = DBEF_UTF;
	dbei.eventType = DB_EVENT_AUDIO_CALL;
	dbei.cbBlob = (DWORD)mir_strlen(pre->szMessage) + 1;
	dbei.pBlob = (PBYTE)pre->szMessage;

	return (INT_PTR)db_event_add(hContact, &dbei);
}

void CToxProto::OnAvRinging(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvStart(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvEnd(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvReject(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvCancel(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvCsChange(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvRequestTimeout(void*, int32_t callId, void *arg) { }
void CToxProto::OnAvPeerTimeout(void*, int32_t callId, void *arg) { }

void CToxProto::OnFriendAudio(void*, int32_t callId, const int16_t *PCM, uint16_t size, void *arg)
{
	CToxProto *proto = (CToxProto*)arg;

	ToxAvCSettings dest;
	if (toxav_get_peer_csettings(proto->toxAv, callId, 0, &dest) != av_ErrorNone)
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
	wfx.wBitsPerSample = dest.audio_bitrate;
	wfx.nSamplesPerSec = dest.audio_sample_rate;
	wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	DWORD deviceId = proto->getDword("AudioOutputDeviceID", -1);
	if (deviceId != av_ErrorNone)
	{
		proto->debugLogA(__FUNCTION__": failed to get device id");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	HWAVEOUT hDevice;
	MMRESULT result = waveOutOpen(&hDevice, deviceId, &wfx, 0, 0, CALLBACK_NULL | WAVE_FORMAT_DIRECT);
	if (result != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to open audio device");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	WAVEHDR header;// = { 0 };
	ZeroMemory(&header, sizeof(WAVEHDR));
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

	do
	{
		Sleep(100);
		result = waveOutUnprepareHeader(hDevice, &header, sizeof(WAVEHDR));
	} while (result == WAVERR_STILLPLAYING);

	if (result != MMSYSERR_NOERROR)
	{
		proto->debugLogA(__FUNCTION__": failed to unprepare audio device header");
		toxav_stop_call(proto->toxAv, callId);
		return;
	}

	waveOutClose(hDevice);
}