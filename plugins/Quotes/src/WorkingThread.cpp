#include "StdAfx.h"

void WorkingThread(void* pParam)
{
	IQuotesProvider* pProvider = reinterpret_cast<IQuotesProvider*>(pParam);
	assert(pProvider);

	if (pProvider)
	{
		pProvider->Run();
	}
}
