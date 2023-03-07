#include "stdafx.h"

template<typename T>
T Var_To(VARIANTARG &pVar, char strType = 'W')
{
	T retVal = NULL;
	switch (pVar.vt) {
	case VT_BSTR:
		if (strType == 'U')
			retVal = (T)mir_utf8encodeW(pVar.bstrVal);
		else if (strType == 'A')
			retVal = (T)mir_u2a(pVar.bstrVal);
		else
			retVal = (T)pVar.bstrVal;
		break;

	case VT_INT:
	case VT_I1:
	case VT_I2:
	case VT_I4:
	case VT_I8:
		retVal = (T)pVar.intVal;
		break;
	}
	return retVal;
}

static const wchar_t* STR(const VARIANTARG &pVar)
{
	return (pVar.vt == VT_BSTR) ? pVar.bstrVal : L"";
}

namespace External
{
	HRESULT mir_CallService(DISPPARAMS *p, VARIANT *pVarResult)
	{
		if (p == nullptr || p->cArgs < 3)
			return E_INVALIDARG;

		wchar_t wType = 'W', lType = 'W';
		if (p->cArgs >= 5) lType = p->rgvarg[4].bstrVal[0];
		if (p->cArgs >= 4) wType = p->rgvarg[3].bstrVal[0];

		BSTR szName = p->rgvarg[2].bstrVal;
		WPARAM wParam = Var_To<WPARAM>(p->rgvarg[1], wType);
		LPARAM lParam = Var_To<LPARAM>(p->rgvarg[0], lType);

		INT_PTR res = CallService(_T2A((wchar_t *)szName), wParam, lParam);

		if (wType == 'A' || wType == 'U') mir_free((void *)wParam);
		if (lType == 'A' || lType == 'U') mir_free((void *)lParam);

		if (pVarResult != nullptr) {
			pVarResult->vt = VT_UINT;
			pVarResult->uintVal = (UINT)res;
		}
		return S_OK;
	}

	HRESULT mir_CallContactService(DISPPARAMS *p, VARIANT *pVarResult)
	{
		if (p == nullptr || p->cArgs < 4)
			return E_INVALIDARG;

		wchar_t wType = 'W', lType = 'W';
		if (p->cArgs >= 6) lType = p->rgvarg[5].bstrVal[0];
		if (p->cArgs >= 5) wType = p->rgvarg[4].bstrVal[0];

		MCONTACT hContact = p->rgvarg[3].intVal;
		BSTR szName = p->rgvarg[2].bstrVal;
		WPARAM wParam = Var_To<WPARAM>(p->rgvarg[1], wType);
		LPARAM lParam = Var_To<LPARAM>(p->rgvarg[0], lType);

		INT_PTR res = ProtoChainSend(hContact, _T2A((wchar_t *)szName), wParam, lParam);

		if (wType == 'A' || wType == 'U') mir_free((void *)wParam);
		if (lType == 'A' || lType == 'U') mir_free((void *)lParam);

		if (pVarResult != nullptr) {
			pVarResult->vt = VT_UINT;
			pVarResult->uintVal = (UINT)res;
		}
		return S_OK;
	}

	HRESULT IEView_GetCurrentContact(IEView *self, DISPPARAMS *, VARIANT *pVarResult)
	{
		if (pVarResult != nullptr) {
			pVarResult->vt = VT_UINT;
			pVarResult->uintVal = self->Get_CurrentContact();
		}
		return S_OK;
	}

	HRESULT db_get(DISPPARAMS *p, VARIANT *pVarResult)
	{
		if (p->cArgs < 3)
			return E_INVALIDARG;

		MCONTACT hContact = p->rgvarg[2].intVal;
		BSTR szModule = p->rgvarg[1].bstrVal;
		BSTR szSetting = p->rgvarg[0].bstrVal;

		DBVARIANT dbv = { 0 };

		if (db_get(hContact, _T2A((wchar_t *)szModule), _T2A((wchar_t *)szSetting), &dbv))
			return S_OK;

		if (pVarResult != nullptr) {
			switch (dbv.type) {
			case DBVT_BYTE:
				//pVarResult->bVal = dbv.bVal != 0;
				//pVarResult->vt = VT_BOOL;
				pVarResult->vt = VT_UI1;
				pVarResult->bVal = dbv.bVal;
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
				pVarResult->iVal = dbv.wVal;
				break;
			}
		}
		db_free(&dbv);
		return S_OK;
	}

	HRESULT db_set(DISPPARAMS *p, VARIANT *pVarResult)
	{
		if (p == nullptr || p->cArgs < 4)
			return E_INVALIDARG;

		MCONTACT hContact = p->rgvarg[3].intVal;
		BSTR szModule = p->rgvarg[2].bstrVal;
		BSTR szSetting = p->rgvarg[1].bstrVal;

		DBVARIANT dbv = { 0 };

		VARIANT &pVal = p->rgvarg[0];

		switch (pVal.vt) {
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

		INT_PTR res = ::db_set(hContact, _T2A((wchar_t *)szModule), _T2A((wchar_t *)szSetting), &dbv);

		if (pVarResult != nullptr) {
			pVarResult->vt = VT_INT;
			pVarResult->intVal = (int)res;
		}
		return S_OK;
	}

	HRESULT win32_ShellExecute(DISPPARAMS *p, VARIANT *pVarResult)
	{
		if (p == nullptr || p->cArgs < 5)
			return E_INVALIDARG;

		HINSTANCE res = ShellExecuteW(nullptr, STR(p->rgvarg[4]), STR(p->rgvarg[3]), STR(p->rgvarg[2]), STR(p->rgvarg[1]), p->rgvarg[0].intVal);

		if (pVarResult != nullptr) {
			pVarResult->vt = VT_HANDLE;
			pVarResult->ullVal = (ULONGLONG)res;
		}

		return S_OK;
	}

	HRESULT win32_CopyToClipboard(DISPPARAMS *p, VARIANT *)
	{
		if (p == nullptr || p->cArgs < 1)
			return E_INVALIDARG;

		Utils_ClipboardCopy(p->rgvarg[0].bstrVal);
		return S_OK;
	}
}
