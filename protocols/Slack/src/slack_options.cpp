#include "stdafx.h"

CSlackOptionsMain::CSlackOptionsMain(CSlackProto *proto, int idDialog)
	: CSuper(proto, idDialog, false)
{
}

void CSlackOptionsMain::OnInitDialog()
{
	CSuper::OnInitDialog();
}

void CSlackOptionsMain::OnApply()
{
}

/////////////////////////////////////////////////////////////////////////////////

int CSlackProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	return 0;
}
