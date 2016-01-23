#include "stdafx.h"

namespace External
{

	HRESULT mir_CallService(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		if (pDispParams == nullptr || pDispParams->cArgs < 3)
			return E_INVALIDARG;

		BSTR szName = pDispParams->rgvarg[2].bstrVal;
		WPARAM wParam = 0;
		LPARAM lParam = 0;

		switch (pDispParams->rgvarg[1].vt)
		{
		case VT_BSTR:
			wParam = (WPARAM)pDispParams->rgvarg[1].bstrVal;
		case VT_INT:
		case VT_I1:
		case VT_I2:
		case VT_I4:
		case VT_I8:
			wParam = (WPARAM)pDispParams->rgvarg[1].intVal;
		}

		switch (pDispParams->rgvarg[0].vt)
		{
		case VT_BSTR:
			lParam = (LPARAM)pDispParams->rgvarg[0].bstrVal;
		case VT_INT:
		case VT_I1:
		case VT_I2:
		case VT_I4:
		case VT_I8:
			lParam = (LPARAM)pDispParams->rgvarg[0].intVal;
		}

		INT_PTR res = CallService(_T2A((TCHAR*)szName), wParam, lParam);

		if (pVarResult != nullptr)
		{
			pVarResult->vt = VT_UINT;
			pVarResult->uintVal = (UINT)res;
		}
		return S_OK;
	}

	HRESULT IEView_GetCurrentContact(IEView *self, DISPPARAMS*, VARIANT *pVarResult)
	{
		if (pVarResult != nullptr)
		{
			pVarResult->vt = VT_UINT;
			pVarResult->uintVal = self->Get_CurrentContact();
		}
		return S_OK;
	}

	HRESULT db_get(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		if (pDispParams->cArgs < 3)
			return E_INVALIDARG;

		MCONTACT hContact = pDispParams->rgvarg[2].intVal;
		BSTR szModule = pDispParams->rgvarg[1].bstrVal;
		BSTR szSetting = pDispParams->rgvarg[0].bstrVal;

		DBVARIANT dbv = { 0 };

		if (db_get(hContact, _T2A((TCHAR*)szModule), _T2A((TCHAR*)szSetting), &dbv)) 
			return S_OK;

		if (pVarResult != nullptr)
		{
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
				break;
			case DBVT_WORD:
				pVarResult->vt = VT_I2;
				pVarResult->iVal = dbv.dVal;
				break;
			}
		}
		db_free(&dbv);
		return S_OK;
	}

	HRESULT db_set(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		if (pDispParams == nullptr || pDispParams->cArgs < 4)
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
			break;
		default: 
			return E_INVALIDARG;
		}

		INT_PTR res = ::db_set(hContact, _T2A((TCHAR*)szModule), _T2A((TCHAR*)szSetting), &dbv);

		if (pVarResult != nullptr)
		{
			pVarResult->vt = VT_INT;
			pVarResult->intVal = (int)res;
		}
		return S_OK;
	}

	HRESULT win32_ShellExecute(DISPPARAMS *pDispParams, VARIANT *pVarResult)
	{
		if (pDispParams == nullptr || pDispParams->cArgs < 5)
			return E_INVALIDARG;

		HINSTANCE res = ShellExecuteW(NULL, pDispParams->rgvarg[4].bstrVal, pDispParams->rgvarg[3].bstrVal, pDispParams->rgvarg[2].bstrVal, pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].intVal);

		if (pVarResult != nullptr)
		{
			pVarResult->vt = VT_HANDLE;
			pVarResult->ullVal = (ULONGLONG)res;
		}

		return S_OK;
	}

	HRESULT win32_CopyToClipboard(DISPPARAMS *pDispParams, VARIANT*)
	{
		if (pDispParams == nullptr || pDispParams->cArgs < 1)
			return E_INVALIDARG;

		BSTR data = pDispParams->rgvarg[0].bstrVal;
		if (OpenClipboard(NULL)) 
		{
			EmptyClipboard();
			size_t size = sizeof(TCHAR)* (mir_wstrlen(data) + 1);
			HGLOBAL hClipboardData = GlobalAlloc(0, size);
			if (hClipboardData) 
			{
				TCHAR *pchData = (TCHAR*)GlobalLock(hClipboardData);
				if (pchData) 
				{
					memcpy(pchData, (TCHAR*)data, size);
					GlobalUnlock(hClipboardData);
					SetClipboardData(CF_UNICODETEXT, hClipboardData);
				}
			}
			CloseClipboard();
		}
		return S_OK;
	}

}