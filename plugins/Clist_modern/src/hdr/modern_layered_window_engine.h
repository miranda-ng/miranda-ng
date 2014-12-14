#pragma once
#include "../m_api/m_skin_eng.h"

// typedef int (/*__stdcall*/ *tPaintCallbackProc)(HWND hWnd, HDC hDC, RECT *rcPaint, HRGN rgnUpdate, DWORD dFlags, void * CallBackData);

class CLayeredWindowEngine
{
private:
	/*class CLweInfo
	{
	HWND hWnd;
	HRGN hInvalidRgn;
	};
	*/
	//typedef std::map<HWND, CLweInfo> WndInfos;

	enum { state_invalid, state_normal };

	//WndInfos			m_infos;
	DWORD				m_hValidatorThread;
	CRITICAL_SECTION	m_cs;
	int					m_state;
	volatile bool		m_invalid;

public:
	CLayeredWindowEngine(void);
	~CLayeredWindowEngine(void);

	void _init();
	void _deinit();

	void lock() { EnterCriticalSection(&m_cs); }
	void unlock() { LeaveCriticalSection(&m_cs); }

	int get_state();

public:
	static void __cdecl LweValidatorProc();

	void LweValidatorProcWorker();

	void LweValidatorWorker();
	int LweInvalidateRect(HWND hWnd, const RECT *rect, BOOL bErase);
	// int LweValidateWindowRect( HWND hWnd, RECT *rect );
	// int RegisterWindow( HWND hwnd, tPaintCallbackProc pPaintCallBackProc );

};

extern CLayeredWindowEngine _lwe;

#define _InvalidateRect _lwe.LweInvalidateRect