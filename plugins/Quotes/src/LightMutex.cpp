#include "StdAfx.h"

CLightMutex::CLightMutex()
{
	InitializeCriticalSection(&m_cs);
}

CLightMutex::~CLightMutex()
{
	::DeleteCriticalSection(&m_cs);
}

void CLightMutex::Lock()
{
	::EnterCriticalSection(&m_cs);
}

void CLightMutex::Unlock()
{
	::LeaveCriticalSection(&m_cs);
}
