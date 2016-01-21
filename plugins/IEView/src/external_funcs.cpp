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

	HRESULT db_set(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		if (pDispParams->cArgs < 4 || pDispParams == nullptr)
			return E_INVALIDARG;

		MCONTACT hContact = pDispParams->rgvarg[3].intVal;
		BSTR szModule = pDispParams->rgvarg[2].bstrVal;
		BSTR szSetting = pDispParams->rgvarg[1].bstrVal;

		DBVARIANT dbv = { 0 };

		VARIANT& pVal = pDispParams->rgvarg[0];

		switch (pVal.vt)
		{
		case VT_BSTR:
			dbv.type = DBVT_WCHAR;
			dbv.pwszVal = mir_wstrdup(pVal.bstrVal);
			break;
		case VT_INT:
		case VT_I1:
		case VT_I2:
		case VT_I4:
		case VT_I8:
			dbv.type = DBVT_DWORD;
			dbv.dVal = pVal.intVal;
			break;
		case VT_BOOL:
			dbv.type = DBVT_BYTE;
			dbv.bVal = pVal.boolVal;
		}

		INT_PTR res = ::db_set(hContact, _T2A((TCHAR*)szModule), _T2A((TCHAR*)szSetting), &dbv);

		if (pVarResult != nullptr)
		{
			pVarResult->vt = VT_INT_PTR;
			pVarResult->ullVal = (ULONGLONG)res;
		}
		return S_OK;
	}

	HRESULT win32_ShellExecute(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		if (pDispParams->cArgs < 5 || pDispParams == nullptr)
			return E_INVALIDARG;

		HINSTANCE res = ShellExecuteW(NULL, pDispParams->rgvarg[4].bstrVal, pDispParams->rgvarg[3].bstrVal, pDispParams->rgvarg[2].bstrVal, pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].intVal);

		if (pVarResult != nullptr)
		{
			pVarResult->vt = VT_HANDLE;
			pVarResult->ullVal = (ULONGLONG)res;
		}

		return S_OK;
	}

}