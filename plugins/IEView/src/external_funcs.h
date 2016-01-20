enum EXTERNAL_FUNCTIONS
{
	DISPID_EXTERNAL_DB_GET = 652
};

namespace External
{
	HRESULT db_get(DISPPARAMS *pDispParams, VARIANT *pVarResult);
	HRESULT ShellExec(DISPPARAMS *pDispParams, VARIANT *pVarResult);
}