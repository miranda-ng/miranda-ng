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

// Prototypes
void SetLock(uint16_t, uint8_t, unsigned int *, INPUT *);

// Globals
BOOL LEDstateSaved = FALSE;
uint8_t LEDstate;


void keypresses_RestoreLEDState(void)
{
	if (LEDstateSaved)
		keypresses_ToggleKeyboardLights(LEDstate);
	LEDstateSaved = FALSE;
}

BOOL keypresses_ToggleKeyboardLights(uint8_t byte)
{
	unsigned int n = 0;
	INPUT keystrokes[6] = {0};

	if (!LEDstateSaved) {
		LEDstate = (uint8_t)(LedState(VK_SCROLL) + (LedState(VK_NUMLOCK)<<1) + (LedState(VK_CAPITAL)<<2));
		LEDstateSaved = TRUE;
	}

	SetLock(VK_NUMLOCK, (uint8_t)(byte&2), &n, keystrokes);
	SetLock(VK_CAPITAL, (uint8_t)(byte&4), &n, keystrokes);
	SetLock(VK_SCROLL, (uint8_t)(byte&1), &n, keystrokes);
	SendInput(n, keystrokes, sizeof(INPUT));

	return TRUE;
}

void SetLock(uint16_t keyCode, uint8_t value, unsigned int *n, INPUT *keystrokes)
{
	uint8_t status;

	GetAsyncKeyState(keyCode);
	status = GetKeyState(keyCode) & 1;

	if ((!value && status) || (value && !status)) {
		keystrokes[*n].type = INPUT_KEYBOARD;
		keystrokes[*n].ki.wVk = keyCode;
		keystrokes[*n].ki.wScan = 0x45;
		keystrokes[(*n)++].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | 0;
		keystrokes[*n].type = INPUT_KEYBOARD;
		keystrokes[*n].ki.wVk = keyCode;
		keystrokes[*n].ki.wScan = 0x45;
		keystrokes[(*n)++].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
	}
}
