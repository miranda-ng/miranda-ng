#include "wumf.h"

PWumf new_wumf( DWORD dwID, 
				LPTSTR szUser, 
				LPTSTR szPath, 
				LPTSTR szComp, 
				LPTSTR szUNC, 
				DWORD dwSess, 
				DWORD dwPerm, 
				DWORD dwAttr)
{
	PWumf w = (PWumf)mir_calloc(sizeof(Wumf));
	if (!w)
		return NULL;

	w->szUser = mir_tstrdup(szUser);
	w->szPath = mir_tstrdup(szPath);
	w->szComp = mir_tstrdup(szComp);
	w->szUNC  = mir_tstrdup(szUNC);

	switch(dwPerm) {
		case PERM_FILE_READ: mir_tstrcpy(w->szPerm, _T("Read"));break;
		case PERM_FILE_WRITE: mir_tstrcpy(w->szPerm, _T("Write"));break;
		case PERM_FILE_CREATE: mir_tstrcpy(w->szPerm, _T("Create"));break;
		default: mir_tstrcpy(w->szPerm, _T("Execute"));
	}
	mir_sntprintf(w->szID, SIZEOF(w->szID), _T("%i"), dwID);

	w->dwID = dwID;
	w->dwSess = dwSess;
	w->dwAttr = dwAttr;
	w->dwPerm = dwPerm;
	w->mark = FALSE;
	w->next = NULL;
	return w;	
}

BOOL del_wumf(PWumf w)
{
	if (!w) return FALSE;
	mir_free(w->szUser);
	mir_free(w->szPath);
	mir_free(w->szComp);
	mir_free(w->szUNC);
	mir_free(w);
	return TRUE;
}

BOOL add_cell(PWumf* l, PWumf w)
{
	if (!w || !l)return FALSE;
	if (!(*l))
		*l = w;
	else {
		PWumf p = *l;
		while(p->next) p = p->next;
		p->next = w;
	}
	w->next = NULL;
	return TRUE;
}

BOOL del_cell(PWumf *l, PWumf w)
{
	if (!l || !*l || !w)return FALSE;
	PWumf p = *l;
	if (w == *l)
		*l = p->next;
	else {
		while(p && p->next != w) p = p->next;
		if (!p) return FALSE;
		p->next = w->next;
	}
	return del_wumf(w);
}

BOOL cpy_cell(PWumf *l, PWumf w)
{
	PWumf w1 = new_wumf(w->dwID, w->szUser, w->szPath, w->szComp,w->szUNC, w->dwSess, w->dwPerm, w->dwAttr);
	if (!w1)
		return FALSE;
	w1->mark = w->mark;
	return add_cell(l, w1);
}

PWumf cpy_list(PWumf *l)
{
	PWumf w, p = NULL;

	if (!l || !*l) return NULL;
	w = *l;
	while(w) {
		if (!cpy_cell(&p, w))return NULL;
		w = w->next;
	}
	return p;
}

PWumf fnd_cell(PWumf *l, DWORD dwID)
{
	if (!l || !*l)return NULL;
	PWumf w = *l;
	while(w && w->dwID != dwID) w = w->next;
	return w;
}

BOOL del_all(PWumf *l)
{
	if (!l || !*l) return FALSE;
	PWumf w = *l;
	while(w) {
		PWumf p = w->next;
		if (!del_cell(l, w)) 
			return FALSE;

		w = p;
	}
	*l = NULL;
	return TRUE;
}

BOOL del_marked(PWumf *l)
{
	PWumf w, p;
	if (!l)return FALSE;
	w = *l;
	while(w) {
		p = w->next;
		if (w->mark)
			if (!del_cell(l, w))
				return FALSE;

		w = p;
	}
	return TRUE;
}

void mark_all(PWumf *l, BOOL mark)
{
	PWumf w = *l;
	while(w) {
		w->mark = mark;
		w = w->next;
	}
}
