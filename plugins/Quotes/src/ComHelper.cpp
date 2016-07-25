#include "StdAfx.h"

tstring ComException2Msg(_com_error& e, const tstring& rsAdditionalInfo)
{
	HRESULT hError = e.Error();
	tostringstream o;
	if (false == rsAdditionalInfo.empty())
		o << rsAdditionalInfo << "\n";

	o << e.ErrorMessage() << L" (" << std::hex << hError << L")";

	IErrorInfo* p = e.ErrorInfo();
	CComPtr<IErrorInfo> pErrorInfo(p);
	if (NULL != p)
		p->Release();

	if (pErrorInfo)
		o << L"\n" << e.Description();

	return o.str();
}

void ShowComError(_com_error& e, const tstring& rsAdditionalInfo)
{
	tstring sErrorMsg = ComException2Msg(e, rsAdditionalInfo);
	LogIt(sErrorMsg);
	Quotes_MessageBox(NULL, sErrorMsg.c_str(), MB_OK | MB_ICONERROR);
}
