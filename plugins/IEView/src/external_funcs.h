enum EXTERNAL_FUNCTIONS
{
	DISPID_EXTERNAL_DB_GET = 652,
	DISPID_EXTERNAL_WIN32_SHELL_EXECUTE
};

namespace External
{
	HRESULT db_get(DISPPARAMS *pDispParams, VARIANT *pVarResult);
	HRESULT win32_ShellExecute(DISPPARAMS *pDispParams, VARIANT *pVarResult);
}