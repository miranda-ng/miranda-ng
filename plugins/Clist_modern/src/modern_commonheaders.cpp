#define DB_USEHELPERFUNCTIONS
#include "hdr/modern_commonheaders.h"

BYTE gl_TrimText = 1;

char * __cdecl strstri( char *a, const char *b)
{
    char * x, *y;
    if ( !a || !b) return FALSE;
    x = _strdup(a);
    y = _strdup(b);
    x = _strupr(x);
    y = _strupr(y);
    char * pos = strstr(x,y);
    if ( pos )
    {
        char * retval = a + ( pos - x );
        free(x);
        free(y);
        return retval;
    }
    free(x);
    free(y);
    return NULL;
}
int __cdecl mir_strcmpi(const char *a, const char *b)
{
	if (a == NULL && b == NULL) return 0;
	if (a == NULL || b == NULL) return _stricmp(a?a:"",b?b:"");
    return _stricmp(a,b);
}

int __cdecl mir_tstrcmpi(const TCHAR *a, const TCHAR *b)
{
	if (a == NULL && b == NULL) return 0;
	if (a == NULL || b == NULL) return _tcsicmp(a?a:_T(""),b?b:_T(""));
	return _tcsicmp(a,b);
}
BOOL __cdecl mir_bool_strcmpi(const char *a, const char *b)
{
	if (a == NULL && b == NULL) return 1;
	if (a == NULL || b == NULL) return _stricmp(a?a:"",b?b:"") == 0;
    return _stricmp(a,b) == 0;
}

BOOL __cdecl mir_bool_tstrcmpi(const TCHAR *a, const TCHAR *b)
{
	if (a == NULL && b == NULL) return 1;
	if (a == NULL || b == NULL) return _tcsicmp(a?a:_T(""),b?b:_T("")) == 0;
	return _tcsicmp(a,b) == 0;
}

#ifdef strlen
#undef strcmp
#undef strlen
#endif

int __cdecl mir_strcmp (const char *a, const char *b)
{
	if ( !(a && b)) return a != b;
	return (strcmp(a,b));
};

int mir_strlen (const char *a)	
{	
	if (a == NULL) return 0;	
	return (int)(strlen(a));	
};	
 	 	
#define strlen(a) mir_strlen(a)
#define strcmp(a,b) mir_strcmp(a,b)
 	
/*
__inline void *mir_calloc( size_t num, size_t size )
{
 	void *p = mir_alloc(num*size);
	if (p == NULL) return NULL;
	memset(p,0,num*size);
    return p;
};
*/
wchar_t * mir_strdupW(const wchar_t * src)
{
	wchar_t * p;
	if (src == NULL) return NULL;
	p = (wchar_t *) mir_alloc((lstrlenW(src)+1)*sizeof(wchar_t));
	if ( !p) return 0;
	lstrcpyW(p, src);
	return p;
}

//copy len symbols from string - do not check is it null terminated or len is more then actual 
char * strdupn(const char * src, int len)
{
    char * p;
    if (src == NULL) return NULL;
    p = (char*)malloc(len+1);
    if ( !p) return 0;
    memcpy(p,src,len);
    p[len] = '\0';
    return p;
}

DWORD exceptFunction(LPEXCEPTION_POINTERS EP) 
{ 
    //printf("1 ");                     // printed first 
	char buf[4096];
	
	
	mir_snprintf(buf, SIZEOF(buf), "\r\nExceptCode: %x\r\nExceptFlags: %x\r\nExceptAddress: %p\r\n",
		EP->ExceptionRecord->ExceptionCode,
		EP->ExceptionRecord->ExceptionFlags,
		EP->ExceptionRecord->ExceptionAddress
		);
	TRACE(buf);
	MessageBoxA(0,buf,"clist_mw Exception",0);

    
	return EXCEPTION_EXECUTE_HANDLER; 
} 

#ifdef _DEBUG
#undef DeleteObject
#endif 

void TRACE_ERROR()
{
		DWORD t = GetLastError();
		LPVOID lpMsgBuf;
		if ( !FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			t,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL ))
		{
		// Handle the error.
		return ;
		}
#ifdef _DEBUG
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );		
		DebugBreak();
#endif
		LocalFree( lpMsgBuf );

}

BOOL DebugDeleteObject(HGDIOBJ a)
{
	BOOL res = DeleteObject(a);
	if ( !res) TRACE_ERROR();
	return res;
}

BOOL mod_DeleteDC(HDC hdc)
{
//  ske_ResetTextEffect(hdc);
  return DeleteDC(hdc);
}
#ifdef _DEBUG
#define DeleteObject(a) DebugDeleteObject(a)
#endif 


// load small icon (shared) it's not need to be destroyed

HICON LoadSmallIconShared(HINSTANCE hInstance, LPCTSTR lpIconName)
{
		int cx = GetSystemMetrics(SM_CXSMICON);
		return (HICON)LoadImage(hInstance,lpIconName, IMAGE_ICON,cx,cx, LR_DEFAULTCOLOR|LR_SHARED);
}

// load small icon (not shared) it IS NEED to be destroyed
HICON LoadSmallIcon(HINSTANCE hInstance, LPCTSTR lpIconName)
{
	HICON hIcon = NULL;				  // icon handle 
	int index = -(int)lpIconName;
	TCHAR filename[MAX_PATH] = {0};
	GetModuleFileName(hInstance,filename,MAX_PATH);
 	ExtractIconEx(filename,index,NULL,&hIcon,1);
	return hIcon;
}

// load small icon from hInstance
HICON LoadIconEx(HINSTANCE hInstance, LPCTSTR lpIconName, BOOL bShared)
{
    HICON hResIcon = bShared?LoadSmallIcon(hInstance,lpIconName):LoadSmallIconShared(hInstance,lpIconName);
    if ( !hResIcon) //Icon not found in hInstance lets try to load it from core
    {
        HINSTANCE hCoreInstance = GetModuleHandle(NULL);
        if (hCoreInstance != hInstance)
            hResIcon = bShared?LoadSmallIcon(hInstance,lpIconName):LoadSmallIconShared(hInstance,lpIconName);
    }
    return hResIcon;
}

BOOL DestroyIcon_protect(HICON icon)
{
	if (icon) return DestroyIcon(icon);
	return FALSE;
}

void li_ListDestruct(SortedList *pList, ItemDestuctor pItemDestructor)
{																			
	int i=0;
	if ( !pList) return;
	for (i=0; i < pList->realCount; i++)	pItemDestructor(pList->items[i]);	
	List_Destroy(pList);																											
   mir_free(pList);
}

void li_RemoveDestruct(SortedList *pList, int index, ItemDestuctor pItemDestructor)
{																																
	if (index >= 0 && index < pList->realCount)	
	{
		pItemDestructor(pList->items[index]);
		List_Remove(pList, index);
	}
}

void li_RemovePtrDestruct(SortedList *pList, void * ptr, ItemDestuctor pItemDestructor)
{																																
	if (List_RemovePtr(pList, ptr))
        pItemDestructor(ptr);
}

void li_SortList(SortedList *pList, FSortFunc pSortFunct)
{
	FSortFunc pOldSort = pList->sortFunc;
	int i;
	if ( !pSortFunct) pSortFunct = pOldSort;
	pList->sortFunc = NULL;
	for (i=0; i < pList->realCount-1; i++)
		if (pOldSort(pList->items[i],pList->items[i+1]) < 0)
		{
		    void * temp = pList->items[i];
			pList->items[i] = pList->items[i+1];
			pList->items[i+1] = temp;
			i--;
			if (i>0) i--;
		}
	pList->sortFunc = pOldSort;
}
