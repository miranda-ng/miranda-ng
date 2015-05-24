/*

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

#include "Common.h"

struct FLASHING_SEQUENCE
{
	unsigned int size;
	unsigned int index;
	BYTE frame[MAX_PATH];
};


// Prototypes
FLASHING_SEQUENCE *getCustomSeq(void);
FLASHING_SEQUENCE *getPredefinedSeq(void);
FLASHING_SEQUENCE *getTrillianSeq(void);
void updateTrillianSeq(void);
static void __cdecl TestThread(void *param);
static void PreviewThread(void *);
FLASHING_SEQUENCE str2FS(TCHAR *);
BYTE KbdChar2Byte(char);
void countUnopenEvents(int *, int *, int *, int *);

#define Leds2Flash	((BYTE)(bFlashLed[2] + (bFlashLed[0]<<1) + (bFlashLed[1]<<2)))

// Flashing settings
FLASHING_SEQUENCE *pFS;
BOOL bTemporarilyUseExtern;
extern BYTE bFlashLed[3];
extern BYTE bFlashEffect; extern BYTE bSequenceOrder;
extern WORD wCustomTheme;
extern BYTE bTrillianLedsMsg; extern BYTE bTrillianLedsURL; extern BYTE bTrillianLedsFile; extern BYTE bTrillianLedsOther;
extern BYTE bEmulateKeypresses;

// TestThread/PreviewThread globals
extern int nWaitDelay; extern WORD wStartDelay;
BOOL bTestSemaphore, bPreviewSemaphore, bPreview;

void RestoreLEDState(void)
{
	if (bEmulateKeypresses)
		keypresses_RestoreLEDState();
	else
		ToggleKeyboardLights((BYTE)(LedState(VK_SCROLL) + (LedState(VK_NUMLOCK)<<1) + (LedState(VK_CAPITAL)<<2)));
}

BYTE getBlinkingLeds(void)
{
	if (!pFS->size)
		return (BYTE)(LedState(VK_SCROLL) + (LedState(VK_NUMLOCK)<<1) + (LedState(VK_CAPITAL)<<2));

	pFS->index %= pFS->size;
	if (bFlashEffect == FLASH_TRILLIAN && !bTemporarilyUseExtern && !pFS->index)
		updateTrillianSeq();
	return pFS->frame[pFS->index++];
}

void setFlashingSequence(void)
{
	switch (bFlashEffect) {
	case FLASH_CUSTOM:
		pFS = getCustomSeq();
		break;
	case FLASH_TRILLIAN:
		pFS = getTrillianSeq();
		break;
	default:
		pFS = getPredefinedSeq();
	}
	bTemporarilyUseExtern = FALSE;
}

FLASHING_SEQUENCE *getCustomSeq(void)
{
	ptrT tszSeq( db_get_wsa(NULL, KEYBDMODULE, fmtDBSettingName("custom%d", wCustomTheme)));

	static FLASHING_SEQUENCE Custom = {0};
	Custom = str2FS((tszSeq != 0) ? tszSeq : L"");
	return &Custom;
}


FLASHING_SEQUENCE *getPredefinedSeq(void)
{
	static FLASHING_SEQUENCE Predefined = {0};

	FLASHING_SEQUENCE *pAux;
	FLASHING_SEQUENCE SameTime = {2, 0, {7, 0}};
	FLASHING_SEQUENCE InTurn = {2, 0, {3, 4}};
	FLASHING_SEQUENCE InSeq = {3, 0, {2, 4, 1}};
	FLASHING_SEQUENCE InSeqRev = {3, 0, {1, 4, 2}};
	FLASHING_SEQUENCE InSeqKIT = {4, 0, {2, 4, 1, 4}};


	if (Leds2Flash < 3 || Leds2Flash == 4)
		pAux = &SameTime;
	else
		switch (bFlashEffect) {
		default:
		case FLASH_SAMETIME:
			pAux = &SameTime;
			break;
		case FLASH_INTURN:
			if (Leds2Flash == 3)	// 3 = Num+Scroll
				pAux = &InSeq;
			else
				pAux = &InTurn;
			break;
		case FLASH_INSEQUENCE:
			switch (bSequenceOrder) {
			default:
			case SEQ_LEFT2RIGHT:
				pAux = &InSeq;
				break;
			case SEQ_RIGHT2LEFT:
				pAux = &InSeqRev;
				break;
			case SEQ_LIKEKIT:
				if (Leds2Flash != 7)	// 7 = Num+Caps+Scroll
					pAux = &InSeq;
				else
					pAux = &InSeqKIT;
				break;
			}
			break;
		}

	Predefined.size = Predefined.index = 0;
	for (pAux->index=0; pAux->index < pAux->size; pAux->index++)
		if (!pAux->frame[pAux->index] || (pAux->frame[pAux->index] & Leds2Flash)) {
			Predefined.size++;
			Predefined.frame[Predefined.size - 1] = pAux->frame[pAux->index] & Leds2Flash;
		}

	return &Predefined;
}


FLASHING_SEQUENCE *getTrillianSeq(void)
{
	static FLASHING_SEQUENCE Trillian = {2, 0, {0, 0}};

	Trillian.size = 2;
	Trillian.index = 0;

	return &Trillian;
}


void updateTrillianSeq(void)
{
	int i, msgCount=0, fileCount=0, urlCount=0, otherCount=0;

	pFS->size = 2;
	countUnopenEvents(&msgCount, &fileCount, &urlCount, &otherCount);

	if ((bTrillianLedsMsg & Leds2Flash) && (pFS->size + 2 * msgCount) <= MAX_PATH)
		for (i=0; i < msgCount; i++) {
			pFS->frame[pFS->size++] = bTrillianLedsMsg & Leds2Flash;
			pFS->frame[pFS->size++] = 0;
		}

	if ((bTrillianLedsFile & Leds2Flash) && (pFS->size + 2 * fileCount) <= MAX_PATH)
		for (i=0; i < fileCount; i++) {
			pFS->frame[pFS->size++] = bTrillianLedsFile & Leds2Flash;
			pFS->frame[pFS->size++] = 0;
		}

	if ((bTrillianLedsURL & Leds2Flash) && (pFS->size + 2 * urlCount) <= MAX_PATH)
		for (i=0; i < urlCount; i++) {
			pFS->frame[pFS->size++] = bTrillianLedsURL & Leds2Flash;
			pFS->frame[pFS->size++] = 0;
		}

	if ((bTrillianLedsOther & Leds2Flash) && (pFS->size + 2 * otherCount) <= MAX_PATH)
		for (i=0; i < otherCount; i++) {
			pFS->frame[pFS->size++] = bTrillianLedsOther & Leds2Flash;
			pFS->frame[pFS->size++] = 0;
		}
}

void useExternSequence(TCHAR *extStr)
{
	static FLASHING_SEQUENCE Extern = {0};

	TCHAR externStr[MAX_PATH+1];
	mir_wstrcpy(externStr, extStr);

	Extern = str2FS(normalizeCustomString(externStr));

	pFS = &Extern;
	bTemporarilyUseExtern = TRUE;
}


TCHAR *normalizeCustomString(TCHAR *customStr)
{
	int len=0, status=0;
	BOOL used[4];
	TCHAR strAux[MAX_PATH+1], *str;

	for (mir_wstrcpy(str=strAux, customStr); *str; str++) {
		switch (*str) {
		case _T('['):
			if (status == 0) {
				status = 1;
				customStr[len++] = *str;
				used[0] = used [1] = used[2] = used[3] = FALSE;
			}
			break;
		case _T(']'):
			if (status == 1) {
				status = 0;
				customStr[len++] = *str;
			}
			break;
		case _T('0'):
		case _T('1'):
		case _T('2'):
		case _T('3'):
			if (status == 0)
				customStr[len++] = *str;
			else if (!used[*str - _T('0')]) {
				customStr[len++] = *str;
				used[*str - _T('0')] = TRUE;
			}
			break;
		}
	}
	if (status == 1)
		customStr[len++] = _T(']');
	customStr[len] = _T('\0');

	return customStr;
}

TCHAR* getCurrentSequenceString(void)
{
	static TCHAR CurrentSeqString[MAX_PATH+1];
	TCHAR *str = CurrentSeqString;

	for (unsigned i=0; i < pFS->size; i++) {
		switch (pFS->frame[i]) {
		case 0:
			*(str++) = _T('0');
			break;
		case 1:
			*(str++) = _T('3');
			break;
		case 2:
			*(str++) = _T('1');
			break;
		case 3:
			*(str++) = _T('[');
			*(str++) = _T('1');
			*(str++) = _T('3');
			*(str++) = _T(']');
			break;
		case 4:
			*(str++) = _T('2');
			break;
		case 5:
			*(str++) = _T('[');
			*(str++) = _T('2');
			*(str++) = _T('3');
			*(str++) = _T(']');
			break;
		case 6:
			*(str++) = _T('[');
			*(str++) = _T('1');
			*(str++) = _T('2');
			*(str++) = _T(']');
			break;
		case 7:
			*(str++) = _T('[');
			*(str++) = _T('1');
			*(str++) = _T('2');
			*(str++) = _T('3');
			*(str++) = _T(']');
		}
	}
	*str = _T('\0');

	return CurrentSeqString;
}

void testSequence(TCHAR *testStr)
{
	if (bTestSemaphore) // we try to avoid concurrent test threads
		return;

	bTestSemaphore = TRUE;

	static FLASHING_SEQUENCE Test = {0};
	Test = str2FS(testStr);
	
	mir_forkthread(TestThread, (void*)&Test);
}


static void __cdecl TestThread(void *param)
{
	FLASHING_SEQUENCE *pTest = (FLASHING_SEQUENCE *)param;
	unsigned int testNum = (unsigned int)db_get_b(NULL, KEYBDMODULE, "testnum", DEF_SETTING_TESTNUM);
	unsigned int testSecs = (unsigned int)db_get_b(NULL, KEYBDMODULE, "testsecs", DEF_SETTING_TESTSECS);

	for (unsigned i=0, dwEndTest=GetTickCount()+testSecs*1000; i < testNum || GetTickCount() < dwEndTest; i++)
		for (pTest->index=0; pTest->index < pTest->size; pTest->index++) {
			ToggleKeyboardLights(pTest->frame[pTest->index]);
			Sleep(nWaitDelay);
		}

	RestoreLEDState();

	bTestSemaphore = FALSE;	
}


void previewFlashing(BOOL buttonState)
{
	bPreview = buttonState;
	if (!bPreview || bPreviewSemaphore) // turn off flashing or already running
		return;

	bPreviewSemaphore = TRUE;
	mir_forkthread(PreviewThread, 0);
}


static void PreviewThread(void *dummy)
{
	if (wStartDelay > 0)
		Sleep(wStartDelay * 1000);

	BYTE unchangedLeds = (BYTE)(LedState(VK_SCROLL) * !bFlashLed[2] + ((LedState(VK_NUMLOCK) * !bFlashLed[0])<<1) + ((LedState(VK_CAPITAL) * !bFlashLed[1])<<2));

	while (bPreview)
		for (unsigned i=0; bPreview && i < pFS->size; i++) {
			ToggleKeyboardLights((BYTE)(pFS->frame[i%pFS->size]|unchangedLeds));
			Sleep(nWaitDelay);				
		}

	RestoreLEDState();

	bPreviewSemaphore = FALSE;	
}


FLASHING_SEQUENCE str2FS(TCHAR *str)
{
	FLASHING_SEQUENCE Temp = {0};

	for (Temp.size=Temp.index=0; *str; str++) {
		Temp.size++;
		if (*str == _T('[')) {
			Temp.frame[Temp.size - 1] = 0;
			for (str++; *str && *str != _T(']'); str++)
				Temp.frame[Temp.size - 1] += KbdChar2Byte(*str) & Leds2Flash;
			if (!*str) break;
		}
		else Temp.frame[Temp.size - 1] = KbdChar2Byte(*str) & Leds2Flash;
	}

	return Temp;
}


BYTE KbdChar2Byte(char kbdChar)
{
	switch (kbdChar) {
	case '1': //NumLock
		return 2;
	case '2': //CapsLock
		return 4;
	case '3': //ScrollLock
		return 1;
	}
	return 0;
}
