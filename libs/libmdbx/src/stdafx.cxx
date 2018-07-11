
#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD reason, LPVOID reserved)
{
  mdbx_dll_callback(hInstDLL, reason, reserved);
  return TRUE;
}
