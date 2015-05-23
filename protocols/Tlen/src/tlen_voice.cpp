/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "tlen.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <m_button.h>
//#include <win2k.h>
#include "resource.h"
#include "tlen_list.h"
#include "tlen_voice.h"
#include "tlen_p2p_old.h"
#include "tlen_file.h"

static int MODE_FREQUENCY[] = { 0, 0, 8000, 11025, 22050, 44100 };
static int MODE_FRAME_SIZE[] = { 0, 0, 5, 5, 10, 25 };
static int FRAMES_AVAILABLE_MAX_LIMIT = 2;
static int VU_METER_HEIGHT = 64;
static int VU_METER_WIDTH = 20;
static int VU_METER_LEVELS = 16;
static HBITMAP vuMeterBitmaps[100];

static void TlenVoiceReceiveParse(TLEN_FILE_TRANSFER *ft);
static void TlenVoiceSendParse(TLEN_FILE_TRANSFER *ft);
static void TlenVoiceReceivingConnection(HANDLE hNewConnection, DWORD dwRemoteIP, void * pExtra);

static void CALLBACK TlenVoicePlaybackCallback(HWAVEOUT hwo, UINT uMsg, DWORD* dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg == WOM_DONE) {
		TLEN_VOICE_CONTROL *control = (TLEN_VOICE_CONTROL *)dwInstance;
		waveOutUnprepareHeader(hwo, (WAVEHDR *)dwParam1, sizeof(WAVEHDR));
		if (control->proto->framesAvailableForPlayback > 0) {
			control->proto->framesAvailableForPlayback--;
		}
		//		playbackControl->waveHeaders[playbackControl->waveHeadersPos].dwFlags =  WHDR_DONE;
		//		playbackControl->waveHeaders[playbackControl->waveHeadersPos].lpData = (char *) (playbackControl->waveData + playbackControl->waveHeadersPos * playbackControl->waveFrameSize);
		//		playbackControl->waveHeaders[playbackControl->waveHeadersPos].dwBufferLength = playbackControl->waveFrameSize * 2;
		//		waveOutPrepareHeader(playbackControl->hWaveOut, &playbackControl->waveHeaders[playbackControl->waveHeadersPos], sizeof(WAVEHDR));
		//		waveOutWrite(playbackControl->hWaveOut, &playbackControl->waveHeaders[playbackControl->waveHeadersPos], sizeof(WAVEHDR));

	}
}

static void __cdecl TlenVoiceRecordingThreadProc(void *param)
{
	TLEN_VOICE_CONTROL *control = (TLEN_VOICE_CONTROL *)param;
	MSG		msg;
	HWAVEIN hWaveIn;
	WAVEHDR *hWaveHdr;
	control->isRunning = 1;
	control->stopThread = 0;
	while (TRUE) {
		GetMessage(&msg, NULL, 0, 0);
		if (msg.message == MM_WIM_DATA) {
			//			TlenLog("recording thread running...%d", msg.message);
			hWaveIn = (HWAVEIN)msg.wParam;
			hWaveHdr = (WAVEHDR *)msg.lParam;
			waveInUnprepareHeader(hWaveIn, hWaveHdr, sizeof(WAVEHDR));
			if (hWaveHdr->dwBytesRecorded > 0 && !control->bDisable) {
				control->recordingData = (short *)hWaveHdr->lpData;
				TlenVoiceSendParse(control->ft);
			}
			if (!control->stopThread) {
				waveInPrepareHeader(hWaveIn, &control->waveHeaders[control->waveHeadersPos], sizeof(WAVEHDR));
				waveInAddBuffer(hWaveIn, &control->waveHeaders[control->waveHeadersPos], sizeof(WAVEHDR));
				control->waveHeadersPos = (control->waveHeadersPos + 1) % control->waveHeadersNum;
			}
		}
		else if (msg.message == MM_WIM_CLOSE) {
			break;
		}
	}
	control->isRunning = 0;
	control->proto->debugLogA("recording thread ended...");
}

static int TlenVoicePlaybackStart(TLEN_VOICE_CONTROL *control)
{
	int i;
	int iNumDevs, iSelDev;
	WAVEOUTCAPS     wic;

	WAVEFORMATEX wfm;
	memset(&wfm, 0, sizeof(wfm));
	wfm.cbSize = sizeof(WAVEFORMATEX);
	wfm.nChannels = 1;
	wfm.wBitsPerSample = 16;
	wfm.nSamplesPerSec = MODE_FREQUENCY[control->codec];
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nChannels * wfm.wBitsPerSample / 8;
	wfm.nBlockAlign = 2 * wfm.nChannels;
	wfm.wFormatTag = WAVE_FORMAT_PCM;

	control->waveMode = 0;
	control->waveFrameSize = MODE_FRAME_SIZE[control->codec] * 160 * wfm.nChannels;// * wfm.wBitsPerSample / 8;
	control->waveHeadersPos = 0;
	control->waveHeadersNum = FRAMES_AVAILABLE_MAX_LIMIT + 2;

	int j = db_get_w(NULL, control->proto->m_szModuleName, "VoiceDeviceOut", 0);
	iSelDev = WAVE_MAPPER;
	if (j != 0) {
		iNumDevs = waveOutGetNumDevs();
		for (i = 0; i < iNumDevs; i++) {
			if (!waveOutGetDevCaps(i, &wic, sizeof(WAVEOUTCAPS))) {
				if (wic.dwFormats != 0) {
					j--;
					if (j == 0) {
						iSelDev = i;
						break;
					}
				}
			}
		}
	}
	if (!waveOutGetDevCaps(iSelDev, &wic, sizeof(WAVEOUTCAPS))) {
		control->proto->debugLogA("Playback device ID #%u: %s\r\n", iSelDev, wic.szPname);
	}

	MMRESULT mmres = waveOutOpen(&control->hWaveOut, iSelDev, &wfm, (DWORD_PTR)&TlenVoicePlaybackCallback, (DWORD)control, CALLBACK_FUNCTION);
	if (mmres != MMSYSERR_NOERROR) {
		control->proto->debugLogA("TlenVoiceStart FAILED!");
		return 1;
	}
	control->waveData = (short *)mir_alloc(control->waveHeadersNum * control->waveFrameSize * 2);
	memset(control->waveData, 0, control->waveHeadersNum * control->waveFrameSize * 2);
	control->waveHeaders = (WAVEHDR *)mir_alloc(control->waveHeadersNum * sizeof(WAVEHDR));
	control->proto->debugLogA("TlenVoiceStart OK!");
	return 0;
}


static int TlenVoiceRecordingStart(TLEN_VOICE_CONTROL *control)
{
	WAVEFORMATEX wfm;
	int i, j;
	int iNumDevs, iSelDev;
	WAVEINCAPS wic;

	memset(&wfm, 0, sizeof(wfm));
	wfm.cbSize = sizeof(WAVEFORMATEX);
	wfm.nChannels = 1;
	wfm.wBitsPerSample = 16;
	wfm.nSamplesPerSec = MODE_FREQUENCY[control->codec];
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nChannels * wfm.wBitsPerSample / 8;
	wfm.nBlockAlign = 2 * wfm.nChannels;
	wfm.wFormatTag = WAVE_FORMAT_PCM;


	control->waveMode = 0;
	//	control->isRunning	= 1;
	control->waveFrameSize = MODE_FRAME_SIZE[control->codec] * 160 * wfm.nChannels;// * wfm.wBitsPerSample / 8;
	control->waveHeadersPos = 0;
	control->waveHeadersNum = 2;

	control->hThread = mir_forkthread(TlenVoiceRecordingThreadProc, (void*)control);


	SetThreadPriority(control->hThread, THREAD_PRIORITY_ABOVE_NORMAL);

	j = db_get_w(NULL, control->proto->m_szModuleName, "VoiceDeviceIn", 0);
	iSelDev = WAVE_MAPPER;
	if (j != 0) {
		iNumDevs = waveInGetNumDevs();
		for (i = 0; i < iNumDevs; i++) {
			if (!waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS))) {
				if (wic.dwFormats != 0) {
					j--;
					if (j == 0) {
						iSelDev = i;
						break;
					}
				}
			}
		}
	}
	if (!waveInGetDevCaps(iSelDev, &wic, sizeof(WAVEINCAPS))) {
		control->proto->debugLogA("Recording device ID #%u: %s\r\n", iSelDev, wic.szPname);
	}

	MMRESULT mmres = waveInOpen(&control->hWaveIn, iSelDev, &wfm, control->threadID, 0, CALLBACK_THREAD);
	//	mmres = waveInOpen(&control->hWaveIn, 3, &wfm, (DWORD) &TlenVoiceRecordingCallback, (DWORD) control, CALLBACK_FUNCTION);
	if (mmres != MMSYSERR_NOERROR) {
		PostThreadMessage(control->threadID, WIM_CLOSE, 0, 0);
		control->proto->debugLogA("TlenVoiceStart FAILED %d!", mmres);
		return 1;
	}
	control->waveData = (short *)mir_alloc(control->waveHeadersNum * control->waveFrameSize * 2);
	memset(control->waveData, 0, control->waveHeadersNum * control->waveFrameSize * 2);
	control->waveHeaders = (WAVEHDR *)mir_alloc(control->waveHeadersNum * sizeof(WAVEHDR));
	for (i = 0; i < control->waveHeadersNum; i++) {
		control->waveHeaders[i].dwFlags = 0;//WHDR_DONE;
		control->waveHeaders[i].lpData = (char *)(control->waveData + i * control->waveFrameSize);
		control->waveHeaders[i].dwBufferLength = control->waveFrameSize * 2;
		mmres = waveInPrepareHeader(control->hWaveIn, &control->waveHeaders[i], sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR) {
			waveInClose(control->hWaveIn);
			//			PostThreadMessage(control->threadID, WIM_CLOSE, 0, 0);
			control->proto->debugLogA("TlenVoiceStart FAILED #2!");
			return 1;
		}
	}
	for (i = 0; i < control->waveHeadersNum; i++) {
		waveInAddBuffer(control->hWaveIn, &control->waveHeaders[i], sizeof(WAVEHDR));
	}
	waveInStart(control->hWaveIn);
	control->proto->debugLogA("TlenVoiceRStart OK!");
	return 0;
}


static TLEN_VOICE_CONTROL *TlenVoiceCreateVC(TlenProtocol *proto, int codec)
{
	TLEN_VOICE_CONTROL *vc = (TLEN_VOICE_CONTROL *)mir_alloc(sizeof(TLEN_VOICE_CONTROL));
	memset(vc, 0, sizeof(TLEN_VOICE_CONTROL));
	vc->gsmstate = gsm_create();
	vc->codec = codec;
	vc->proto = proto;
	return vc;
}
static void TlenVoiceFreeVc(TLEN_VOICE_CONTROL *vc)
{
	vc->proto->debugLogA("-> TlenVoiceFreeVc");
	vc->stopThread = 1;
	PostThreadMessage(vc->threadID, MM_WIM_CLOSE, 0, 0);
	while (vc->isRunning) {
		Sleep(50);
	}
	if (vc->hThread != NULL) CloseHandle(vc->hThread);
	if (vc->hWaveIn) {
		for (int i = 0; i < vc->waveHeadersNum; i++) {
			while (waveInUnprepareHeader(vc->hWaveIn, &vc->waveHeaders[i], sizeof(WAVEHDR)) == WAVERR_STILLPLAYING) {
				Sleep(50);
			}
		}
		while (waveInClose(vc->hWaveIn) == WAVERR_STILLPLAYING) {
			Sleep(50);
		}
	}
	if (vc->hWaveOut) {
		for (int i = 0; i < vc->waveHeadersNum; i++) {
			while (waveOutUnprepareHeader(vc->hWaveOut, &vc->waveHeaders[i], sizeof(WAVEHDR)) == WAVERR_STILLPLAYING) {
				Sleep(50);
			}
		}
		while (waveOutClose(vc->hWaveOut) == WAVERR_STILLPLAYING) {
			Sleep(50);
		}
	}
	if (vc->waveData) mir_free(vc->waveData);
	if (vc->waveHeaders) mir_free(vc->waveHeaders);
	if (vc->gsmstate) gsm_release(vc->gsmstate);
	vc->proto->debugLogA("<- TlenVoiceFreeVc");
	mir_free(vc);
}

static void TlenVoiceCrypt(char *buffer, int len)
{
	int i, j, k;
	j = 0x71;
	for (i = 0; i < len; i++) {
		k = j;
		j = j << 6;
		j += k;
		j = k + (j << 1);
		j += 0xBB;
		buffer[i] ^= j;
	}
}

void __cdecl TlenVoiceReceiveThread(TLEN_FILE_TRANSFER *ft)
{
	ft->proto->debugLogA("Thread started: type=file_receive server='%s' port='%d'", ft->hostName, ft->wPort);

	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.szHost = ft->hostName;
	nloc.wPort = ft->wPort;
	SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Connecting..."));
	HANDLE s = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)ft->proto->m_hNetlibUser, (LPARAM)&nloc);
	if (s != NULL) {
		ft->s = s;
		ft->proto->debugLogA("Entering file receive loop");
		TlenP2PEstablishOutgoingConnection(ft, FALSE);
		if (ft->state != FT_ERROR) {
			ft->proto->playbackControl = NULL;
			ft->proto->recordingControl = TlenVoiceCreateVC(ft->proto, 3);
			ft->proto->recordingControl->ft = ft;
			TlenVoiceRecordingStart(ft->proto->recordingControl);
			while (ft->state != FT_DONE && ft->state != FT_ERROR) {
				TlenVoiceReceiveParse(ft);
			}
			TlenVoiceFreeVc(ft->proto->recordingControl);
			ft->proto->playbackControl = NULL;
			ft->proto->recordingControl = NULL;
		}
		if (ft->s) {
			Netlib_CloseHandle(s);
		}
		ft->s = NULL;
	}
	else {
		ft->proto->debugLogA("Connection failed - receiving as server");
		ft->pfnNewConnectionV2 = TlenVoiceReceivingConnection;
		s = TlenP2PListen(ft);
		if (s != NULL) {
			SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Waiting for connection..."));
			ft->s = s;
			HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			ft->hFileEvent = hEvent;
			ft->currentFile = 0;
			ft->state = FT_CONNECTING;
			char *nick = TlenNickFromJID(ft->jid);
			TlenSend(ft->proto, "<v t='%s' i='%s' e='7' a='%s' p='%d'/>", nick, ft->iqId, ft->localName, ft->wLocalPort);
			mir_free(nick);
			ft->proto->debugLogA("Waiting for the file to be received...");
			WaitForSingleObject(hEvent, INFINITE);
			ft->hFileEvent = NULL;
			CloseHandle(hEvent);
			ft->proto->debugLogA("Finish all files");
			Netlib_CloseHandle(s);
		}
		else {
			ft->state = FT_ERROR;
		}
	}
	TlenListRemove(ft->proto, LIST_VOICE, ft->iqId);
	if (ft->state == FT_DONE) {
		SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Finished..."));
		//ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	}
	else {
		char *nick;
		nick = TlenNickFromJID(ft->jid);
		TlenSend(ft->proto, "<f t='%s' i='%s' e='8'/>", nick, ft->iqId);
		mir_free(nick);
		SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Error..."));
		//ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
	}
	ft->proto->debugLogA("Thread ended: type=file_receive server='%s'", ft->hostName);

	TlenP2PFreeFileTransfer(ft);
}

static void TlenVoiceReceivingConnection(HANDLE hConnection, DWORD dwRemoteIP, void * pExtra)
{
	HANDLE slisten;
	TlenProtocol *proto = (TlenProtocol *)pExtra;

	TLEN_FILE_TRANSFER *ft = TlenP2PEstablishIncomingConnection(proto, hConnection, LIST_VOICE, FALSE);
	if (ft != NULL) {
		slisten = ft->s;
		ft->s = hConnection;
		ft->proto->debugLogA("Set ft->s to %d (saving %d)", hConnection, slisten);
		ft->proto->debugLogA("Entering send loop for this file connection... (ft->s is hConnection)");
		proto->playbackControl = NULL;
		proto->recordingControl = TlenVoiceCreateVC(proto, 3);
		proto->recordingControl->ft = ft;
		TlenVoiceRecordingStart(proto->recordingControl);
		while (ft->state != FT_DONE && ft->state != FT_ERROR) {
			TlenVoiceReceiveParse(ft);
		}
		TlenVoiceFreeVc(proto->recordingControl);
		proto->playbackControl = NULL;
		proto->recordingControl = NULL;
		if (ft->state == FT_DONE) {
			SetDlgItemText(proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Finished..."));
			//			ProtoBroadcastAck(proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
		}
		else {
			//			ProtoBroadcastAck(proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Error..."));
		}
		ft->proto->debugLogA("Closing connection for this file transfer... (ft->s is now hBind)");
		ft->s = slisten;
		ft->proto->debugLogA("ft->s is restored to %d", ft->s);
	}
	Netlib_CloseHandle(hConnection);
	if (ft != NULL && ft->hFileEvent != NULL)
		SetEvent(ft->hFileEvent);
}

static void TlenVoiceReceiveParse(TLEN_FILE_TRANSFER *ft)
{
	char *statusTxt;
	int i, j;
	char *p;
	float val;
	TLEN_FILE_PACKET *packet = TlenP2PPacketReceive(ft->s);
	if (packet != NULL) {
		statusTxt = " Unknown packet ";
		p = packet->packet;
		if (packet->type == TLEN_VOICE_PACKET) {
			short *out;
			int codec, chunkNum;
			statusTxt = " OK ";
			TlenVoiceCrypt(packet->packet + 4, packet->len - 4);
			codec = *((int *)packet->packet);
			if (codec < 2 || codec>5) {
				statusTxt = " Unknown codec ";
			}
			else {
				if (ft->proto->playbackControl == NULL) {
					ft->proto->playbackControl = TlenVoiceCreateVC(ft->proto, codec);
					TlenVoicePlaybackStart(ft->proto->playbackControl);
					ft->proto->framesAvailableForPlayback = 0;
					ft->proto->availOverrunValue = 0;
				}
				else if (ft->proto->playbackControl->codec != codec) {
					TlenVoiceFreeVc(ft->proto->playbackControl);
					ft->proto->playbackControl = TlenVoiceCreateVC(ft->proto, codec);
					TlenVoicePlaybackStart(ft->proto->playbackControl);
					ft->proto->framesAvailableForPlayback = 0;
					ft->proto->availOverrunValue = 0;
				}
				if (!ft->proto->playbackControl->bDisable) {
					ft->proto->playbackControl->waveHeaders[ft->proto->playbackControl->waveHeadersPos].dwFlags = WHDR_DONE;
					ft->proto->playbackControl->waveHeaders[ft->proto->playbackControl->waveHeadersPos].lpData = (char *)(ft->proto->playbackControl->waveData + ft->proto->playbackControl->waveHeadersPos * ft->proto->playbackControl->waveFrameSize);
					ft->proto->playbackControl->waveHeaders[ft->proto->playbackControl->waveHeadersPos].dwBufferLength = ft->proto->playbackControl->waveFrameSize * 2;
					/*
				if (availPlayback == 0) {
				statusTxt = "!! Buffer is empty !!";
				availPlayback++;
				waveOutPrepareHeader(playbackControl->hWaveOut, &playbackControl->waveHeaders[playbackControl->waveHeadersPos], sizeof(WAVEHDR));
				waveOutWrite(playbackControl->hWaveOut, &playbackControl->waveHeaders[playbackControl->waveHeadersPos], sizeof(WAVEHDR));
				playbackControl->waveHeadersPos = (playbackControl->waveHeadersPos +1) % playbackControl->waveHeadersNum;
				playbackControl->waveHeaders[playbackControl->waveHeadersPos].dwFlags =  WHDR_DONE;
				playbackControl->waveHeaders[playbackControl->waveHeadersPos].lpData = (char *) (playbackControl->waveData + playbackControl->waveHeadersPos * playbackControl->waveFrameSize);
				playbackControl->waveHeaders[playbackControl->waveHeadersPos].dwBufferLength = playbackControl->waveFrameSize * 2;
				}
				*/
					chunkNum = min(MODE_FRAME_SIZE[codec], (int)(packet->len - 4) / 33);
					out = (short *)ft->proto->playbackControl->waveHeaders[ft->proto->playbackControl->waveHeadersPos].lpData;
					for (i = 0; i < chunkNum; i++) {
						for (j = 0; j < 33; j++) {
							ft->proto->playbackControl->gsmstate->gsmFrame[j] = packet->packet[i * 33 + j + 4];
						}
						gsm_decode(ft->proto->playbackControl->gsmstate, out);
						out += 160;
					}
					out = (short *)ft->proto->playbackControl->waveHeaders[ft->proto->playbackControl->waveHeadersPos].lpData;
					val = 0;
					for (i = 0; i<MODE_FRAME_SIZE[codec] * 160; i += MODE_FRAME_SIZE[codec]) {
						val += out[i] * out[i];
					}
					j = (int)((log10(val) - 4) * 2.35);
					if (j > VU_METER_LEVELS - 1) {
						j = VU_METER_LEVELS - 1;
					}
					else if (j < 0) {
						j = 0;
					}
					ft->proto->playbackControl->vuMeter = j;
					ft->proto->playbackControl->bytesSum += 8 + packet->len;
					/* Simple logic to avoid huge delays. If a delay is detected a frame is not played */
					j = ft->proto->availOverrunValue > 0 ? -1 : 0;
					while (ft->proto->framesAvailableForPlayback > FRAMES_AVAILABLE_MAX_LIMIT) {
						j = 1;
						SleepEx(5, FALSE);
					}
					ft->proto->availOverrunValue += j;
					/* 40 frames - 800ms/8kHz */
					if (ft->proto->availOverrunValue < 40) {
						ft->proto->framesAvailableForPlayback++;
						waveOutPrepareHeader(ft->proto->playbackControl->hWaveOut, &ft->proto->playbackControl->waveHeaders[ft->proto->playbackControl->waveHeadersPos], sizeof(WAVEHDR));
						waveOutWrite(ft->proto->playbackControl->hWaveOut, &ft->proto->playbackControl->waveHeaders[ft->proto->playbackControl->waveHeadersPos], sizeof(WAVEHDR));
						ft->proto->playbackControl->waveHeadersPos = (ft->proto->playbackControl->waveHeadersPos + 1) % ft->proto->playbackControl->waveHeadersNum;
					}
					else {
						ft->proto->availOverrunValue -= 10;
						statusTxt = "!! Skipping frame !!";
					}
				}
			}
		}
		{
			char ttt[2048];
			mir_snprintf(ttt, SIZEOF(ttt), "%s %d %d ", statusTxt, ft->proto->framesAvailableForPlayback, ft->proto->availOverrunValue);
			SetDlgItemTextA(ft->proto->voiceDlgHWND, IDC_STATUS, ttt);
		}
		TlenP2PPacketFree(packet);
	}
	else {
		if (ft->proto->playbackControl != NULL) {
			TlenVoiceFreeVc(ft->proto->playbackControl);
			ft->proto->playbackControl = NULL;
		}
		ft->state = FT_ERROR;
	}
}


void __cdecl TlenVoiceSendingThread(TLEN_FILE_TRANSFER *ft)
{
	char *nick;

	ft->proto->debugLogA("Thread started: type=voice_send");
	ft->pfnNewConnectionV2 = TlenVoiceReceivingConnection;
	HANDLE s = TlenP2PListen(ft);
	if (s != NULL) {
		SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Waiting for connection..."));
		//ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, ft, 0);
		ft->s = s;
		//TlenLog("ft->s = %d", s);
		//TlenLog("fileCount = %d", ft->fileCount);

		HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		ft->hFileEvent = hEvent;
		ft->currentFile = 0;
		ft->state = FT_CONNECTING;

		nick = TlenNickFromJID(ft->jid);
		TlenSend(ft->proto, "<v t='%s' i='%s' e='6' a='%s' p='%d'/>", nick, ft->iqId, ft->localName, ft->wLocalPort);
		mir_free(nick);
		ft->proto->debugLogA("Waiting for the voice data to be sent...");
		WaitForSingleObject(hEvent, INFINITE);
		ft->hFileEvent = NULL;
		CloseHandle(hEvent);
		ft->proto->debugLogA("Finish voice");
		Netlib_CloseHandle(s);
		ft->s = NULL;
		ft->proto->debugLogA("ft->s is NULL");

		if (ft->state == FT_SWITCH) {
			ft->proto->debugLogA("Sending as client...");
			ft->state = FT_CONNECTING;

			NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
			nloc.szHost = ft->hostName;
			nloc.wPort = ft->wPort;
			HANDLE s = (HANDLE)CallService(MS_NETLIB_OPENCONNECTION, (WPARAM)ft->proto->m_hNetlibUser, (LPARAM)&nloc);
			if (s != NULL) {
				SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Connecting..."));
				//ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, ft, 0);
				ft->s = s;
				TlenP2PEstablishOutgoingConnection(ft, FALSE);
				if (ft->state != FT_ERROR) {
					ft->proto->debugLogA("Entering send loop for this file connection...");
					ft->proto->playbackControl = NULL;
					ft->proto->recordingControl = TlenVoiceCreateVC(ft->proto, 3);
					ft->proto->recordingControl->ft = ft;
					TlenVoiceRecordingStart(ft->proto->recordingControl);
					while (ft->state != FT_DONE && ft->state != FT_ERROR) {
						TlenVoiceReceiveParse(ft);
					}
				}
				ft->proto->debugLogA("Closing connection for this file transfer... ");
				Netlib_CloseHandle(s);
			}
			else ft->state = FT_ERROR;
		}
	}
	else {
		ft->proto->debugLogA("Cannot allocate port to bind for file server thread, thread ended.");
		ft->state = FT_ERROR;
	}
	TlenListRemove(ft->proto, LIST_VOICE, ft->iqId);
	switch (ft->state) {
	case FT_DONE:
		ft->proto->debugLogA("Finish successfully");
		SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Finished..."));
		//ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
		break;
	case FT_DENIED:
		SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Denied..."));
		//ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, ft, 0);
		break;
	default: // FT_ERROR:
		nick = TlenNickFromJID(ft->jid);
		TlenSend(ft->proto, "<v t='%s' i='%s' e='8'/>", nick, ft->iqId);
		mir_free(nick);
		ft->proto->debugLogA("Finish with errors");
		SetDlgItemText(ft->proto->voiceDlgHWND, IDC_STATUS, TranslateT("...Error..."));
		//ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		break;
	}
	ft->proto->debugLogA("Thread ended: type=voice_send");
	TlenP2PFreeFileTransfer(ft);
}

static void TlenVoiceSendParse(TLEN_FILE_TRANSFER *ft)
{
	int i;

	int codec = ft->proto->recordingControl->codec;
	TLEN_FILE_PACKET *packet = TlenP2PPacketCreate(sizeof(DWORD) + MODE_FRAME_SIZE[codec] * 33);
	if (packet != NULL) {
		short *in;
		float val;
		in = ft->proto->recordingControl->recordingData;
		TlenP2PPacketSetType(packet, 0x96);
		packet->packet[0] = codec;
		TlenP2PPacketPackDword(packet, codec);
		val = 0;
		for (i = 0; i<MODE_FRAME_SIZE[codec] * 160; i += MODE_FRAME_SIZE[codec]) {
			val += in[i] * in[i];
		}
		i = (int)((log10(val) - 4) * 2.35);
		if (i > VU_METER_LEVELS - 1) {
			i = VU_METER_LEVELS - 1;
		}
		else if (i < 0) {
			i = 0;
		}
		ft->proto->recordingControl->vuMeter = i;
		for (i = 0; i < MODE_FRAME_SIZE[codec]; i++) {
			gsm_encode(ft->proto->recordingControl->gsmstate, in + i * 160);
			TlenP2PPacketPackBuffer(packet, (char*)ft->proto->recordingControl->gsmstate->gsmFrame, 33);
		}
		TlenVoiceCrypt(packet->packet + 4, packet->len - 4);
		if (!TlenP2PPacketSend(ft->s, packet)) {
			ft->state = FT_ERROR;
		}
		ft->proto->recordingControl->bytesSum += 8 + packet->len;
		TlenP2PPacketFree(packet);
	}
	else {
		ft->state = FT_ERROR;
	}
}

int TlenVoiceCancelAll(TlenProtocol *proto)
{
	HANDLE hEvent;
	int i = 0;

	while ((i = TlenListFindNext(proto, LIST_VOICE, 0)) >= 0) {
		TLEN_LIST_ITEM *item = TlenListGetItemPtrFromIndex(proto, i);
		if (item != NULL) {
			TLEN_FILE_TRANSFER *ft = item->ft;
			TlenListRemoveByIndex(proto, i);
			if (ft != NULL) {
				if (ft->s) {
					//ProtoBroadcastAck(proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
					proto->debugLogA("Closing ft->s = %d", ft->s);
					ft->state = FT_ERROR;
					Netlib_CloseHandle(ft->s);
					ft->s = NULL;
					if (ft->hFileEvent != NULL) {
						hEvent = ft->hFileEvent;
						ft->hFileEvent = NULL;
						SetEvent(hEvent);
					}
				}
				else {
					proto->debugLogA("freeing (V) ft struct");
					TlenP2PFreeFileTransfer(ft);
				}
			}
		}
	}
	if (proto->voiceDlgHWND != NULL) {
		EndDialog(proto->voiceDlgHWND, 0);
	}
	return 0;
}

INT_PTR TlenProtocol::VoiceContactMenuHandleVoice(WPARAM wParam, LPARAM lParam)
{
	if (!isOnline)
		return 1;

	MCONTACT hContact = (MCONTACT)wParam;
	if (hContact != NULL) {
		DBVARIANT dbv;
		if (!db_get(hContact, m_szModuleName, "jid", &dbv)) {
			char serialId[32];
			mir_snprintf(serialId, SIZEOF(serialId), "%d", TlenSerialNext(this));
			TLEN_LIST_ITEM *item = TlenListAdd(this, LIST_VOICE, serialId);
			if (item != NULL) {
				TLEN_FILE_TRANSFER *ft = TlenFileCreateFT(this, dbv.pszVal);
				ft->iqId = mir_strdup(serialId);
				item->ft = ft;
				TlenVoiceStart(ft, 2);
				TlenSend(ft->proto, "<v t='%s' e='1' i='%s' v='1'/>", ft->jid, serialId);
			}
			db_free(&dbv);
		}
	}
	return 0;
}

int TlenVoiceIsInUse(TlenProtocol *proto) {
	if (TlenListFindNext(proto, LIST_VOICE, 0) >= 0 || proto->voiceDlgHWND != NULL) {
		proto->debugLogA("voice in use ? %d", proto->voiceDlgHWND);
		return 1;
	}
	return 0;
}

static HBITMAP TlenVoiceMakeBitmap(int w, int h, int bpp, void *ptr)
{
	BITMAPINFO bmih;
	bmih.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmih.bmiHeader.biWidth = w & 0xFFFFFFFC;
	bmih.bmiHeader.biHeight = h;//&0xFFFFFFFC;
	bmih.bmiHeader.biPlanes = 1;			   // musi byc 1
	bmih.bmiHeader.biBitCount = bpp;
	bmih.bmiHeader.biCompression = BI_RGB;
	bmih.bmiHeader.biSizeImage = 0;
	bmih.bmiHeader.biXPelsPerMeter = 0;
	bmih.bmiHeader.biYPelsPerMeter = 0;
	bmih.bmiHeader.biClrUsed = 0;
	bmih.bmiHeader.biClrImportant = 0;
	HDC hdc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	HBITMAP hbm = CreateDIBitmap(hdc, (PBITMAPINFOHEADER)&bmih, CBM_INIT, ptr, &bmih, DIB_RGB_COLORS);
	ReleaseDC(NULL, hdc);
	return hbm;
}

static void TlenVoiceInitVUMeters()
{
	int i, v, y, x, x0, col, col0;
	unsigned char *pBits;
	int ledWidth, ledHeight;
	ledWidth = 9;
	ledHeight = 6;
	VU_METER_HEIGHT = ledHeight;
	VU_METER_WIDTH = (VU_METER_LEVELS - 1) * ledWidth;
	VU_METER_WIDTH = (VU_METER_WIDTH + 3) & (~3);
	pBits = (unsigned char *)mir_alloc(3 * VU_METER_WIDTH*VU_METER_HEIGHT);
	memset(pBits, 0x80, 3 * VU_METER_WIDTH*VU_METER_HEIGHT);
	for (i = 0; i < VU_METER_LEVELS; i++) {
		for (v = 0; v < VU_METER_LEVELS - 1; v++) {
			if (v >= i) {
				if (v < 10) col0 = 0x104010;
				else if (v < 13) col0 = 0x404010;
				else 	col0 = 0x401010;
			}
			else {
				if (v < 10) col0 = 0x00f000;
				else if (v < 13) col0 = 0xf0f000;
				else col0 = 0xf00000;
			}
			x0 = v * ledWidth;
			for (y = 1; y < VU_METER_HEIGHT - 1; y++) {
				col = col0;
				for (x = 1; x < ledWidth; x++) {
					pBits[3 * (x + x0 + y*VU_METER_WIDTH)] = col & 0xFF;
					pBits[3 * (x + x0 + y*VU_METER_WIDTH) + 1] = (col >> 8) & 0xFF;
					pBits[3 * (x + x0 + y*VU_METER_WIDTH) + 2] = (col >> 16) & 0xFF;
				}
			}
		}
		vuMeterBitmaps[i] = TlenVoiceMakeBitmap(VU_METER_WIDTH, VU_METER_HEIGHT, 24, pBits);
	}
	mir_free(pBits);
}

static INT_PTR CALLBACK TlenVoiceDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hDC, hMemDC;
	int v;
	static int counter;
	TlenProtocol *proto = (TlenProtocol *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		proto = (TlenProtocol *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)proto);
		proto->voiceDlgHWND = hwndDlg;
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_VCQUALITY, CB_ADDSTRING, 0, (LPARAM)_T("8000 Hz / 13.8 kbps"));
		SendDlgItemMessage(hwndDlg, IDC_VCQUALITY, CB_ADDSTRING, 0, (LPARAM)_T("11025 Hz / 19.1 kbps"));
		SendDlgItemMessage(hwndDlg, IDC_VCQUALITY, CB_ADDSTRING, 0, (LPARAM)_T("22050 Hz / 36.8 kbps"));
		SendDlgItemMessage(hwndDlg, IDC_VCQUALITY, CB_ADDSTRING, 0, (LPARAM)_T("44100 Hz / 72 kbps"));
		SendDlgItemMessage(hwndDlg, IDC_VCQUALITY, CB_SETCURSEL, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_MICROPHONE, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_SPEAKER, BUTTONSETASFLATBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_MICROPHONE, BUTTONSETASPUSHBTN, TRUE, 0);
		SendDlgItemMessage(hwndDlg, IDC_SPEAKER, BUTTONSETASPUSHBTN, TRUE, 0);
		{
			HICON hIcon = GetIcolibIcon(IDI_MICROPHONE);
			SendDlgItemMessage(hwndDlg, IDC_MICROPHONE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			ReleaseIcolibIcon(hIcon);
			hIcon = GetIcolibIcon(IDI_SPEAKER);
			SendDlgItemMessage(hwndDlg, IDC_SPEAKER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			ReleaseIcolibIcon(hIcon);
		}
		CheckDlgButton(hwndDlg, IDC_MICROPHONE, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_SPEAKER, BST_CHECKED);
		TlenVoiceInitVUMeters();
		SetDlgItemText(hwndDlg, IDC_STATUS, TranslateT("...???..."));
		counter = 0;
		SetTimer(hwndDlg, 1, 100, NULL);
		return FALSE;
	case WM_TIMER:
		if (proto->recordingControl != NULL && !proto->recordingControl->bDisable) {
			v = proto->recordingControl->vuMeter % VU_METER_LEVELS;
			if (proto->recordingControl->vuMeter > 0) {
				proto->recordingControl->vuMeter--;
			}
		}
		else {
			v = 0;
		}
		hDC = GetDC(GetDlgItem(hwndDlg, IDC_VUMETERIN));
		if (NULL != (hMemDC = CreateCompatibleDC(hDC))) {
			SelectObject(hMemDC, vuMeterBitmaps[v]);
			BitBlt(hDC, 0, 0, VU_METER_WIDTH, VU_METER_HEIGHT, hMemDC, 0, 0, SRCCOPY);
			DeleteDC(hMemDC);
		}
		ReleaseDC(GetDlgItem(hwndDlg, IDC_PLAN), hDC);
		if (proto->playbackControl != NULL  && !proto->playbackControl->bDisable) {
			v = proto->playbackControl->vuMeter % VU_METER_LEVELS;
			if (proto->playbackControl->vuMeter > 0) {
				proto->playbackControl->vuMeter--;
			}
		}
		else {
			v = 0;
		}
		hDC = GetDC(GetDlgItem(hwndDlg, IDC_VUMETEROUT));
		if (NULL != (hMemDC = CreateCompatibleDC(hDC))) {
			SelectObject(hMemDC, vuMeterBitmaps[v]);
			BitBlt(hDC, 0, 0, VU_METER_WIDTH, VU_METER_HEIGHT, hMemDC, 0, 0, SRCCOPY);
			DeleteDC(hMemDC);
		}
		ReleaseDC(GetDlgItem(hwndDlg, IDC_PLAN), hDC);
		counter++;
		if (counter % 10 == 0) {
			char str[50];
			float fv;
			if (proto->recordingControl != NULL) {
				fv = (float)proto->recordingControl->bytesSum;
				proto->recordingControl->bytesSum = 0;
			}
			else {
				fv = 0;
			}
			mir_snprintf(str, SIZEOF(str), "%.1f kB/s", fv / 1024);
			SetDlgItemTextA(hwndDlg, IDC_BYTESOUT, str);
			if (proto->playbackControl != NULL) {
				fv = (float)proto->playbackControl->bytesSum;
				proto->playbackControl->bytesSum = 0;
			}
			else {
				fv = 0;
			}
			mir_snprintf(str, SIZEOF(str), "%.1f kB/s", fv / 1024);
			SetDlgItemTextA(hwndDlg, IDC_BYTESIN, str);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		case IDC_VCQUALITY:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				if (proto->recordingControl != NULL) {
					int codec = SendDlgItemMessage(hwndDlg, IDC_VCQUALITY, CB_GETCURSEL, 0, 0) + 2;
					if (codec != proto->recordingControl->codec && codec > 1 && codec < 6) {
						TLEN_FILE_TRANSFER *ft = proto->recordingControl->ft;
						TlenVoiceFreeVc(proto->recordingControl);
						proto->recordingControl = TlenVoiceCreateVC(ft->proto, codec);
						proto->recordingControl->ft = ft;
						TlenVoiceRecordingStart(proto->recordingControl);
					}
				}
			}
			break;
		case IDC_MICROPHONE:
			if (proto->recordingControl != NULL) {
				proto->recordingControl->bDisable = BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_MICROPHONE);
			}
			break;
		case IDC_SPEAKER:
			if (proto->playbackControl != NULL) {
				proto->playbackControl->bDisable = BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SPEAKER);
			}
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		proto->voiceDlgHWND = NULL;
		break;

	}
	return FALSE;
}

static void __cdecl TlenVoiceDlgThread(void *ptr)
{

	TLEN_FILE_TRANSFER *ft = (TLEN_FILE_TRANSFER *)ptr;
	TlenProtocol *proto = ft->proto;
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VOICE), NULL, TlenVoiceDlgProc, (LPARAM)proto);
	TlenVoiceCancelAll(proto);
}

int TlenVoiceStart(TLEN_FILE_TRANSFER *ft, int mode)
{

	ft->proto->debugLogA("starting voice %d", mode);
	if (mode == 0) {
		forkthread((void(__cdecl *)(void*))TlenVoiceReceiveThread, 0, ft);
	}
	else if (mode == 1) {
		forkthread((void(__cdecl *)(void*))TlenVoiceSendingThread, 0, ft);
	}
	else {
		forkthread((void(__cdecl *)(void*))TlenVoiceDlgThread, 0, ft);
	}
	return 0;
}

static char *getDisplayName(TlenProtocol *proto, const char *id)
{
	char jid[256];
	MCONTACT hContact;
	DBVARIANT dbv;
	if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
		mir_snprintf(jid, SIZEOF(jid), "%s@%s", id, dbv.pszVal);
		db_free(&dbv);
		if ((hContact = TlenHContactFromJID(proto, jid)) != NULL)
			return mir_strdup((char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, 0));
	}
	return mir_strdup(id);
}

typedef struct {
	TlenProtocol *proto;
	TLEN_LIST_ITEM *item;
}ACCEPTDIALOGDATA;

static INT_PTR CALLBACK TlenVoiceAcceptDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ACCEPTDIALOGDATA * data;
	char *str;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		data = (ACCEPTDIALOGDATA *)lParam;
		str = getDisplayName(data->proto, data->item->nick);
		SetDlgItemTextA(hwndDlg, IDC_FROM, str);
		mir_free(str);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ACCEPT:
			EndDialog(hwndDlg, 1);
			return TRUE;
		case IDCANCEL:
		case IDCLOSE:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	}
	return FALSE;
}

static void __cdecl TlenVoiceAcceptDlgThread(void *ptr)
{

	ACCEPTDIALOGDATA *data = (ACCEPTDIALOGDATA *)ptr;
	int result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ACCEPT_VOICE), NULL, TlenVoiceAcceptDlgProc, (LPARAM)data);
	if (result && data->proto->isOnline) {
		data->item->ft = TlenFileCreateFT(data->proto, data->item->nick);
		data->item->ft->iqId = mir_strdup(data->item->jid);
		TlenVoiceStart(data->item->ft, 2);
		TlenSend(data->proto, "<v t='%s' i='%s' e='5' v='1'/>", data->item->nick, data->item->jid);
	}
	else {
		if (data->proto->isOnline) {
			TlenSend(data->proto, "<v t='%s' i='%s' e='4' />", data->item->nick, data->item->jid);
		}
		TlenListRemove(data->proto, LIST_VOICE, data->item->jid);
	}
	mir_free(data);
}

int TlenVoiceAccept(TlenProtocol *proto, const char *id, const char *from)
{
	if (!TlenVoiceIsInUse(proto)) {
		TLEN_LIST_ITEM *item = TlenListAdd(proto, LIST_VOICE, id);
		if (item != NULL) {
			bool ask = true, ignore = false;
			int voiceChatPolicy = db_get_w(NULL, proto->m_szModuleName, "VoiceChatPolicy", 0);
			if (voiceChatPolicy == TLEN_MUC_ASK) {
				ignore = false;
				ask = true;
			}
			else if (voiceChatPolicy == TLEN_MUC_IGNORE_ALL) {
				ignore = true;
			}
			else if (voiceChatPolicy == TLEN_MUC_IGNORE_NIR) {
				char jid[256];
				DBVARIANT dbv;
				if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
					mir_snprintf(jid, SIZEOF(jid), "%s@%s", from, dbv.pszVal);
					db_free(&dbv);
				}
				else {
					mir_strncpy(jid, from, SIZEOF(jid) - 1);
				}
				ignore = !IsAuthorized(proto, jid);
				ask = true;
			}
			else if (voiceChatPolicy == TLEN_MUC_ACCEPT_IR) {
				char jid[256];
				DBVARIANT dbv;
				if (!db_get(NULL, proto->m_szModuleName, "LoginServer", &dbv)) {
					mir_snprintf(jid, SIZEOF(jid), "%s@%s", from, dbv.pszVal);
					db_free(&dbv);
				}
				else {
					mir_strncpy(jid, from, SIZEOF(jid) - 1);
				}
				ask = !IsAuthorized(proto, jid);
				ignore = false;
			}
			else if (voiceChatPolicy == TLEN_MUC_ACCEPT_ALL) {
				ask = false;
				ignore = false;
			}
			if (ignore) {
				if (proto->isOnline) {
					TlenSend(proto, "<v t='%s' i='%s' e='4' />", from, id);
				}
				TlenListRemove(proto, LIST_VOICE, id);
			}
			else {
				item->nick = mir_strdup(from);
				if (ask) {
					ACCEPTDIALOGDATA *data = (ACCEPTDIALOGDATA *)mir_alloc(sizeof(ACCEPTDIALOGDATA));
					data->proto = proto;
					data->item = item;
					forkthread((void(__cdecl *)(void*))TlenVoiceAcceptDlgThread, 0, data);
				}
				else if (proto->isOnline) {
					item->ft = TlenFileCreateFT(proto, from);
					item->ft->iqId = mir_strdup(id);
					TlenVoiceStart(item->ft, 2);
					TlenSend(proto, "<v t='%s' i='%s' e='5' v='1'/>", item->nick, item->jid);
				}
			}
			return 1;
		}
	}
	return 0;
}

int TlenVoiceBuildInDeviceList(TlenProtocol *proto, HWND hWnd)
{
	int i, j, iNumDevs;
	WAVEINCAPS     wic;
	iNumDevs = waveInGetNumDevs();
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)TranslateT("Default"));
	for (i = j = 0; i < iNumDevs; i++) {
		if (!waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS))) {
			if (wic.dwFormats != 0) {
				SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)wic.szPname);
				j++;
			}
		}
	}
	i = db_get_w(NULL, proto->m_szModuleName, "VoiceDeviceIn", 0);
	if (i > j) i = 0;
	SendMessage(hWnd, CB_SETCURSEL, i, 0);
	return 0;
}

int TlenVoiceBuildOutDeviceList(TlenProtocol *proto, HWND hWnd)
{
	int i, j, iNumDevs;
	WAVEOUTCAPS  woc;
	iNumDevs = waveInGetNumDevs();
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)TranslateT("Default"));
	for (i = j = 0; i < iNumDevs; i++) {
		if (!waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS))) {
			if (woc.dwFormats != 0) {
				SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)woc.szPname);
				j++;
			}
		}
	}
	i = db_get_w(NULL, proto->m_szModuleName, "VoiceDeviceOut", 0);
	if (i > j) i = 0;
	SendMessage(hWnd, CB_SETCURSEL, i, 0);
	return 0;
}
