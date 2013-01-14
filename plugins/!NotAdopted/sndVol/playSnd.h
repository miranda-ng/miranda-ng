#include <windows.h>
#include <mmsystem.h>
#include <basetsd.h>
#include "dmusicc.h"
#include "dmusici.h"
#include <dxerr8.h>
#include <tchar.h>
#include "DMUtil.h"
#include "DXUtil.h"

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------

namespace playSnd
{
	extern BOOL g_bInOption;

	HRESULT Init();
	void Destroy();
	HRESULT LoadSegmentFile( TCHAR* strFileName );
	BOOL WINAPI PlaySound(char* pszSound, HMODULE hmod, DWORD fdwSound);
	BOOL SetVolume(long Value);
}
