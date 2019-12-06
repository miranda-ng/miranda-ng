
#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD reason, LPVOID reserved)
{
   mdbx_dll_handler(hInstDLL, reason, reserved);
  return TRUE;
}
