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

#include "stdafx.h"

struct FLASHING_SEQUENCE
{
	unsigned int size;
	unsigned int index;
	uint8_t frame[MAX_PATH];
};


// Prototypes
FLASHING_SEQUENCE *getCustomSeq(void);
FLASHING_SEQUENCE *getPredefinedSeq(void);
FLASHING_SEQUENCE *getTrillianSeq(void);
void updateTrillianSeq(void);
static void __cdecl TestThread(void *param);
static void PreviewThread(void *);
FLASHING_SEQUENCE str2FS(wchar_t *);
uint8_t KbdChar2Byte(char);
void countUnopenEvents(int *, int *, int *);

#define Leds2Flash	((uint8_t)(bFlashLed[2] + (bFlashLed[0]<<1) + (bFlashLed[1]<<2)))

// Flashing settings
FLASHING_SEQUENCE *pFS;
BOOL bTemporarilyUseExtern;
extern uint8_t bFlashLed[3];
extern uint8_t bFlashEffect; extern uint8_t bSequenceOrder;
extern uint16_t wCustomTheme;
extern uint8_t bTrillianLedsMsg, bTrillianLedsFile, bTrillianLedsOther;
extern uint8_t bEmulateKeypresses;

// TestThread/PreviewThread globals
extern int nWaitDelay; extern uint16_t wStartDelay;
BOOL bTestSemaphore, bPreviewSemaphore, bPreview;

void RestoreLEDState(void)
{
	if (bEmulateKeypresses)
		keypresses_RestoreLEDState();
	else
		ToggleKeyboardLights((uint8_t)(LedState(VK_SCROLL) + (LedState(VK_NUMLOCK)<<1) + (LedState(VK_CAPITAL)<<2)));
}

uint8_t getBlinkingLeds(void)
{
	if (!pFS->size)
		return (uint8_t)(LedState(VK_SCROLL) + (LedState(VK_NUMLOCK)<<1) + (LedState(VK_CAPITAL)<<2));

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
	ptrW tszSeq( g_plugin.getWStringA(fmtDBSettingName("custom%d", wCustomTheme)));

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
	int i, msgCount=0, fileCount=0, otherCount=0;

	pFS->size = 2;
	countUnopenEvents(&msgCount, &fileCount, &otherCount);

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

	if ((bTrillianLedsOther & Leds2Flash) && (pFS->size + 2 * otherCount) <= MAX_PATH)
		for (i=0; i < otherCount; i++) {
			pFS->frame[pFS->size++] = bTrillianLedsOther & Leds2Flash;
			pFS->frame[pFS->size++] = 0;
		}
}

void useExternSequence(wchar_t *extStr)
{
	static FLASHING_SEQUENCE Extern = {0};

	wchar_t externStr[MAX_PATH+1];
	mir_wstrcpy(externStr, extStr);

	Extern = str2FS(normalizeCustomString(externStr));

	pFS = &Extern;
	bTemporarilyUseExtern = TRUE;
}


wchar_t *normalizeCustomString(wchar_t *customStr)
{
	int len=0, status=0;
	BOOL used[4];
	wchar_t strAux[MAX_PATH+1], *str;

	for (mir_wstrcpy(str=strAux, customStr); *str; str++) {
		switch (*str) {
		case '[':
			if (status == 0) {
				status = 1;
				customStr[len++] = *str;
				used[0] = used [1] = used[2] = used[3] = FALSE;
			}
			break;
		case ']':
			if (status == 1) {
				status = 0;
				customStr[len++] = *str;
			}
			break;
		case '0':
		case '1':
		case '2':
		case '3':
			if (status == 0)
				customStr[len++] = *str;
			else if (!used[*str - '0']) {
				customStr[len++] = *str;
				used[*str - '0'] = TRUE;
			}
			break;
		}
	}
	if (status == 1)
		customStr[len++] = ']';
	customStr[len] = '\0';

	return customStr;
}

wchar_t* getCurrentSequenceString(void)
{
	static wchar_t CurrentSeqString[MAX_PATH+1];
	wchar_t *str = CurrentSeqString;

	for (unsigned i=0; i < pFS->size; i++) {
		switch (pFS->frame[i]) {
		case 0:
			*(str++) = '0';
			break;
		case 1:
			*(str++) = '3';
			break;
		case 2:
			*(str++) = '1';
			break;
		case 3:
			*(str++) = '[';
			*(str++) = '1';
			*(str++) = '3';
			*(str++) = ']';
			break;
		case 4:
			*(str++) = '2';
			break;
		case 5:
			*(str++) = '[';
			*(str++) = '2';
			*(str++) = '3';
			*(str++) = ']';
			break;
		case 6:
			*(str++) = '[';
			*(str++) = '1';
			*(str++) = '2';
			*(str++) = ']';
			break;
		case 7:
			*(str++) = '[';
			*(str++) = '1';
			*(str++) = '2';
			*(str++) = '3';
			*(str++) = ']';
		}
	}
	*str = '\0';

	return CurrentSeqString;
}

void testSequence(wchar_t *testStr)
{
	if (bTestSemaphore) // we try to avoid concurrent test threads
		return;

	bTestSemaphore = TRUE;

	static FLASHING_SEQUENCE Test = {0};
	Test = str2FS(testStr);
	
	mir_forkthread(TestThread, &Test);
}


static void __cdecl TestThread(void *param)
{
	FLASHING_SEQUENCE *pTest = (FLASHING_SEQUENCE *)param;
	unsigned int testNum = (unsigned int)g_plugin.getByte("testnum", DEF_SETTING_TESTNUM);
	unsigned int testSecs = (unsigned int)g_plugin.getByte("testsecs", DEF_SETTING_TESTSECS);

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
	mir_forkthread(PreviewThread);
}


static void PreviewThread(void*)
{
	if (wStartDelay > 0)
		Sleep(wStartDelay * 1000);

	uint8_t unchangedLeds = (uint8_t)(LedState(VK_SCROLL) * !bFlashLed[2] + ((LedState(VK_NUMLOCK) * !bFlashLed[0])<<1) + ((LedState(VK_CAPITAL) * !bFlashLed[1])<<2));

	while (bPreview)
		for (unsigned i=0; bPreview && i < pFS->size; i++) {
			ToggleKeyboardLights((uint8_t)(pFS->frame[i%pFS->size]|unchangedLeds));
			Sleep(nWaitDelay);				
		}

	RestoreLEDState();

	bPreviewSemaphore = FALSE;	
}


FLASHING_SEQUENCE str2FS(wchar_t *str)
{
	FLASHING_SEQUENCE Temp = {0};

	for (Temp.size=Temp.index=0; *str; str++) {
		Temp.size++;
		if (*str == '[') {
			Temp.frame[Temp.size - 1] = 0;
			for (str++; *str && *str != ']'; str++)
				Temp.frame[Temp.size - 1] += KbdChar2Byte(*str) & Leds2Flash;
			if (!*str) break;
		}
		else Temp.frame[Temp.size - 1] = KbdChar2Byte(*str) & Leds2Flash;
	}

	return Temp;
}


uint8_t KbdChar2Byte(char kbdChar)
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
