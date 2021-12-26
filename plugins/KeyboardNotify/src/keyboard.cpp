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

// Globals
extern uint8_t bEmulateKeypresses;
HANDLE hKbdDev[10] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

// Defines
#define MAX_KBDHANDLES                  10
#define IOCTL_KEYBOARD_SET_INDICATORS	CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC	CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS	CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _KEYBOARD_INDICATOR_PARAMETERS {
	USHORT UnitId;		// Unit identifier.
	USHORT LedFlags;	// LED indicator state.

} KEYBOARD_INDICATOR_PARAMETERS, *PKEYBOARD_INDICATOR_PARAMETERS;

BOOL OpenKeyboardDevice()
{
	int i = 0;
	wchar_t aux1[MAX_PATH+1], aux2[MAX_PATH+1];

	do {
		mir_snwprintf(aux1, L"Kbd%d", i);
		mir_snwprintf(aux2, L"\\Device\\KeyboardClass%d", i);
		DefineDosDevice(DDD_RAW_TARGET_PATH, aux1, aux2);

		mir_snwprintf(aux1, L"\\\\.\\Kbd%d", i);
		hKbdDev[i] = CreateFile(aux1, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	} while (hKbdDev[i] != INVALID_HANDLE_VALUE && ++i < MAX_KBDHANDLES);
	
	return hKbdDev[0] != INVALID_HANDLE_VALUE;
}

BOOL ToggleKeyboardLights(uint8_t byte)
{
	int i; BOOL result = FALSE;
	KEYBOARD_INDICATOR_PARAMETERS InputBuffer;	// Input buffer for DeviceIoControl
	ULONG DataLength = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
	ULONG ReturnedLength;						// Number of bytes returned in output buffer

	if (bEmulateKeypresses)
		return keypresses_ToggleKeyboardLights(byte);
	
	InputBuffer.UnitId = 0;
	InputBuffer.LedFlags = byte;

	for (i=0; i < MAX_KBDHANDLES && hKbdDev[i] != INVALID_HANDLE_VALUE; i++)
		result |= DeviceIoControl(hKbdDev[i], IOCTL_KEYBOARD_SET_INDICATORS, &InputBuffer, DataLength, nullptr, 0, &ReturnedLength, nullptr);

	return result;
}

void CloseKeyboardDevice()
{
	int i = 0;
	wchar_t aux[MAX_PATH+1];

	do {
		if (hKbdDev[i] != INVALID_HANDLE_VALUE)
			CloseHandle(hKbdDev[i]);

		mir_snwprintf(aux, L"Kbd%d", i);
		DefineDosDevice(DDD_REMOVE_DEFINITION, aux, nullptr);

	} while (hKbdDev[i] != INVALID_HANDLE_VALUE && ++i < MAX_KBDHANDLES);
}
