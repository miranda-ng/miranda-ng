#include "commonheaders.h"

// get context data on context id
pCNTX get_context_on_id(HANDLE context)
{
	if (context) {
		pCNTX cntx = (pCNTX)context;
		if (cntx->header == HEADER && cntx->footer == FOOTER)
			return cntx;

#if defined(_DEBUG) || defined(NETLIB_LOG)
		Sent_NetLog("get_context_on_id: corrupted context %08X", cntx);
#endif
	}
	return NULL;
}

// create context, return context id
HANDLE __cdecl cpp_create_context(int mode)
{
	pCNTX cntx = (pCNTX)malloc(sizeof(CNTX));
	memset(cntx, 0, sizeof(CNTX)); // очищаем выделенный блок
	cntx->header = HEADER;
	cntx->footer = FOOTER;
	cntx->mode = mode;
	return (HANDLE)cntx;
}

// delete context
void __cdecl cpp_delete_context(HANDLE context)
{
	pCNTX tmp = get_context_on_id(context);
	if (tmp) { // помечаем на удаление
		cpp_free_keys(tmp);
		free(tmp);
	}
}

// reset context
void __cdecl cpp_reset_context(HANDLE context)
{
	pCNTX tmp = get_context_on_id(context);
	if (tmp)
		cpp_free_keys(tmp);
}

// allocate pdata
PBYTE cpp_alloc_pdata(pCNTX ptr)
{
	if (!ptr->pdata) {
		if (ptr->mode & MODE_PGP) {
			ptr->pdata = (PBYTE)malloc(sizeof(PGPDATA));
			memset(ptr->pdata, 0, sizeof(PGPDATA));
		}
		else if (ptr->mode & MODE_GPG) {
			ptr->pdata = (PBYTE)malloc(sizeof(GPGDATA));
			memset(ptr->pdata, 0, sizeof(GPGDATA));
		}
		else if (ptr->mode & MODE_RSA) {
			rsa_alloc(ptr);
		}
		else {
			ptr->pdata = (PBYTE)malloc(sizeof(SIMDATA));
			memset(ptr->pdata, 0, sizeof(SIMDATA));
		}
	}
	return ptr->pdata;
}

// free memory from keys
void cpp_free_keys(pCNTX ptr)
{
	replaceStr(ptr->tmp, 0);
	cpp_alloc_pdata(ptr);
	if (ptr->mode & MODE_PGP) {
		pPGPDATA p = (pPGPDATA)ptr->pdata;
		SAFE_FREE(p->pgpKeyID);
		SAFE_FREE(p->pgpKey);
		SAFE_FREE(ptr->pdata);
	}
	else if (ptr->mode & MODE_GPG) {
		pGPGDATA p = (pGPGDATA)ptr->pdata;
		SAFE_FREE(p->gpgKeyID);
		SAFE_FREE(ptr->pdata);
	}
	else if (ptr->mode & MODE_RSA) {
		if (rsa_free(ptr))
			SAFE_DELETE(ptr->pdata);
	}
	else {
		pSIMDATA p = (pSIMDATA)ptr->pdata;
		SAFE_FREE(p->PubA);
		SAFE_FREE(p->KeyA);
		mir_free(p->KeyB);
		SAFE_FREE(p->KeyX);
		SAFE_FREE(p->KeyP);
		SAFE_DELETE(p->dh);
		SAFE_FREE(ptr->pdata);
	}
}
