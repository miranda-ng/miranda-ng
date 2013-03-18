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
	//TCHAR szID[10];

	PWumf w = (PWumf)malloc(sizeof(Wumf));
	if(!w)return NULL;
	
	//#define SCPY(X) if(X){w->X = (LPTSTR)malloc(1+_tcslen(X));if(!w->X)return NULL;_tcscpy(w->X, X);} else { w->X = NULL;}
	//#define SCPYW(X) if(X){w->X = (LPTSTR)malloc(1+lstrlenW((LPWSTR)X));if(!w->X)return NULL;wsprintfA(w->X, "%S", X);} else { w->X = NULL;}

    //#define SCCPY(X, Y) w->X = (LPSTR)malloc(1+strlen(Y));if(!w->X)return NULL;strcpy(w->X, Y)

	_tcscpy(w->szUser, szUser);
	_tcscpy(w->szPath, szPath);
	_tcscpy(w->szComp, szComp);
	_tcscpy(w->szUNC, szUNC);

	switch(dwPerm)
	{
	case PERM_FILE_READ: _tcscpy(w->szPerm, _T("Read"));break;
		case PERM_FILE_WRITE: _tcscpy(w->szPerm, _T("Write"));break;
		case PERM_FILE_CREATE: _tcscpy(w->szPerm, _T("Create"));break;
		default: _tcscpy(w->szPerm, _T("Execute"));//w->szPerm = NULL;
	};
	wsprintf(w->szID, _T("%i"), dwID);
	//_tcscpy(w->szID, szID);
	//SCPY(szID);

	//#undef SCPY
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
	if(!w) return FALSE;
	free(w->szUser);
	free(w->szPath);
	free(w->szComp);
	free(w->szUNC);
	free(w->szID);
	free(w->szPerm);
	free(w);
	return TRUE;
}

BOOL  add_cell(PWumf* l, PWumf w)
{
    PWumf p;
	if(!w || !l)return FALSE;
	if(!(*l))
	{
		*l = w;
	}
	else
	{
		p = *l;
		while(p->next) p = p->next;
		p->next = w;
	}
	w->next = NULL;
	return TRUE;
}

BOOL  del_cell(PWumf *l, PWumf w)
{
	PWumf p;
	if(!l || !*l || !w)return FALSE;
	p = *l;
	if(w == *l)
		*l = p->next;
	else
	{
		while(p && p->next != w) p = p->next;
		if(!p) return FALSE;
		p->next = w->next;
	}
	return del_wumf(w);
	
};

BOOL  cpy_cell(PWumf *l, PWumf w)
{
	PWumf w1;
	w1 = new_wumf(w->dwID, w->szUser, w->szPath, w->szComp,w->szUNC, w->dwSess, w->dwPerm, w->dwAttr);
	if(!w1) return FALSE;
	w1->mark = w->mark;
	return add_cell(l, w1);
};

PWumf cpy_list(PWumf *l)
{
	PWumf w, p = NULL;

	if(!l || !*l) return NULL;
	w = *l;
	while(w) 
	{
		if(!cpy_cell(&p, w))return NULL;
		w = w->next;
	}
	return p;
}

PWumf fnd_cell(PWumf *l, DWORD dwID)
{
	PWumf w;
	if(!l || !*l)return NULL;
	w = *l;
	while(w && w->dwID != dwID) w = w->next;
	return w;
}
	
BOOL del_all(PWumf *l)
{
	PWumf w, p;
	if(!l || !*l) return FALSE;
	w = *l;
	while(w)
	{
		p = w->next;
		if(!del_cell(l, w)) 
		{	
			return FALSE;
		}
		w = p;
	}
	*l = NULL;
	
	return TRUE;
}

BOOL del_marked(PWumf *l)
{
	PWumf w, p;
	if(!l)return FALSE;
	w = *l;
	while(w)
	{
		p = w->next;
		if(w->mark)
		{
			if(!del_cell(l, w)) return FALSE;
		};
		w = p;
	}
	return TRUE;
}

void mark_all(PWumf *l, BOOL mark)
{
	PWumf w;
	w = *l;
	while(w)
	{
		w->mark = mark;
		w = w->next;
	}
}
