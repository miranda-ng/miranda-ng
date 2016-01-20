#include "stdafx.h"

namespace External
{
	HRESULT db_get(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		if (pDispParams->cArgs < 3)
			return TYPE_E_OUTOFBOUNDS;

		if (!pDispParams || !pVarResult)
			return S_OK;

		MCONTACT hContact = pDispParams->rgvarg[2].vt == VT_INT ? pDispParams->rgvarg[2].intVal : NULL;
		BSTR szModule = pDispParams->rgvarg[1].vt == VT_BSTR ? pDispParams->rgvarg[1].bstrVal : NULL;
		BSTR szSetting = pDispParams->rgvarg[0].vt == VT_BSTR ? pDispParams->rgvarg[0].bstrVal : NULL;

		DBVARIANT dbv = { 0 };
		db_get(hContact, _T2A((TCHAR*)szModule), _T2A((TCHAR*)szSetting), &dbv);

		switch (dbv.type)
		{
		case DBVT_BYTE:
			pVarResult->bVal = dbv.bVal;
			pVarResult->vt = VT_BOOL;
			break;
		case DBVT_WCHAR:
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString(dbv.pwszVal);
			break;
		case DBVT_UTF8:
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString(ptrW(mir_utf8decodeW(dbv.pszVal)));
			break;
		case DBVT_ASCIIZ:
			pVarResult->vt = VT_BSTR;
			pVarResult->bstrVal = ::SysAllocString(_A2T(dbv.pszVal));
			break;
		case DBVT_DWORD:
			pVarResult->vt = VT_INT;
			pVarResult->intVal = dbv.dVal;
		}
		return S_OK;
	}

	HRESULT ShellExec(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		return S_OK;
	}

}