#include"main.h"

char *szFEMode[] =
{
	"Recv file",
	"Send file"
};

char* ltoax(char* s, DWORD value)
{
	if(value == 0)
	{
		*s++ = '0';
	}
	uchar data;
	int indx = 8;
	while(indx && !(data = (uchar)(value >> 28) & 0x0F))
	{
		value <<= 4;
		indx--;
	}
	while(indx)
	{
		data = (uchar)(value >> 28) & 0x0F;
		if(data > 9) data += 'A' - 10;
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

	while( *value && (ch = *value - '0') >= 0 )
	{
		if(ch > 9)
		{
			ch -= 'A' - '0';
			if(ch > 5) break;
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

static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = PUGetContact(hWnd);
	HWND hDlg = (HWND)PUGetPluginData(hWnd);
/*
	if(hContact)
	{
		CLISTEVENT *lpcle;
		int indx = 0;
		for(;;)
		{
			if((lpcle = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, hContact, indx)) == NULL)
				break;
			if(mir_tstrcmp(lpcle->pszService, SERVICE_NAME "/FERecvFile") == 0)
			{
				lpcle->lParam = (LPARAM)hWnd;
				break;
			}
			indx++;
		}
	}
*/
	switch(message) {
		case WM_COMMAND:
		{
			PUDeletePopup(hWnd);
			CallService(MS_CLIST_REMOVEEVENT, hContact, 0);

			if(IsWindow(hDlg))
			{
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
//
// Just create simple Popup for specified contact
//
void MakePopupMsg(HWND hDlg, MCONTACT hContact, char *msg)
{
	HWND hFocused = GetForegroundWindow();
	if(hDlg == hFocused || hDlg == GetParent(hFocused)) return;

	//
	//The text for the second line. You could even make something like: char lpzText[128]; mir_tstrcpy(lpzText, "Hello world!"); It's your choice.
	//
	POPUPDATA ppd = { 0 };
	ppd.lchContact = hContact;
	ppd.lchIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALLICON));
	mir_tstrcpy(ppd.lpzContactName, (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, 0));
	mir_tstrcpy(ppd.lpzText, msg);
	ppd.colorBack = GetSysColor(COLOR_INFOBK);
	ppd.colorText = GetSysColor(COLOR_INFOTEXT);
	ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
	ppd.PluginData = (void*)hDlg;
	ppd.iSeconds = -1;
	PUAddPopup(&ppd);
}
//
// Get ID of string message
//
int getMsgId(char *msg)
{
	for(int indx = 0; indx < CMD_COUNT; indx++)
	{
		if(*msg == cCmdList[indx]) return indx;
	}
	return -1;
};

int RetrieveFileSize(char *filename)
{
	int handle = open(filename, O_RDONLY|O_BINARY,0);
	if(handle != -1)
	{
		int size = filelength(handle);
		close(handle);
		return size;
	}
	return handle;
}

FILEECHO::FILEECHO(MCONTACT Contact)
{
	hContact = Contact;
	dwSendInterval = db_get_dw(NULL, SERVICE_NAME, "SendDelay", 6000);
	//dwChunkSize = db_get_dw(NULL, SERVICE_NAME, "ChunkSize", 5000);

	chunkMaxLen = db_get_dw(NULL, SERVICE_NAME, "ChunkSize", 5000);
	chunkCount = 0;
	filename = NULL;

	rgbRecv = db_get_dw(NULL, SERVICE_NAME, "colorRecv", RGB(64,255,64));
	rgbSent = db_get_dw(NULL, SERVICE_NAME, "colorSent", RGB(255,255,64));
	rgbUnSent = db_get_dw(NULL, SERVICE_NAME, "colorUnsent", RGB(128,128,128));
	rgbToSend = db_get_dw(NULL, SERVICE_NAME, "colorTosend", RGB(192,192,192));
	asBinary = db_get_dw(NULL, SERVICE_NAME, "base64", 1) == 0;
}

uint controlEnabled[][2] =
{
 	IDC_PLAY,
		STATE_OPERATE|STATE_PAUSED|STATE_PRERECV|STATE_ACKREQ|STATE_IDLE,
	IDC_STOP,
		STATE_OPERATE|STATE_PAUSED|STATE_PRERECV|STATE_REQSENT|STATE_ACKREQ,
//	IDC_FILENAME,
//		STATE_IDLE|STATE_PRERECV|STATE_FINISHED|STATE_CANCELLED,
//	IDC_BROWSE,
//		STATE_IDLE|STATE_PRERECV|STATE_FINISHED|STATE_CANCELLED,
};
/*
char *stateMsg[][2] =
{
	(char*)STATE_IDLE,"Idle",
	(char*)STATE_REQSENT,"ReqSent",
	(char*)STATE_PRERECV,"PreRecv",
	(char*)STATE_OPERATE,"Operate",
	(char*)STATE_ACKREQ,"AckReq",
	(char*)STATE_CANCELLED,"Cancelled",
	(char*)STATE_FINISHED,"Finished",
	(char*)STATE_PAUSED,"Paused"
};
*/

char *hint_controls[4] = {
	"Perform",
	"Pause",
	"Revive a transfer",
	"Stop"
};

void FILEECHO::setState(DWORD state)
{
	iState = state;
	int indx;

	for(indx = 0; indx < SIZEOF(controlEnabled); indx++)
	{
		EnableWindow(GetDlgItem(hDlg, controlEnabled[indx][0]), (iState & controlEnabled[indx][1]) != 0);
	}

	if(!inSend) // recv
	{
		int kind;
		SendDlgItemMessage(hDlg, IDC_FILENAME, EM_SETREADONLY, (state != STATE_PRERECV), 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BROWSE), (iState & (STATE_PRERECV|STATE_FINISHED)));
		//SendDlgItemMessage(hDlg, IDC_FILENAME, EM_SETREADONLY, (iState & STATE_PRERECV) == 0, 0);
		//EnableWindow(GetDlgItem(hDlg, IDC_FILENAME), (iState == STATE_PRERECV));
		//EnableWindow(GetDlgItem(hDlg, IDC_FILENAME), (iState & STATE_IDLE|STATE_PRERECV|STATE_FINISHED|STATE_CANCELLED) != 0);
		if(state & (STATE_IDLE|STATE_FINISHED|STATE_CANCELLED|STATE_PRERECV))
			kind = ICON_PLAY;
		else
			kind = ICON_REFRESH;
		SendDlgItemMessage(hDlg, IDC_PLAY,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcons[kind]);
		SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONADDTOOLTIP,(WPARAM)Translate(hint_controls[kind]),0);
	}
	else
	{
		SendDlgItemMessage(hDlg, IDC_FILENAME, EM_SETREADONLY, (iState & (STATE_IDLE|STATE_FINISHED|STATE_CANCELLED)) == 0, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BROWSE), (iState & (STATE_IDLE|STATE_FINISHED|STATE_CANCELLED)) != 0);
		//EnableWindow(GetDlgItem(hDlg, IDC_FILENAME), (iState & STATE_IDLE|STATE_PRERECV|STATE_FINISHED|STATE_CANCELLED) != 0);
		switch(state)
		{
			case STATE_FINISHED:
			case STATE_CANCELLED:
			case STATE_IDLE:
			case STATE_PAUSED:
				EnableWindow(GetDlgItem(hDlg, IDC_PLAY), TRUE);
				SendDlgItemMessage(hDlg, IDC_PLAY, BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcons[ICON_PLAY]);
				SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONADDTOOLTIP,(WPARAM)Translate(hint_controls[ICON_PLAY]),0);
				break;
			case STATE_OPERATE:
				SendDlgItemMessage(hDlg, IDC_PLAY, BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcons[ICON_PAUSE]);
				SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONADDTOOLTIP,(WPARAM)Translate(hint_controls[ICON_PAUSE]),0);
				break;
		}
	}
	updateProgress();
}

void FILEECHO::updateTitle()
{
	char newtitle[256], *contactName;

	contactName=(char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,0);
	if(iState == STATE_OPERATE && chunkCount != 0)
		mir_snprintf(newtitle, sizeof(newtitle), "%d%% - %s: %s", chunkSent * 100 / chunkCount, Translate(szFEMode[inSend]), contactName);
	else
		mir_snprintf(newtitle, sizeof(newtitle), "%s: %s", Translate(szFEMode[inSend]), contactName);
	SetWindowText(hDlg, newtitle);
}

void BuildFreqTable(uchar *data, uint len, uint *freqTable)
{
	memset(freqTable, 0, (256 * sizeof(uint)));
	for(uint indx = 0; indx < len; indx++)
		freqTable[data[indx]]++;
}

int FILEECHO::createTransfer()
{
	uint LastError;
	hFile = INVALID_HANDLE_VALUE;
	hMapping = NULL;
	lpData = NULL;
#ifdef DEBUG
	overhead = 0;
#endif
	BYTE bAuto = db_get_b(NULL, "SRFile", "AutoAccept", 0);

	hFile = CreateFile(filename, inSend ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE), inSend ? FILE_SHARE_READ : 0, NULL, inSend ? OPEN_EXISTING : (bAuto ? CREATE_ALWAYS : CREATE_NEW), FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE && !inSend && GetLastError() == ERROR_FILE_EXISTS)
	{
		if(MessageBox(hDlg, Translate("File already exists. Overwrite?"),
			Translate(SERVICE_TITLE),
			MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return 0;
		hFile = CreateFile(filename, GENERIC_READ|GENERIC_WRITE, 0,
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	}
	if(hFile == INVALID_HANDLE_VALUE) goto createTransfer_FAILED;
	if(!inSend)
	{
		SetFilePointer(hFile, fileSize, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
	}
	else
		fileSize = GetFileSize(hFile, NULL);
	hMapping = CreateFileMapping(hFile, NULL, inSend?PAGE_READONLY:PAGE_READWRITE,
		0, fileSize, NULL);
	LastError = GetLastError();
	if(hMapping == NULL) goto createTransfer_FAILED;
	lpData = (uchar*)MapViewOfFile(hMapping, inSend?FILE_MAP_READ:FILE_MAP_WRITE, 0,0,0);
	LastError = GetLastError();
	if(lpData == NULL) goto createTransfer_FAILED;

	if (inSend)
	//
	// frequency analysis of source file
	// and building the table of offsets
	//
	{
		if(asBinary)
		{
			uint freq_table[256] = { 0 };
			codeSymb = 1;
			//
			// searching for symbol with lowest frequency: "codeSymb"
			//
			BuildFreqTable(lpData, fileSize, freq_table);
			for (uint i = codeSymb + 1; i < 256; ++i)
			{
				if (freq_table[codeSymb] > freq_table[i])
				codeSymb = i;
			}
			//DEBUG
			//codeSymb = ':';

			//
			// calculating chunks sizes
			// build table of chunks offsets: chunkPos
			//
			uint chunk_count_limit = 2 * fileSize / chunkMaxLen + 2;
			chunkPos = (uint*)calloc(chunk_count_limit, sizeof(uint));

			uchar *data = lpData;
			uint chunk_size = 0;
			uint out_size = 0;
			uint indx = 0;
			uint chunk_offset = 0;

			for (uint len = fileSize; len; --len)
			{
				if (*data == 0 || *data == codeSymb)
					out_size += 2;
				else
					++out_size;

				data++; chunk_size++;
				if (out_size >= chunkMaxLen-1)
				{
					chunkPos[indx] = chunk_offset;
					chunk_offset += chunk_size;
					chunk_size = 0; out_size = 0;
					++indx;
				}
			}
			chunkPos[indx++] = chunk_offset; chunkCount = indx;
			chunkPos = (uint*)realloc(chunkPos, sizeof(uint)*(chunkCount+1));
			chunkPos[indx] = chunk_offset + chunk_size;
		}
		else
		{
			int EncodedMaxLen = Netlib_GetBase64EncodedBufferSize(Netlib_GetBase64DecodedBufferSize(chunkMaxLen));
			int DecodedMaxLen = Netlib_GetBase64DecodedBufferSize(EncodedMaxLen);

			codeSymb = '-';
			chunkCount = (fileSize + DecodedMaxLen - 1) / DecodedMaxLen;
			chunkPos = (uint*)calloc(chunkCount + 1, sizeof(uint));

			uint i = 0;
			for (uint chunk_offset = 0; i < chunkCount; ++i, chunk_offset += DecodedMaxLen)
			{
				chunkPos[i] = chunk_offset;
			}

			chunkPos[i] = chunkPos[i - 1] + (fileSize % DecodedMaxLen);
		}
	}
	else
		chunkCount = chunkCountx;

	chunkAck = (uchar*)calloc(chunkCount, sizeof(uchar));
	chunkIndx = 0; chunkSent = 0;

	return 1;
createTransfer_FAILED:
	if(lpData != NULL) UnmapViewOfFile(lpData);
	if(hMapping != NULL) CloseHandle(hMapping);
	if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	return 0;
}

void FILEECHO::destroyTransfer()
{
	if(chunkCount)
	{
		chunkCount = 0;
		if(inSend)
			free(chunkPos);
		free(chunkAck);
		if(lpData != NULL) UnmapViewOfFile(lpData);
		if(hMapping != NULL) CloseHandle(hMapping);
		if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
	}
	//setState(STATE_IDLE);
	return;
}

void FILEECHO::sendReq()
{
	char sendbuf[MAX_PATH];

	if(!createTransfer())
	{
		SetDlgItemText(hDlg, IDC_FILESIZE, Translate("Couldn't open a file"));
		return;
	}
	///!!!!!!!
	char *p = filename + strlen(filename);
	while(p != filename && *p != '\\')
		p--;
	if(*p == '\\')
		strcpy(filename,p+1);

	mir_snprintf(sendbuf, sizeof(sendbuf), Translate("Size: %d bytes"), fileSize);
	SetDlgItemText(hDlg, IDC_FILESIZE, sendbuf);
	mir_snprintf(sendbuf, sizeof(sendbuf), "?%c%c%d:%d \n" NOPLUGIN_MESSAGE, asBinary+'0', codeSymb, chunkCount, fileSize);
	sendCmd(0, CMD_REQ, sendbuf, filename);

	SetDlgItemText(hDlg, IDC_STATUS, Translate("Request sent. Awaiting of acceptance.."));
	setState(STATE_REQSENT);
}

void FILEECHO::incomeRequest(char *param)
{
	// param: filename?cCOUNT:SIZE
	char buf[MAX_PATH];
	// param == &filename
	char *p = strchr(param, '?');
	if(p == NULL) return; *p++ = 0;
	CallService(MS_FILE_GETRECEIVEDFILESFOLDER, hContact, (LPARAM)buf);
	strncat(buf, param, sizeof(buf));
	free(filename);
	filename = strdup(buf);
	// p == &c
	if(*p == 0) return; asBinary = (*p++) != '0';
	if(*p == 0) return; codeSymb = *p++;
	// p == &COUNT
	if(*p == 0) return; param = strchr(p, ':');
	// param == &SIZE
	if(param == NULL) return; *param++ = 0;
	if(*param == 0) return;
	chunkCountx = atoi(p);
	fileSize = atoi(param);

	mir_snprintf(buf, sizeof(buf), Translate("Size: %d bytes"), fileSize);
	SetDlgItemText(hDlg, IDC_FILENAME, filename);
	SetDlgItemText(hDlg, IDC_FILESIZE, buf);

	setState(STATE_PRERECV);
	inSend = FALSE;

	SkinPlaySound("RecvFile");
	int AutoMin = db_get_b(NULL,"SRFile","AutoMin",0);
	if(db_get_b(NULL,"SRFile","AutoAccept",0) && !db_get_b(hContact,"CList","NotOnList",0))
	{
		PostMessage(hDlg, WM_COMMAND, IDC_PLAY, 0);
		if(AutoMin)
			ShowWindow(hDlg, SW_SHOWMINIMIZED);
//		ShowWindow(hDlg, SW_MINIMIZE);
//		UpdateWindow(hDlg);
	}
//	else
	if(!IsWindowVisible(hDlg) && !AutoMin)
	{
		CLISTEVENT cle;
		memset(&cle, 0, sizeof(cle));
		cle.cbSize = sizeof(cle);
		cle.hContact = hContact;
		cle.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SMALLICON));
		cle.flags = CLEF_URGENT;
		cle.hDbEvent = 0;
		cle.pszService = SERVICE_NAME "/FERecvFile";
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);

		MakePopupMsg(hDlg, hContact, "Incoming file...");
	}
}

void FILEECHO::cmdACCEPT()
{
	if(chunkCount == 0) return;
	setState(STATE_OPERATE);
	SetDlgItemText(hDlg, IDC_STATUS, Translate("Sending..."));
	lastTimestamp = GetTickCount();
	//PostMessage(hDlg, WM_TIMER, 0,0);
	//onSendTimer();
	SetTimer(hDlg, TIMER_SEND, dwSendInterval, 0);
}

void FILEECHO::updateProgress()
{
	InvalidateRect(GetDlgItem(hDlg, IDC_PROGRESS), NULL, TRUE);
	updateTitle();
}
//
// called in receive mode
// used to transfer acknowledge
//
void FILEECHO::onRecvTimer()
{
	if(chunkCount == 0) return;
	char *buffer = (char*)malloc(1024);
	char *p = buffer;
	uchar prev_value;
	uint indx, jndx;
	
	KillTimer(hDlg, TIMER_SEND);
	//
	// Build response about successfully received chunks
	//
	indx = jndx = 0; prev_value = chunkAck[jndx];
	while(jndx < chunkCount)
	{
		if(chunkAck[jndx] != prev_value)
		{
			if(prev_value != CHUNK_ACK)
			{
				p = ltoax(p, indx);
				if(indx != jndx-1)
				{
					*p++ = '-';
					p = ltoax(p, jndx-1);
				}
				*p++ = ',';
			}
			indx = jndx;
			prev_value = chunkAck[jndx];
		}
		jndx++;
	}
	if(prev_value != CHUNK_ACK)
	{
		p = ltoax(p, indx);
		if(indx != jndx-1)
		{
			*p++ = '-';
			p = ltoax(p, jndx-1);
		}
	}
	*p = 0;
	if(*buffer == 0)
	{
		char *msg = Translate("Received successfully");
		SetDlgItemText(hDlg, IDC_STATUS, msg);
		MakePopupMsg(hDlg, hContact, msg);
		setState(STATE_FINISHED);
		if(db_get_b(NULL,"SRFile","AutoClose",0))
		{
			PostMessage(hDlg, WM_CLOSE, 0,0);
			CallService(MS_CLIST_REMOVEEVENT, hContact, 0);
		}
		SkinPlaySound("FileDone");
		destroyTransfer();
		buffer[0] = 'x'; buffer[1] = 0;
	}
	sendCmd(0, CMD_DACK, buffer);
	free(buffer);
	//if(iState != STATE_FINISHED) SetTimer(hDlg, TIMER_SEND, lastDelay*2, 0);
}
//
// called in sending mode
// used to data transfer and
// sending of scheduled commands
//
void FILEECHO::onSendTimer()
{
	if(chunkCount == 0) return;
	//
	// perform request of acknowledge, if scheduled
	//
	KillTimer(hDlg, TIMER_SEND);
	//
	// Search for next unsent chunk
	//
	while(chunkIndx < chunkCount && chunkAck[chunkIndx] != CHUNK_UNSENT) chunkIndx++;
	if(iState == STATE_ACKREQ || chunkIndx == chunkCount)
	{
		SetDlgItemText(hDlg, IDC_STATUS, Translate("Requesting of missing chunks"));
		setState(STATE_OPERATE);
		sendCmd(0, CMD_END, "", NULL);
		chunkIndx = chunkCount+1;
		return;
	}
	if(chunkIndx > chunkCount) return;

	uchar *buffer = (uchar*)malloc(chunkMaxLen*2);
	uchar *p = buffer;
	uchar *data = lpData + chunkPos[chunkIndx];
	uchar *data_end = lpData + chunkPos[chunkIndx+1];
	ulong chksum = memcrc32(data, data_end - data, INITCRC);

	if(asBinary)
	{
		//
		// Encoding data to transfer with symb. filtering
		//
		while(data < data_end)
		{
			uchar ch = *data++;
			if(ch == 0)
			{
				*p++ = codeSymb; *p++ = '0';
			}
			else if (ch == codeSymb)
			{
				*p++ = codeSymb; *p++ = '+';
			}
			else
				*p++ = ch;
		}
		*p = 0;
	}
	else
	{
		ptrA enc( mir_base64_encode(data, data_end - data));
		strncpy((char*)buffer, enc, chunkMaxLen*2);
	}

	char prefix[128];
	mir_snprintf(prefix, sizeof(prefix), "%X,%X,%X>", chunkIndx+1, chunkPos[chunkIndx], chksum);
#ifdef DEBUG
	overhead += mir_tstrlen((char*)buffer);
#endif
	sendCmd(0, CMD_DATA, (char*)buffer, (char*)prefix);
	chunkAck[chunkIndx] = CHUNK_SENT;

	free(buffer);

	chunkIndx++; chunkSent++;

	if(chunkIndx == chunkCount)
		setState(STATE_ACKREQ);
	else
	{
		SetDlgItemText(hDlg, IDC_STATUS, Translate("Sending..."));
		updateProgress();
	}
	SetTimer(hDlg, TIMER_SEND, dwSendInterval, 0);
}
void FILEECHO::cmdDATA(char *param)
{
	if(chunkCount == 0) return;
	chunkIndx = atolx(param); param++;
	if(chunkIndx-- == 0) return;
	uint filepos = atolx(param); param++;
	if(filepos >= fileSize) return;

	ulong chksum_local;
	ulong chksum_remote = atolx(param); param++;

	KillTimer(hDlg, TIMER_SEND);

	//
	// Decoding of incoming data
	//
	uchar *data = lpData + filepos;
	uchar *data_end = lpData + fileSize;
	if(asBinary)
	{
		uchar ch;
		while(ch = *param++)
		{
			if(ch == codeSymb)
			{
				if((ch = *param++) == 0) goto cmdDATA_corrupted;
				switch(ch)
				{
					case '+':
						ch = codeSymb;
						break;
					case '0':
						ch = 0;
						break;
					default:
						goto cmdDATA_corrupted;
				}
			}
			if(data > data_end) goto cmdDATA_corrupted;
			*data++ = ch;
		}
	}
	else
	{
		unsigned bufLen;
		mir_ptr<BYTE> buf((BYTE*)mir_base64_decode(param, &bufLen));
		memcpy(data, buf, min(bufLen, unsigned(data_end - data)));
		data += bufLen;		
	}
	//
	// let's check it up
	//
	chksum_local = memcrc32(lpData + filepos, data - (lpData + filepos), INITCRC);
	if(chksum_local == chksum_remote)
	{
		if(chunkAck[chunkIndx] != CHUNK_ACK) chunkSent++;
		chunkAck[chunkIndx] = CHUNK_ACK;
		//chunkPos[chunkIndx++] = filepos;
	}
	SetDlgItemText(hDlg, IDC_STATUS, Translate("Receiving..."));
	updateProgress();
cmdDATA_corrupted:
	//SetTimer(hDlg, TIMER_SEND, lastDelay*2, 0);
	;
}
void FILEECHO::cmdEND()
{
	SetTimer(hDlg, TIMER_SEND, dwSendInterval, 0);
}
void FILEECHO::cmdDACK(char *param)
{
	uint indx, jndx;

	if(chunkCount == 0) return;
	memset(chunkAck, CHUNK_ACK, sizeof(uchar)*chunkCount);
	if(*param == 'x')
	//
	// All chunks has been received successfully
	//
	{
		char *msg = Translate("Sent successfully");
		SetDlgItemText(hDlg, IDC_STATUS, msg);

		SkinPlaySound("FileDone");
		destroyTransfer();
		MakePopupMsg(hDlg, hContact, msg);
		setState(STATE_FINISHED);
		return;
	}
	chunkSent = chunkCount;
	//
	// Mark chunks to re-transfer,
	// according received info
	// 
	// format: chunk1, chunk3-chunk10, etc..
	//
	while(*param)
	{
		indx = atolx(param);
		if(*param == '-')
		{
			param++; jndx = atolx(param);
		}
		else
			jndx = indx;
		if(*param == 0 || *param == ',')
		{
			for(uint p = indx; p <= jndx; p++)
			{
				if(p < chunkCount)
				{
					chunkAck[p] = CHUNK_UNSENT;
					chunkSent--;
				}
			}
			if(*param == ',')
				param++;
		}
	}
	updateProgress();

	//
	// retransfer some parts
	//
	chunkIndx = 0;
	SetTimer(hDlg, TIMER_SEND, dwSendInterval, 0);
}

void FILEECHO::perform(char *str)
{
	int msgId = getMsgId(str);
	if(msgId == -1)
	{
		MakePopupMsg(hDlg, hContact, Translate("Unknown command for \"File As Message\" was received"));
		return;
	}
	if(inSend)
		switch(msgId)
		{
			case CMD_REQ:
				if(MessageBox(hDlg, Translate("Incoming file request. Do you want to proceed?"),
					Translate(SERVICE_TITLE), MB_YESNO | MB_ICONWARNING) == IDYES)
				{
					SetDlgItemText(hDlg, IDC_STATUS, "");
					SendMessage(hDlg, WM_COMMAND, IDC_STOP, 0);

					incomeRequest(str+1);
					updateTitle();
					break;
				}
				break;
			case CMD_ACCEPT:
				cmdACCEPT();
				break;
			case CMD_CANCEL:
			{
				if(iState & (STATE_PRERECV|STATE_REQSENT|STATE_OPERATE|STATE_ACKREQ|STATE_PAUSED))
				{
					char *msg = Translate("Canceled by remote user");
					SetDlgItemText(hDlg, IDC_STATUS, msg);
					MakePopupMsg(hDlg, hContact, msg);
					destroyTransfer();
					setState(STATE_CANCELLED);
				}
				break;
			}
			case CMD_DACK:
				cmdDACK(str+1);
				break;
		}
	else
		switch(msgId)
		{
			case CMD_CANCEL:
			{
				if(iState & (STATE_PRERECV|STATE_REQSENT|STATE_OPERATE|STATE_ACKREQ|STATE_PAUSED))
				{
					char *msg = Translate("Canceled by remote user");
					SetDlgItemText(hDlg, IDC_STATUS, msg);
					MakePopupMsg(hDlg, hContact, msg);
					destroyTransfer();
					setState(STATE_CANCELLED);
				}
				break;
			}
			case CMD_REQ:
				if(chunkCount)
				{
					if(MessageBox(hDlg, Translate("New incoming file request. Do you want to proceed?"),
						Translate(SERVICE_TITLE), MB_YESNO | MB_ICONWARNING) != IDYES)
						break;
					//sendCmd(0, CMD_CANCEL, "", NULL);
					destroyTransfer();
				}
				SetDlgItemText(hDlg, IDC_STATUS, "");
				incomeRequest(str+1);
				break;
			case CMD_DATA:
				cmdDATA(str+1);
				break;
			case CMD_END:
				cmdEND();
				break;
		};
};

int FILEECHO::sendCmd(int id, int cmd, char *szParam, char *szPrefix)
{
	char *buf;
	int retval;
	int buflen = (int)_tcslen(szServicePrefix) + (int)_tcslen(szParam) + 2;
	if(szPrefix != NULL)
		buflen += (int)_tcslen(szPrefix);
	
	buf = (char*)malloc(buflen);
	if(szPrefix == NULL)
		mir_snprintf(buf, buflen, "%s%c%s", szServicePrefix, cCmdList[cmd], szParam);
	else
		mir_snprintf(buf, buflen, "%s%c%s%s", szServicePrefix, cCmdList[cmd], szPrefix, szParam);
	retval = CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)buf);
	free(buf);
	updateProgress();
	return retval;	
}


void CreateDirectoryTree(char *szDir)
{
	DWORD dwAttributes;
	char *pszLastBackslash,szTestDir[MAX_PATH];

	mir_tstrncpy(szTestDir,szDir,sizeof(szTestDir));
	if((dwAttributes=GetFileAttributes(szTestDir))!=0xffffffff
	   && dwAttributes&FILE_ATTRIBUTE_DIRECTORY) return;
	pszLastBackslash=strrchr(szTestDir,'\\');
	if(pszLastBackslash==NULL) {GetCurrentDirectory(MAX_PATH,szDir); return;}
	*pszLastBackslash='\0';
	CreateDirectoryTree(szTestDir);
	CreateDirectory(szTestDir,NULL);
}

LRESULT CALLBACK ProgressWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_NCPAINT:
			return 0;
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			RECT rc;
			HRGN hrgn;
			HBRUSH frameBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			struct FILEECHO *dat;

			dat = (struct FILEECHO*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
			//if(dat == NULL)
			//	return CallSubclassed(hwnd, uMsg, wParam, lParam);
			GetClientRect(hwnd, &rc);
			if(dat == NULL || dat->chunkCount == 0)
			{
				COLORREF colour;
				HBRUSH hbr;

				if(dat == NULL || dat->iState != STATE_FINISHED)
				{
					hbr = (HBRUSH)(COLOR_3DFACE+1);
				}
				else
				{
					colour = dat->rgbRecv;
					hbr = CreateSolidBrush(colour);
				}
				hdc=BeginPaint(hwnd,&ps);
					FillRect(hdc, &rc, hbr);
					FrameRect(hdc, &rc, frameBrush);
					if(hbr != (HBRUSH)(COLOR_3DFACE+1))
						DeleteObject(hbr);
				EndPaint(hwnd,&ps);
				return 0;
			}

			hrgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
			
			hdc=BeginPaint(hwnd,&ps);
				SelectClipRgn(hdc, hrgn);
			
				RECT rc2 = rc;
				//uint sliceWidth = (rc.right - rc.left) / dat->chunkCount;
				float sliceWidth = (float)((float)(rc.right - rc.left) / (float)dat->chunkCount);
				float dx = (float)rc2.left;
				for(uint indx = 0; indx < dat->chunkCount; indx++)
				{
					HBRUSH hbr;
					COLORREF colour;
					if(dat->inSend && indx == dat->chunkIndx)
						colour = dat->rgbToSend;
					else
						switch(dat->chunkAck[indx])
						{
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
					/*
					if(indx == 5) colour = RGB(255,64,64);
					else if(indx < 2) colour = RGB(64,255,64);
					else if(indx < 4) colour = RGB(255,255,64);
					else colour = RGB(128,128,128);
					//*/
					if(indx == dat->chunkCount-1)
						rc2.right = rc.right;
					hbr = CreateSolidBrush(colour);
					rc2.left = (int)dx;
					rc2.right = (int)(dx + sliceWidth);
					FillRect(hdc, &rc2, hbr);
					FrameRect(hdc, &rc2, frameBrush);
					DeleteObject(hbr);
					dx += sliceWidth-1;
				}
				if(rc2.right < rc.right)
				{
					rc2.left = rc2.right;
					rc2.right = rc.right;
					FillRect(hdc, &rc2, (HBRUSH)(COLOR_3DFACE+1));
				}
				//FrameRect(hdc, &rc, (HBRUSH)(COLOR_3DLIGHT+1));
				//OffsetRect(&rc, 1,1);
				//FrameRect(hdc, &rc, (HBRUSH)(COLOR_BTNTEXT+1));
				//FrameRect(hdc, &rc, (HBRUSH)(COLOR_BTNTEXT+1));
			EndPaint(hwnd,&ps);

			DeleteObject(hrgn);

			return 0;
		}
	}
	return mir_callNextSubclass(hwnd, ProgressWndProc, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	struct FILEECHO *dat = (struct FILEECHO*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndStatus = NULL;
	switch( uMsg )
	{
		case WM_INITDIALOG:
		{
			dat = (FILEECHO*)lParam;
			dat->hDlg = hDlg;

			dat->updateTitle();

			hwndStatus = CreateStatusWindow(WS_CHILD|WS_VISIBLE, "", hDlg, IDC_STATUS);
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)dat);
			WindowList_Add(hFileList, hDlg, dat->hContact);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcons[ICON_MAIN]);
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcons[ICON_MAIN]);
			SendDlgItemMessage(hDlg, IDC_STOP, BUTTONADDTOOLTIP,(WPARAM)Translate(hint_controls[ICON_STOP]),0);

			//SetDlgItemText(hDlg, IDC_FILENAME, "C:\\!Developer\\!Miranda\\miranda\\bin\\release\\emo\\biggrin.gif");

			mir_subclassWindow(GetDlgItem(hDlg, IDC_PROGRESS), ProgressWndProc);
			
			SendDlgItemMessage(hDlg, IDC_PLAY, BUTTONSETASFLATBTN,0,0);				
			SendDlgItemMessage(hDlg, IDC_PLAY, BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcons[ICON_PLAY]);
			SendDlgItemMessage(hDlg, IDC_STOP, BUTTONSETASFLATBTN,0,0);
			SendDlgItemMessage(hDlg, IDC_STOP, BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcons[ICON_STOP]);
			dat->setState(STATE_IDLE);

			//ShowWindow(hDlg, SW_HIDE);
			//UpdateWindow(hDlg);

			if(dat->inSend)
				PostMessage(hDlg, WM_COMMAND, IDC_BROWSE, NULL);

			return FALSE;
		}
		case WM_FE_MESSAGE:
		{
			dat->perform((char *)lParam);
			delete (char *)lParam;

			return TRUE;
		}
		case WM_FE_SKINCHANGE:
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcons[ICON_MAIN]);
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcons[ICON_MAIN]);
			dat->setState(dat->iState);
			SendDlgItemMessage(hDlg, IDC_STOP, BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcons[ICON_STOP]);

			break;
		case WM_FE_STATUSCHANGE:
		{
			char *szProto = GetContactProto(dat->hContact);
			if (szProto)
			{
				int dwStatus = db_get_w(dat->hContact,szProto,"Status",ID_STATUS_OFFLINE);
				if(dat->inSend && dwStatus != dat->contactStatus)
				{
					if(dat->contactStatus == ID_STATUS_OFFLINE)
					{
						dat->chunkIndx = dat->chunkCount;
					}
					else
						if(dwStatus == ID_STATUS_OFFLINE)
						{
							if(dat->iState & (STATE_OPERATE|STATE_ACKREQ))
							{
								char *msg = Translate("File transfer is paused because of dropped connection");
								SetDlgItemText(hDlg, IDC_STATUS, msg);
								MakePopupMsg(dat->hDlg, dat->hContact, msg);
								dat->setState(STATE_PAUSED);
								KillTimer(hDlg, TIMER_SEND);
							}
						}
				}
				dat->contactStatus = dwStatus;
			}
			return TRUE;
		}
		case WM_DESTROY:
			WindowList_Remove(hFileList, hDlg);
			DestroyWindow(hwndStatus);
			delete dat;
		
			return TRUE;

		case WM_TIMER:
			if(dat->inSend)
				dat->onSendTimer();
			else
				dat->onRecvTimer();
			break;
		case WM_COMMAND:
			switch(wParam)
			{
				case IDC_PLAY:
				{
					if(dat->iState & (STATE_IDLE|STATE_FINISHED|STATE_CANCELLED|STATE_PRERECV))
					{
						int len = GetWindowTextLength(GetDlgItem(hDlg, IDC_FILENAME))+1;
						free(dat->filename);
						dat->filename = (char*)malloc(len);
						GetDlgItemText(hDlg, IDC_FILENAME, dat->filename, len);
						if(dat->inSend)
						// Send offer to remote side
						{							
							dat->sendReq();
						}
						else
						// Send the accept and starting to receive
						{
							char buff[MAX_PATH];
							char *bufname;

							GetFullPathName(dat->filename, sizeof(buff), buff, &bufname);
							*bufname = 0;
							CreateDirectoryTree(buff);
							if(!dat->createTransfer())
							{
								SetDlgItemText(hDlg, IDC_STATUS, Translate("Failed on file initialization"));
								break;
							}
							dat->sendCmd(0, CMD_ACCEPT, "");
							dat->lastTimestamp = GetTickCount();
							SetDlgItemText(hDlg, IDC_STATUS, Translate("Receiving..."));
							dat->setState(STATE_OPERATE);
						}
					}
					else
					{
						if(dat->inSend)
						{
							if(dat->iState == STATE_OPERATE)
							{
								SetDlgItemText(hDlg, IDC_STATUS, Translate("Paused..."));
								dat->setState(STATE_PAUSED);
								KillTimer(hDlg, TIMER_SEND);
							}
							else
							{
								SetDlgItemText(hDlg, IDC_STATUS, Translate("Sending..."));
								if(dat->chunkIndx < dat->chunkCount)
									dat->setState(STATE_OPERATE);
								else
									dat->setState(STATE_ACKREQ);
								PostMessage(hDlg, WM_TIMER, 0,0);
								//dat->onRecvTimer();
								//SetTimer(hDlg, TIMER_SEND, dwSendInterval, NULL);
							}
						}
						else
						{
							SetDlgItemText(hDlg, IDC_STATUS, Translate("Synchronizing..."));
							dat->setState(STATE_ACKREQ);
							PostMessage(hDlg, WM_TIMER, 0,0);
							//dat->onRecvTimer();
							//SetTimer(hDlg, TIMER_SEND, dwSendInterval, 0);
						}
						break;
					}
					break;
				}
				case IDC_BROWSE:
				{
					char str[MAX_PATH];
					OPENFILENAME ofn;

					memset(&ofn, 0, sizeof(ofn));
					*str = 0;
					GetDlgItemText(hDlg, IDC_FILENAME, str, SIZEOF(str));
					//ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hDlg;
					//ofn.lpstrFilter = "*.*";
					ofn.lpstrFile = str;
					ofn.Flags = dat->inSend?OFN_FILEMUSTEXIST:0;
					ofn.lpstrTitle = dat->inSend?Translate("Select a file"):Translate("Save as");
					ofn.nMaxFile = SIZEOF(str);
					ofn.nMaxFileTitle = MAX_PATH;
					if(!GetOpenFileName(&ofn)) break;
					if(!dat->inSend && dat->iState == STATE_FINISHED) break;
					SetDlgItemText(hDlg, IDC_FILENAME, str);

					int size = RetrieveFileSize(str);
					if(size != -1)
						mir_snprintf(str, sizeof(str), Translate("Size: %d bytes"), size);
					else
						mir_snprintf(str, sizeof(str), Translate("Can't get a file size"), size);
					SetDlgItemText(hDlg, IDC_FILESIZE, str);					

					break;
				}

				case IDC_STOP:
				case IDCANCEL:
					if(dat->iState == STATE_PRERECV)
					{
						SetDlgItemText(hDlg, IDC_STATUS, Translate("Canceled by user"));
						dat->sendCmd(0, CMD_CANCEL, "", NULL);
						dat->setState(STATE_CANCELLED);
					}
					if(dat->chunkCount)
					{
						if(MessageBox(hDlg, Translate("Transfer is in progress. Do you really want to close?"),
							Translate(SERVICE_TITLE), MB_ICONWARNING|MB_YESNO|MB_DEFBUTTON2) == IDYES)
						{
							SetDlgItemText(hDlg, IDC_STATUS, Translate("Canceled by user"));
							dat->setState(STATE_CANCELLED);
							dat->sendCmd(0, CMD_CANCEL, "", NULL);
							dat->destroyTransfer();
							if(wParam == IDCANCEL)
								DestroyWindow(hDlg);
						}
					}
					else
						if(wParam == IDCANCEL)
							DestroyWindow(hDlg);
					break;//return TRUE;
			}
			break;
	}

	return FALSE;
}
