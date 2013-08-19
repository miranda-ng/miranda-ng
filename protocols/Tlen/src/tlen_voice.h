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

#ifndef _TLEN_VOICE_H_
#define _TLEN_VOICE_H_

#include <windows.h>
#include <mmsystem.h>

#include "tlen.h"
extern "C" {
	#include "codec/gsm.h"
}

typedef struct TLEN_VOICE_CONTROL_STRUCT {
	int			waveMode;
	int			codec;
	int			bDisable;

	short		*recordingData;
	short		*waveData;
	WAVEHDR		*waveHeaders;
	int			waveFrameSize;
	int			waveHeadersPos;
	int			waveHeadersNum;

//	HANDLE		hEvent;
	HANDLE		hThread;
	DWORD		threadID;
	HWAVEOUT	hWaveOut;
	HWAVEIN		hWaveIn;
	int			isRunning;
	int			stopThread;
	gsm_state	*gsmstate;
	TLEN_FILE_TRANSFER *ft;
	int			vuMeter;
	int			bytesSum;
	TlenProtocol *proto;
} TLEN_VOICE_CONTROL;


void __cdecl TlenVoiceSendingThread(TLEN_FILE_TRANSFER *ft);
void __cdecl TlenVoiceReceiveThread(TLEN_FILE_TRANSFER *ft);

int TlenVoiceStart(TLEN_FILE_TRANSFER *ft, int mode) ;
int TlenVoiceAccept(TlenProtocol *proto, const char *id, const char *from);
int TlenVoiceIsInUse(TlenProtocol *proto);
int TlenVoiceCancelAll(TlenProtocol *proto);
int TlenVoiceBuildInDeviceList(TlenProtocol *proto, HWND hWnd);
int TlenVoiceBuildOutDeviceList(TlenProtocol *proto, HWND hWnd);

#endif

