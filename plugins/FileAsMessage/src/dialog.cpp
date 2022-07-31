#include "stdafx.h"

char *szFEMode[] =
{
	LPGEN("Receive file"),
	LPGEN("Send file")
};

char* ltoax(char* s, uint32_t value)
{
	if (value == 0)
		*s++ = '0';

	uchar data;
	int indx = 8;
	while (indx && !(data = (uchar)(value >> 28) & 0x0F)) {
		value <<= 4;
		indx--;
	}
	while (indx) {
		data = (uchar)(value >> 28) & 0x0F;
		if (data > 9) data += 'A' - 10;
		else data += '0';
		*s++ = data;
		value <<= 4;
		indx--;
	}
	return s;
}

uint atolx(char* &value)
{
	uint result = 0;
	uchar ch;

	while (*value && (ch = *value - '0') >= 0) {
		if (ch > 9) {
			ch -= 'A' - '0';
			if (ch > 5) break;
			ch += 10;
		}
		result = result * 16 + ch;
		value++;
	}
	return result;
}

char cCmdList[CMD_COUNT] =
{
	'?',
	'+',
	'-',

	'*',

	'>',
	'!',
	'.'
};

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = PUGetContact(hWnd);
	HWND hDlg = (HWND)PUGetPluginData(hWnd);

	switch (message) {
	case WM_COMMAND:
	{
		PUDeletePopup(hWnd);
		g_clistApi.pfnRemoveEvent(hContact, 0);

		if (IsWindow(hDlg)) {
			ShowWindow(hDlg, SW_SHOWNORMAL);
			SetForegroundWindow(hDlg);
			SetFocus(hDlg);
		}

		break;
	}
	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		break;
	case UM_FREEPLUGINDATA:
		return TRUE; //TRUE or FALSE is the same, it gets ignored.
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Just create simple Popup for specified contact

void MakePopupMsg(HWND hDlg, MCONTACT hContact, wchar_t *msg)
{
	HWND hFocused = GetForegroundWindow();
	if (hDlg == hFocused || hDlg == GetParent(hFocused)) return;

	POPUPDATAW ppd;
	ppd.lchContact = hContact;
	ppd.lchIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_SMALLICON));
	mir_wstrcpy(ppd.lpwzContactName, Clist_GetContactDisplayName(hContact));
	mir_wstrcpy(ppd.lpwzText, msg);
	ppd.colorBack = GetSysColor(COLOR_INFOBK);
	ppd.colorText = GetSysColor(COLOR_INFOTEXT);
	ppd.PluginWindowProc = PopupDlgProc;
	ppd.PluginData = (void*)hDlg;
	ppd.iSeconds = -1;
	PUAddPopupW(&ppd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get ID of string message

int getMsgId(char *msg)
{
	for (int indx = 0; indx < CMD_COUNT; indx++)
		if (*msg == cCmdList[indx])
			return indx;

	return -1;
};

int RetrieveFileSize(wchar_t *filename)
{
	FILE *hFile = _wfopen(filename, L"rb");
	if (hFile != nullptr) {
		fseek(hFile, 0, SEEK_END);
		int size = ftell(hFile);
		fclose(hFile);
		return size;
	}
	return 0;
}

FILEECHO::FILEECHO(MCONTACT Contact)
{
	hContact = Contact;
	dwSendInterval = g_plugin.getDword("SendDelay", 6000);

	chunkMaxLen = g_plugin.getDword("ChunkSize", 5000);
	chunkCount = 0;
	filename = nullptr;

	rgbRecv = g_plugin.getDword("colorRecv", RGB(64, 255, 64));
	rgbSent = g_plugin.getDword("colorSent", RGB(255, 255, 64));
	rgbUnSent = g_plugin.getDword("colorUnsent", RGB(128, 128, 128));
	rgbToSend = g_plugin.getDword("colorTosend", RGB(192, 192, 192));
	asBinary = g_plugin.getDword("base64", 1) == 0;
}

uint controlEnabled[][2] =
{
	IDC_PLAY,
	STATE_OPERATE | STATE_PAUSED | STATE_PRERECV | STATE_ACKREQ | STATE_IDLE,
	IDC_STOP,
	STATE_OPERATE | STATE_PAUSED | STATE_PRERECV | STATE_REQSENT | STATE_ACKREQ,
};

char *hint_controls[4] =
{
	"Perform",
	"Pause",
	"Revive a transfer",
	"Stop"
};

void FILEECHO::setState(uint32_t state)
{
	iState = state;

	for (int indx = 0; indx < _countof(controlEnabled); indx++)
		EnableWindow(GetDlgItem(hDlg, controlEnabled[indx][0]), (iState & controlEnabled[indx][1]) != 0);

	if (!inSend) {
		// recv
		int kind;
		SendDlgItemMessage(hDlg, IDC_FILENAME, EM_SETREADONLY, (state != STATE_PRERECV), 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BROWSE), (iState & (STATE_PRERECV | STATE_FINISHED)));

		if (state & (STATE_IDLE | STATE_FINISHED | STATE_CANCELLED | STATE_PRERECV))
			kind = ICON_PLAY;
		else
			kind = ICON_REFRESH;
		SendDlgItemMessage(hDlg, IDC_PLAY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[kind]);
		SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONADDTOOLTIP, (WPARAM)Translate(hint_controls[kind]), 0);
	}
	else {
		SendDlgItemMessage(hDlg, IDC_FILENAME, EM_SETREADONLY, (iState & (STATE_IDLE | STATE_FINISHED | STATE_CANCELLED)) == 0, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BROWSE), (iState & (STATE_IDLE | STATE_FINISHED | STATE_CANCELLED)) != 0);

		switch (state) {
		case STATE_FINISHED:
		case STATE_CANCELLED:
		case STATE_IDLE:
		case STATE_PAUSED:
			EnableWindow(GetDlgItem(hDlg, IDC_PLAY), TRUE);
			SendDlgItemMessage(hDlg, IDC_PLAY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[ICON_PLAY]);
			SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONADDTOOLTIP, (WPARAM)Translate(hint_controls[ICON_PLAY]), 0);
			break;
		case STATE_OPERATE:
			SendDlgItemMessage(hDlg, IDC_PLAY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[ICON_PAUSE]);
			SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONADDTOOLTIP, (WPARAM)Translate(hint_controls[ICON_PAUSE]), 0);
			break;
		}
	}
	updateProgress();
}

void FILEECHO::updateTitle()
{
	char newtitle[256];

	char *contactName = _T2A(Clist_GetContactDisplayName(hContact));
	if (iState == STATE_OPERATE && chunkCount != 0)
		mir_snprintf(newtitle, "%d%% - %s: %s", chunkSent * 100 / chunkCount, Translate(szFEMode[inSend]), contactName);
	else
		mir_snprintf(newtitle, "%s: %s", Translate(szFEMode[inSend]), contactName);
	SetWindowTextA(hDlg, newtitle);
}

void BuildFreqTable(uchar *data, uint len, uint *freqTable)
{
	memset(freqTable, 0, (256 * sizeof(uint)));
	for (uint indx = 0; indx < len; indx++)
		freqTable[data[indx]]++;
}

int FILEECHO::createTransfer()
{
	uint LastError;
	hFile = INVALID_HANDLE_VALUE;
	hMapping = nullptr;
	lpData = nullptr;
#ifdef DEBUG
	overhead = 0;
#endif
	uint8_t bAuto = db_get_b(0, "SRFile", "AutoAccept", 0);

	hFile = CreateFileA(filename, inSend ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE), inSend ? FILE_SHARE_READ : 0, nullptr, inSend ? OPEN_EXISTING : (bAuto ? CREATE_ALWAYS : CREATE_NEW), FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE && !inSend && GetLastError() == ERROR_FILE_EXISTS) {
		if (MessageBox(hDlg, TranslateT("File already exists. Overwrite?"),
			TranslateT(SERVICE_TITLE),
			MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return 0;
		hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0,
			nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	}
	if (hFile == INVALID_HANDLE_VALUE)
		goto createTransfer_FAILED;

	if (!inSend) {
		SetFilePointer(hFile, fileSize, nullptr, FILE_BEGIN);
		SetEndOfFile(hFile);
	}
	else fileSize = GetFileSize(hFile, nullptr);

	hMapping = CreateFileMapping(hFile, nullptr, inSend ? PAGE_READONLY : PAGE_READWRITE, 0, fileSize, nullptr);
	LastError = GetLastError();
	if (hMapping == nullptr)
		goto createTransfer_FAILED;

	lpData = (uchar*)MapViewOfFile(hMapping, inSend ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	LastError = GetLastError();
	if (lpData == nullptr)
		goto createTransfer_FAILED;

	if (inSend) {
		// frequency analysis of source file and building the table of offsets
		if (asBinary) {
			uint freq_table[256] = { 0 };
			codeSymb = 1;

			// searching for symbol with lowest frequency: "codeSymb"
			BuildFreqTable(lpData, fileSize, freq_table);
			for (uint i = codeSymb + 1; i < 256; ++i) {
				if (freq_table[codeSymb] > freq_table[i])
					codeSymb = i;
			}

			// calculating chunks sizes
			// build table of chunks offsets: chunkPos
			uint chunk_count_limit = 2 * fileSize / chunkMaxLen + 2;
			chunkPos = (uint*)calloc(chunk_count_limit, sizeof(uint));

			uchar *data = lpData;
			uint chunk_size = 0;
			uint out_size = 0;
			uint indx = 0;
			uint chunk_offset = 0;

			for (uint len = fileSize; len; --len) {
				if (*data == 0 || *data == codeSymb)
					out_size += 2;
				else
					++out_size;

				data++; chunk_size++;
				if (out_size >= chunkMaxLen - 1) {
					chunkPos[indx] = chunk_offset;
					chunk_offset += chunk_size;
					chunk_size = 0; out_size = 0;
					++indx;
				}
			}
			chunkPos[indx++] = chunk_offset; chunkCount = indx;
			chunkPos = (uint*)realloc(chunkPos, sizeof(uint)*(chunkCount + 1));
			chunkPos[indx] = chunk_offset + chunk_size;
		}
		else {
			int EncodedMaxLen = Netlib_GetBase64EncodedBufferSize(Netlib_GetBase64DecodedBufferSize(chunkMaxLen));
			int DecodedMaxLen = Netlib_GetBase64DecodedBufferSize(EncodedMaxLen);

			codeSymb = '-';
			chunkCount = (fileSize + DecodedMaxLen - 1) / DecodedMaxLen;
			chunkPos = (uint*)calloc(chunkCount + 1, sizeof(uint));

			uint i = 0;
			for (uint chunk_offset = 0; i < chunkCount; ++i, chunk_offset += DecodedMaxLen) {
				chunkPos[i] = chunk_offset;
			}

			chunkPos[i] = chunkPos[i - 1] + (fileSize % DecodedMaxLen);
		}
	}
	else chunkCount = chunkCountx;

	chunkAck = (uchar*)calloc(chunkCount, sizeof(uchar));
	chunkIndx = 0; chunkSent = 0;
	return 1;

createTransfer_FAILED:
	if (lpData != nullptr) UnmapViewOfFile(lpData);
	if (hMapping != nullptr) CloseHandle(hMapping);
	if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	return 0;
}

void FILEECHO::destroyTransfer()
{
	if (chunkCount) {
		chunkCount = 0;
		if (inSend)
			free(chunkPos);
		free(chunkAck);
		if (lpData != nullptr) UnmapViewOfFile(lpData);
		if (hMapping != nullptr) CloseHandle(hMapping);
		if (hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	}
}

void FILEECHO::sendReq()
{
	char sendbuf[MAX_PATH];

	if (!createTransfer()) {
		SetDlgItemText(hDlg, IDC_FILESIZE, TranslateT("Couldn't open a file"));
		return;
	}

	///!!!!!!!
	char *p = filename + mir_strlen(filename);
	while (p != filename && *p != '\\')
		p--;
	if (*p == '\\')
		mir_strcpy(filename, p + 1);

	mir_snprintf(sendbuf, Translate("Size: %d bytes"), fileSize);
	SetDlgItemTextA(hDlg, IDC_FILESIZE, sendbuf);
	mir_snprintf(sendbuf, "?%c%c%d:%d \n" NOPLUGIN_MESSAGE, asBinary + '0', codeSymb, chunkCount, fileSize);
	sendCmd(0, CMD_REQ, sendbuf, filename);

	SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Request sent. Awaiting of acceptance.."));
	setState(STATE_REQSENT);
}

void FILEECHO::incomeRequest(char *param)
{
	// param: filename?cCOUNT:SIZE
	char buf[MAX_PATH];
	// param == &filename
	char *p = strchr(param, '?');
	if (p == nullptr) return; *p++ = 0;
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, hContact, (LPARAM)buf);
	mir_strncat(buf, param, _countof(buf) - mir_strlen(buf));
	free(filename);
	filename = mir_strdup(buf);
	// p == &c
	if (*p == 0) return; asBinary = (*p++) != '0';
	if (*p == 0) return; codeSymb = *p++;
	// p == &COUNT
	if (*p == 0) return; param = strchr(p, ':');
	// param == &SIZE
	if (param == nullptr) return; *param++ = 0;
	if (*param == 0) return;
	chunkCountx = atoi(p);
	fileSize = atoi(param);

	mir_snprintf(buf, Translate("Size: %d bytes"), fileSize);
	SetDlgItemTextA(hDlg, IDC_FILENAME, filename);
	SetDlgItemTextA(hDlg, IDC_FILESIZE, buf);

	setState(STATE_PRERECV);
	inSend = FALSE;

	Skin_PlaySound("RecvFile");
	int AutoMin = db_get_b(0, "SRFile", "AutoMin", 0);
	if (db_get_b(0, "SRFile", "AutoAccept", 0) && Contact::OnList(hContact)) {
		PostMessage(hDlg, WM_COMMAND, IDC_PLAY, 0);
		if (AutoMin)
			ShowWindow(hDlg, SW_SHOWMINIMIZED);
		//		ShowWindow(hDlg, SW_MINIMIZE);
		//		UpdateWindow(hDlg);
	}
	//	else
	if (!IsWindowVisible(hDlg) && !AutoMin) {
		CLISTEVENT cle = {};
		cle.hContact = hContact;
		cle.hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_SMALLICON));
		cle.flags = CLEF_URGENT;
		cle.hDbEvent = 0;
		cle.pszService = MODULENAME "/FERecvFile";
		g_clistApi.pfnAddEvent(&cle);

		MakePopupMsg(hDlg, hContact, L"Incoming file...");
	}
}

void FILEECHO::cmdACCEPT()
{
	if (chunkCount == 0) return;
	setState(STATE_OPERATE);
	SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Sending..."));
	lastTimestamp = GetTickCount();

	SetTimer(hDlg, TIMER_SEND, dwSendInterval, nullptr);
}

void FILEECHO::updateProgress()
{
	InvalidateRect(GetDlgItem(hDlg, IDC_PROGRESS), nullptr, TRUE);
	updateTitle();
}

/////////////////////////////////////////////////////////////////////////////////////////
// called in receive mode
// used to transfer acknowledge

void FILEECHO::onRecvTimer()
{
	if (chunkCount == 0) return;
	char *buffer = (char*)malloc(1024);
	char *p = buffer;
	uchar prev_value;
	uint indx, jndx;

	KillTimer(hDlg, TIMER_SEND);

	// Build response about successfully received chunks
	indx = jndx = 0; prev_value = chunkAck[jndx];
	while (jndx < chunkCount) {
		if (chunkAck[jndx] != prev_value) {
			if (prev_value != CHUNK_ACK) {
				p = ltoax(p, indx);
				if (indx != jndx - 1) {
					*p++ = '-';
					p = ltoax(p, jndx - 1);
				}
				*p++ = ',';
			}
			indx = jndx;
			prev_value = chunkAck[jndx];
		}
		jndx++;
	}
	if (prev_value != CHUNK_ACK) {
		p = ltoax(p, indx);
		if (indx != jndx - 1) {
			*p++ = '-';
			p = ltoax(p, jndx - 1);
		}
	}
	*p = 0;
	if (*buffer == 0) {
		wchar_t *msg = TranslateT("Received successfully");
		SetDlgItemText(hDlg, IDC_STATUS, msg);
		MakePopupMsg(hDlg, hContact, msg);
		setState(STATE_FINISHED);
		if (db_get_b(0, "SRFile", "AutoClose", 0)) {
			PostMessage(hDlg, WM_CLOSE, 0, 0);
			g_clistApi.pfnRemoveEvent(hContact, 0);
		}
		Skin_PlaySound("FileDone");
		destroyTransfer();
		buffer[0] = 'x'; buffer[1] = 0;
	}
	sendCmd(0, CMD_DACK, buffer);
	free(buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////
// called in sending mode
// used to data transfer and
// sending of scheduled commands

void FILEECHO::onSendTimer()
{
	if (chunkCount == 0) return;

	// perform request of acknowledge, if scheduled
	KillTimer(hDlg, TIMER_SEND);

	// Search for next unsent chunk
	while (chunkIndx < chunkCount && chunkAck[chunkIndx] != CHUNK_UNSENT) chunkIndx++;
	if (iState == STATE_ACKREQ || chunkIndx == chunkCount) {
		SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Requesting of missing chunks"));
		setState(STATE_OPERATE);
		sendCmd(0, CMD_END, "", nullptr);
		chunkIndx = chunkCount + 1;
		return;
	}
	if (chunkIndx > chunkCount) return;

	uchar *buffer = (uchar*)malloc(chunkMaxLen * 2);
	uchar *p = buffer;
	uchar *data = lpData + chunkPos[chunkIndx];
	uchar *data_end = lpData + chunkPos[chunkIndx + 1];
	ulong chksum = memcrc32(data, data_end - data, INITCRC);

	if (asBinary) {
		// Encoding data to transfer with symb. filtering
		while (data < data_end) {
			uchar ch = *data++;
			if (ch == 0) {
				*p++ = codeSymb; *p++ = '0';
			}
			else if (ch == codeSymb) {
				*p++ = codeSymb; *p++ = '+';
			}
			else
				*p++ = ch;
		}
		*p = 0;
	}
	else {
		ptrA enc(mir_base64_encode(data, data_end - data));
		mir_strncpy((char*)buffer, enc, chunkMaxLen * 2);
	}

	char prefix[128];
	mir_snprintf(prefix, "%X,%X,%X>", chunkIndx + 1, chunkPos[chunkIndx], chksum);
#ifdef DEBUG
	overhead += mir_strlen((char*)buffer);
#endif
	sendCmd(0, CMD_DATA, (char*)buffer, (char*)prefix);
	chunkAck[chunkIndx] = CHUNK_SENT;

	free(buffer);

	chunkIndx++; chunkSent++;

	if (chunkIndx == chunkCount)
		setState(STATE_ACKREQ);
	else {
		SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Sending..."));
		updateProgress();
	}
	SetTimer(hDlg, TIMER_SEND, dwSendInterval, nullptr);
}

void FILEECHO::cmdDATA(char *param)
{
	if (chunkCount == 0) return;
	chunkIndx = atolx(param); param++;
	if (chunkIndx-- == 0) return;
	uint filepos = atolx(param); param++;
	if (filepos >= fileSize) return;

	ulong chksum_local;
	ulong chksum_remote = atolx(param); param++;

	KillTimer(hDlg, TIMER_SEND);

	// Decoding of incoming data
	uchar *data = lpData + filepos;
	uchar *data_end = lpData + fileSize;
	if (asBinary) {
		uchar ch;
		while (ch = *param++) {
			if (ch == codeSymb) {
				if ((ch = *param++) == 0)
					return;

				switch (ch) {
				case '+':
					ch = codeSymb;
					break;
				case '0':
					ch = 0;
					break;
				default:
					return;
				}
			}
			if (data > data_end)
				return;
			*data++ = ch;
		}
	}
	else {
		size_t bufLen;
		mir_ptr<uint8_t> buf((uint8_t*)mir_base64_decode(param, &bufLen));
		memcpy(data, buf, min(bufLen, unsigned(data_end - data)));
		data += bufLen;
	}

	// let's check it up
	chksum_local = memcrc32(lpData + filepos, data - (lpData + filepos), INITCRC);
	if (chksum_local == chksum_remote) {
		if (chunkAck[chunkIndx] != CHUNK_ACK) chunkSent++;
		chunkAck[chunkIndx] = CHUNK_ACK;
	}
	SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Receiving..."));
	updateProgress();
}

void FILEECHO::cmdEND()
{
	SetTimer(hDlg, TIMER_SEND, dwSendInterval, nullptr);
}

void FILEECHO::cmdDACK(char *param)
{
	uint indx, jndx;

	if (chunkCount == 0) return;
	memset(chunkAck, CHUNK_ACK, sizeof(uchar)*chunkCount);

	// All chunks has been received successfully
	if (*param == 'x') {
		wchar_t *msg = TranslateT("Sent successfully");
		SetDlgItemText(hDlg, IDC_STATUS, msg);

		Skin_PlaySound("FileDone");
		destroyTransfer();
		MakePopupMsg(hDlg, hContact, msg);
		setState(STATE_FINISHED);
		return;
	}
	chunkSent = chunkCount;

	// Mark chunks to re-transfer,
	// according received info
	// 
	// format: chunk1, chunk3-chunk10, etc..

	while (*param) {
		indx = atolx(param);
		if (*param == '-') {
			param++; jndx = atolx(param);
		}
		else jndx = indx;

		if (*param == 0 || *param == ',') {
			for (uint p = indx; p <= jndx; p++) {
				if (p < chunkCount) {
					chunkAck[p] = CHUNK_UNSENT;
					chunkSent--;
				}
			}
			if (*param == ',')
				param++;
		}
	}
	updateProgress();

	// retransfer some parts
	chunkIndx = 0;
	SetTimer(hDlg, TIMER_SEND, dwSendInterval, nullptr);
}

void FILEECHO::perform(char *str)
{
	int msgId = getMsgId(str);
	if (msgId == -1) {
		MakePopupMsg(hDlg, hContact, TranslateT("Unknown command for \"File As Message\" was received"));
		return;
	}
	if (inSend)
		switch (msgId) {
		case CMD_REQ:
			if (MessageBox(hDlg, TranslateT("Incoming file request. Do you want to proceed?"),
				TranslateT(SERVICE_TITLE), MB_YESNO | MB_ICONWARNING) == IDYES) {
				SetDlgItemText(hDlg, IDC_STATUS, L"");
				SendMessage(hDlg, WM_COMMAND, IDC_STOP, 0);

				incomeRequest(str + 1);
				updateTitle();
				break;
			}
			break;
		case CMD_ACCEPT:
			cmdACCEPT();
			break;
		case CMD_CANCEL:
		{
			if (iState & (STATE_PRERECV | STATE_REQSENT | STATE_OPERATE | STATE_ACKREQ | STATE_PAUSED)) {
				wchar_t *msg = TranslateT("Canceled by remote user");
				SetDlgItemText(hDlg, IDC_STATUS, msg);
				MakePopupMsg(hDlg, hContact, msg);
				destroyTransfer();
				setState(STATE_CANCELLED);
			}
			break;
		}
		case CMD_DACK:
			cmdDACK(str + 1);
			break;
		}
	else
		switch (msgId) {
		case CMD_CANCEL:
		{
			if (iState & (STATE_PRERECV | STATE_REQSENT | STATE_OPERATE | STATE_ACKREQ | STATE_PAUSED)) {
				wchar_t *msg = TranslateT("Canceled by remote user");
				SetDlgItemText(hDlg, IDC_STATUS, msg);
				MakePopupMsg(hDlg, hContact, msg);
				destroyTransfer();
				setState(STATE_CANCELLED);
			}
			break;
		}

		case CMD_REQ:
			if (chunkCount) {
				if (MessageBox(hDlg, TranslateT("New incoming file request. Do you want to proceed?"),
					TranslateT(SERVICE_TITLE), MB_YESNO | MB_ICONWARNING) != IDYES)
					break;
				//sendCmd(0, CMD_CANCEL, "", NULL);
				destroyTransfer();
			}
			SetDlgItemText(hDlg, IDC_STATUS, L"");
			incomeRequest(str + 1);
			break;

		case CMD_DATA:
			cmdDATA(str + 1);
			break;

		case CMD_END:
			cmdEND();
			break;
		};
};

int FILEECHO::sendCmd(int, int cmd, char *szParam, char *szPrefix)
{
	int buflen = (int)mir_strlen(szServicePrefix) + (int)mir_strlen(szParam) + 2;
	if (szPrefix != nullptr)
		buflen += (int)mir_strlen(szPrefix);

	char *buf = (char*)malloc(buflen);
	if (szPrefix == nullptr)
		mir_snprintf(buf, buflen, "%s%c%s", szServicePrefix, cCmdList[cmd], szParam);
	else
		mir_snprintf(buf, buflen, "%s%c%s%s", szServicePrefix, cCmdList[cmd], szPrefix, szParam);

	int retval = ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)buf);
	free(buf);
	updateProgress();
	return retval;
}


LRESULT CALLBACK ProgressWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_NCPAINT:
		return 0;

	case WM_PAINT:
	{
		HDC hdc;
		PAINTSTRUCT ps;
		RECT rc;
		HRGN hrgn;
		HBRUSH frameBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		FILEECHO *dat = (FILEECHO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

		GetClientRect(hwnd, &rc);
		if (dat == nullptr || dat->chunkCount == 0) {
			COLORREF colour;
			HBRUSH hbr;

			if (dat == nullptr || dat->iState != STATE_FINISHED) {
				hbr = (HBRUSH)(COLOR_3DFACE + 1);
			}
			else {
				colour = dat->rgbRecv;
				hbr = CreateSolidBrush(colour);
			}
			hdc = BeginPaint(hwnd, &ps);
			FillRect(hdc, &rc, hbr);
			FrameRect(hdc, &rc, frameBrush);
			if (hbr != (HBRUSH)(COLOR_3DFACE + 1))
				DeleteObject(hbr);
			EndPaint(hwnd, &ps);
			return 0;
		}

		hrgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);

		hdc = BeginPaint(hwnd, &ps);
		SelectClipRgn(hdc, hrgn);

		RECT rc2 = rc;
		float sliceWidth = (float)((float)(rc.right - rc.left) / (float)dat->chunkCount);
		float dx = (float)rc2.left;
		for (uint indx = 0; indx < dat->chunkCount; indx++) {
			HBRUSH hbr;
			COLORREF colour = 0;
			if (dat->inSend && indx == dat->chunkIndx)
				colour = dat->rgbToSend;
			else
				switch (dat->chunkAck[indx]) {
				case CHUNK_UNSENT:
					colour = dat->rgbUnSent;
					break;
				case CHUNK_SENT:
					colour = dat->rgbSent;
					break;
				case CHUNK_ACK:
					colour = dat->rgbRecv;
					break;
				}
			hbr = CreateSolidBrush(colour);
			rc2.left = (int)dx;
			rc2.right = (int)(dx + sliceWidth);
			FillRect(hdc, &rc2, hbr);
			FrameRect(hdc, &rc2, frameBrush);
			DeleteObject(hbr);
			dx += sliceWidth - 1;
		}
		if (rc2.right < rc.right) {
			rc2.left = rc2.right;
			rc2.right = rc.right;
			FillRect(hdc, &rc2, (HBRUSH)(COLOR_3DFACE + 1));
		}
		EndPaint(hwnd, &ps);

		DeleteObject(hrgn);
		return 0;
	}
	}
	return mir_callNextSubclass(hwnd, ProgressWndProc, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	FILEECHO *dat = (FILEECHO*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndStatus = nullptr;
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);

		dat = (FILEECHO*)lParam;
		dat->hDlg = hDlg;
		dat->updateTitle();

		hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, L"", hDlg, IDC_STATUS);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)dat);
		WindowList_Add(hFileList, hDlg, dat->hContact);
		Window_SetIcon_IcoLib(hDlg, iconList[ICON_MAIN].hIcolib);
		SendDlgItemMessage(hDlg, IDC_STOP, BUTTONADDTOOLTIP, (WPARAM)Translate(hint_controls[ICON_STOP]), 0);

		mir_subclassWindow(GetDlgItem(hDlg, IDC_PROGRESS), ProgressWndProc);

		SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hDlg, IDC_PLAY, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[ICON_PLAY]);
		SendDlgItemMessage(hDlg, IDC_STOP, BUTTONSETASFLATBTN, 0, 0);
		SendDlgItemMessage(hDlg, IDC_STOP, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[ICON_STOP]);
		dat->setState(STATE_IDLE);

		if (dat->inSend)
			PostMessage(hDlg, WM_COMMAND, IDC_BROWSE, NULL);
		return FALSE;

	case WM_FE_MESSAGE:
		dat->perform((char *)lParam);
		delete (char *)lParam;
		return TRUE;

	case WM_FE_SKINCHANGE:
		Window_SetIcon_IcoLib(hDlg, iconList[ICON_MAIN].hIcolib);
		dat->setState(dat->iState);
		SendDlgItemMessage(hDlg, IDC_STOP, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcons[ICON_STOP]);

		break;

	case WM_FE_STATUSCHANGE:
	{
		char *szProto = Proto_GetBaseAccountName(dat->hContact);
		if (szProto) {
			int dwStatus = db_get_w(dat->hContact, szProto, "Status", ID_STATUS_OFFLINE);
			if (dat->inSend && dwStatus != dat->contactStatus) {
				if (dat->contactStatus == ID_STATUS_OFFLINE) {
					dat->chunkIndx = dat->chunkCount;
				}
				else
					if (dwStatus == ID_STATUS_OFFLINE) {
						if (dat->iState & (STATE_OPERATE | STATE_ACKREQ)) {
							wchar_t *msg = TranslateT("File transfer is paused because of dropped connection");
							SetDlgItemText(hDlg, IDC_STATUS, msg);
							MakePopupMsg(dat->hDlg, dat->hContact, msg);
							dat->setState(STATE_PAUSED);
							KillTimer(hDlg, TIMER_SEND);
						}
					}
			}
			dat->contactStatus = dwStatus;
		}
	}
	return TRUE;

	case WM_DESTROY:
		WindowList_Remove(hFileList, hDlg);
		DestroyWindow(hwndStatus);
		delete dat;
		return TRUE;

	case WM_TIMER:
		if (dat->inSend)
			dat->onSendTimer();
		else
			dat->onRecvTimer();
		break;

	case WM_COMMAND:
		switch (wParam) {
		case IDC_PLAY:
		{
			if (dat->iState & (STATE_IDLE | STATE_FINISHED | STATE_CANCELLED | STATE_PRERECV)) {
				int len = GetWindowTextLength(GetDlgItem(hDlg, IDC_FILENAME)) + 1;
				free(dat->filename);
				dat->filename = (char*)malloc(len);
				GetDlgItemTextA(hDlg, IDC_FILENAME, dat->filename, len);
				if (dat->inSend)
					// Send offer to remote side
				{
					dat->sendReq();
				}
				else
					// Send the accept and starting to receive
				{
					char buff[MAX_PATH];
					char *bufname;

					GetFullPathNameA(dat->filename, sizeof(buff), buff, &bufname);
					*bufname = 0;
					CreateDirectoryTree(buff);
					if (!dat->createTransfer()) {
						SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Failed on file initialization"));
						break;
					}
					dat->sendCmd(0, CMD_ACCEPT, "");
					dat->lastTimestamp = GetTickCount();
					SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Receiving..."));
					dat->setState(STATE_OPERATE);
				}
			}
			else {
				if (dat->inSend) {
					if (dat->iState == STATE_OPERATE) {
						SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Paused..."));
						dat->setState(STATE_PAUSED);
						KillTimer(hDlg, TIMER_SEND);
					}
					else {
						SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Sending..."));
						if (dat->chunkIndx < dat->chunkCount)
							dat->setState(STATE_OPERATE);
						else
							dat->setState(STATE_ACKREQ);
						PostMessage(hDlg, WM_TIMER, 0, 0);
					}
				}
				else {
					SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Synchronizing..."));
					dat->setState(STATE_ACKREQ);
					PostMessage(hDlg, WM_TIMER, 0, 0);
				}
				break;
			}
		}
		break;

		case IDC_BROWSE:
		{
			wchar_t str[MAX_PATH]; *str = 0;
			GetDlgItemText(hDlg, IDC_FILENAME, str, _countof(str));

			OPENFILENAME ofn = {};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = str;
			ofn.Flags = dat->inSend ? OFN_FILEMUSTEXIST : 0;
			ofn.lpstrTitle = dat->inSend ? TranslateT("Select a file") : TranslateT("Save as");
			ofn.nMaxFile = _countof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			if (!GetOpenFileName(&ofn))
				break;
			if (!dat->inSend && dat->iState == STATE_FINISHED)
				break;

			SetDlgItemText(hDlg, IDC_FILENAME, str);

			int size = RetrieveFileSize(str);
			if (size != -1)
				mir_snwprintf(str, TranslateT("Size: %d bytes"), size);
			else
				mir_wstrncpy(str, TranslateT("Can't get a file size"), _countof(str));
			SetDlgItemText(hDlg, IDC_FILESIZE, str);
		}
		break;

		case IDC_STOP:
		case IDCANCEL:
			if (dat->iState == STATE_PRERECV) {
				SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Canceled by user"));
				dat->sendCmd(0, CMD_CANCEL, "", nullptr);
				dat->setState(STATE_CANCELLED);
			}
			if (dat->chunkCount) {
				if (MessageBox(hDlg, TranslateT("Transfer is in progress. Do you really want to close?"),
					TranslateT(SERVICE_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
					SetDlgItemText(hDlg, IDC_STATUS, TranslateT("Canceled by user"));
					dat->setState(STATE_CANCELLED);
					dat->sendCmd(0, CMD_CANCEL, "", nullptr);
					dat->destroyTransfer();
					if (wParam == IDCANCEL)
						DestroyWindow(hDlg);
				}
			}
			else if (wParam == IDCANCEL)
				DestroyWindow(hDlg);
			break;
		}
		break;
	}

	return FALSE;
}
