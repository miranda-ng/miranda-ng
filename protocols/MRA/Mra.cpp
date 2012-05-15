#include "Mra.h"






#ifndef  _WIN64

/*extern "C" __declspec(naked) void _chkstk()
{
	#define _PAGESIZE_ 4096

			__asm
			{
			push    ecx

	; Calculate new TOS.

			lea     ecx, [esp] + 8 - 4      ; TOS before entering function + size for ret value
			sub     ecx, eax                ; new TOS

	; Handle allocation size that results in wraparound.
	; Wraparound will result in StackOverflow exception.

			sbb     eax, eax                ; 0 if CF==0, ~0 if CF==1
			not     eax                     ; ~0 if TOS did not wrapped around, 0 otherwise
			and     ecx, eax                ; set to 0 if wraparound

			mov     eax, esp                ; current TOS
			and     eax, not ( _PAGESIZE_ - 1) ; Round down to current page boundary

	cs10:
			cmp     ecx, eax                ; Is new TOS
			jb      short cs20              ; in probed page?
			mov     eax, ecx                ; yes.
			pop     ecx
			xchg    esp, eax                ; update esp
			mov     eax, dword ptr [eax]    ; get return address
			mov     dword ptr [esp], eax    ; and put it at new TOS
			ret

	; Find next lower page and probe
	cs20:
			sub     eax, _PAGESIZE_         ; decrease by PAGESIZE
			test    dword ptr [eax],eax     ; probe page.
			jmp     short cs10

			}
}//*/




/*extern "C" __declspec(naked) void __cdecl _chkstk()
{ 
    _asm {    jz    esp_okay    }; 
    _asm {    int    3    }; 
esp_okay: 
    _asm {    ret    }; 
} //*/






 /*//_alloca_probe_16 : 16 byte aligned alloca
 extern "C" void _alloca_probe_16()
 {
 	__asm 
 	{
 		push    ecx
 		lea     ecx, [esp] + 8          ; TOS before entering this function
 		sub     ecx, eax                ; New TOS
 		and     ecx, (16 - 1)           ; Distance from 16 bit align (align down)
 		add     eax, ecx                ; Increase allocation size
 		sbb     ecx, ecx                ; ecx = 0xFFFFFFFF if size wrapped around
 		or      eax, ecx                ; cap allocation size on wraparound
 		pop     ecx                     ; Restore ecx
 		jmp     _chkstk
 	}
 }
 
 //alloca_8: 8 byte aligned alloca
 extern "C" void _alloca_probe_8()
 {
 	__asm 
 	{
 		push    ecx
 		lea     ecx, [esp] + 8          ; TOS before entering this function
 		sub     ecx, eax                ; New TOS
 		and     ecx, (8 - 1)            ; Distance from 8 bit align (align down)
 		add     eax, ecx                ; Increase allocation Size
 		sbb     ecx, ecx                ; ecx = 0xFFFFFFFF if size wrapped around
 		or      eax, ecx                ; cap allocation size on wraparound
 		pop     ecx                     ; Restore ecx
 		jmp     _chkstk
 	}
 }//*/

/*extern "C" void __cdecl _chkstk()
{ 
	__asm
	{ 
		push	ebp 
		sub		eax, 4 
		xchg	[esp+4], ebp 
		sub		esp, eax 
		xchg	[esp], ebp 
		mov		ebp, esp 
		add		ebp, eax 
		mov		ebp, [ebp] 
	} 
}//*/



/*#define _PAGESIZE_ 1000h

#if !defined (__MARM__)
extern "C" void __declspec(naked) _chkstk(void)
{
	_asm 
	{
		push ecx 
		cmp eax,_PAGESIZE_
		lea ecx,[esp] + 8
		jb short lastpage

		probepages:
		sub ecx,_PAGESIZE_
		sub eax,_PAGESIZE_

		test dword ptr [ecx],eax

		cmp eax,_PAGESIZE_
		jae short probepages

		lastpage:
		sub ecx,eax
		mov eax,esp

		test dword ptr [ecx],eax

		mov esp,ecx

		mov ecx,dword ptr [eax]
		mov eax,dword ptr [eax + 4]

		push eax
		ret
	}
}
#endif**/



/*// будем тока выдел€ть место в стеке под переменные
extern "C" __declspec(naked) void _chkstk()
{
	__asm
	{
		// Enter: EAX = size

		xor ebx, ebx
		xchg [esp], ebx  // запоминаем адрес возврата
		add esp, 4  // удал€ем его
		sub esp, eax  //  выдел€ем место
		push ebx  // кладем адрес возврата на место и возвращаемс€
		retn
	}
}//*/



//extern "C" void __declspec(naked) __cdecl _chkesp()
/*extern "C" __declspec(naked)  void __cdecl _chkstk()
{
#ifndef _PAGESIZE_
    #define _PAGESIZE_ 1000h
#endif //_PAGESIZE_

    __asm
    {
        push    ecx                     // save ecx
        cmp     eax, _PAGESIZE_         // more than one page requested?
        lea     ecx, [esp] + 8          // compute new stack pointer in ecx
                                        // correct for return address and
                                        // saved ecx
        jb      short lastpage          // no


probepages:

        sub     ecx, _PAGESIZE_         // yes, move down a page
        sub     eax, _PAGESIZE_         // adjust request and...

        test    dword ptr [ecx], eax    // ...probe it

        cmp     eax, _PAGESIZE_         // more than one page requested?
        jae     short probepages        // no


lastpage:

        sub     ecx, eax                // move stack down by eax
        mov     eax, esp                // save current tos and do a...

        test    dword ptr [ecx], eax    // ...probe in case a page was crossed

        mov     esp, ecx                // set the new stack pointer

        mov     ecx, dword ptr [eax]    // recover ecx
        mov     eax, dword ptr [eax + 4]// recover return address

        push    eax                     // prepare return address
                                        // ...probe in case a page was crossed
        ret
    }
}//*/






extern "C" __declspec(naked) void __cdecl _chkstk()
{
	#define _PAGESIZE_ 4096

			__asm
			{
			push    ecx

	; Calculate new TOS.

			lea     ecx, [esp] + 8 - 4      ; TOS before entering function + size for ret value
			sub     ecx, eax                ; new TOS

	; Handle allocation size that results in wraparound.
	; Wraparound will result in StackOverflow exception.

			sbb     eax, eax                ; 0 if CF==0, ~0 if CF==1
			not     eax                     ; ~0 if TOS did not wrapped around, 0 otherwise
			and     ecx, eax                ; set to 0 if wraparound

			mov     eax, esp                ; current TOS
			and     eax, not ( _PAGESIZE_ - 1) ; Round down to current page boundary

	cs10:
			cmp     ecx, eax                ; Is new TOS
			jb      short cs20              ; in probed page?
			mov     eax, ecx                ; yes.
			pop     ecx
			xchg    esp, eax                ; update esp
			mov     eax, dword ptr [eax]    ; get return address
			mov     dword ptr [esp], eax    ; and put it at new TOS
			ret

	; Find next lower page and probe
	cs20:
			sub     eax, _PAGESIZE_         ; decrease by PAGESIZE
			test    dword ptr [eax],eax     ; probe page.
			jmp     short cs10

			}
}//


extern "C" void __declspec(naked) __cdecl _aulldiv()
{// http://tamiaode.3322.org/svn/ntldr/trunk/source/ntldr/ia32/x86stub.cpp
	__asm
	{
		push	ebx
		push	esi

		mov	eax,[esp + 24]
		or	eax,eax
		jnz	short L1

		mov	ecx,[esp + 20]
		mov	eax,[esp + 16]
		xor	edx,edx
		div	ecx
		mov	ebx,eax
		mov	eax,[esp + 12]
		div	ecx
		mov	edx,ebx
		jmp	short L2

	L1:
		mov	ecx,eax
		mov	ebx,[esp + 20]
		mov	edx,[esp + 14]
		mov	eax,[esp + 12]

	L3:
		shr	ecx,1
		rcr	ebx,1
		shr	edx,1
		rcr	eax,1
		or	ecx,ecx
		jnz	short L3
		div	ebx
		mov	esi,eax

		mul	dword ptr [esp + 24]
		mov	ecx,eax
		mov	eax,[esp + 20]
		mul	esi
		add	edx,ecx
		jc	short L4

		cmp	edx,[esp + 16]
		ja	short L4
		jb	short L5
		cmp	eax,[esp + 12]
		jbe	short L5
	L4:
		dec	esi
	L5:
		xor	edx,edx
		mov	eax,esi

	L2:

		pop	esi
		pop	ebx

		ret	16
	}
}//

//
// LONG64 mul
//
extern "C" void __declspec(naked) __cdecl _allmul()
{
	__asm
	{
		mov	eax,[esp + 4][4]
		mov	ecx,[esp + 12][4]
		or	ecx,eax
		mov	ecx,[esp + 12]
		jnz	short hard

		mov	eax,[esp + 4]
		mul	ecx
		ret	16

hard:
		push	ebx
		mul	ecx
		mov	ebx,eax
		mov	eax,[esp + 8]
		mul	dword ptr [esp + 16][4]
		add	ebx,eax
		mov	eax,[esp + 8]
		mul	ecx
		add	edx,ebx
		pop	ebx

		ret	16
	}
}//*/


//
// 64bits shift left
//
extern "C" __declspec(naked) void __cdecl _allshl()
{
	__asm
	{
		cmp	cl, 64
		jae	short RETZERO

		cmp	cl, 32
		jae	short MORE32
		shld	edx,eax,cl
		shl	eax,cl
		ret

	MORE32:
		mov	edx,eax
		xor	eax,eax
		and	cl,31
		shl	edx,cl
		ret

	RETZERO:
		xor	eax,eax
		xor	edx,edx
		ret
	}
}
#endif




PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
MRA_SETTINGS masMraSettings;
int hLangpack;

PLUGININFOEX pluginInfoEx={
	sizeof(PLUGININFOEX),
	PROTOCOL_DISPLAY_NAME_ORIGA,
	PLUGIN_VERSION_DWORD,
	"Provides support for Mail.ru agent Instant Messenger protocol.",
	"Rozhuk Ivan",
	"Rozhuk_I@mail.ru",
	"© 2006-2011 Rozhuk Ivan",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=2544",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
	// {E7C48BAB-8ACE-4CB3-8446-D4B73481F497}
	{ 0xe7c48bab, 0x8ace, 0x4cb3, { 0x84, 0x46, 0xd4, 0xb7, 0x34, 0x81, 0xf4, 0x97 } }
};

static const MUUID interfaces[]={MIID_PROTOCOL,MIID_LAST};


int		OnModulesLoaded		(WPARAM wParam,LPARAM lParam);
int		OnPreShutdown		(WPARAM wParam,LPARAM lParam);
void	VersionConversions	();






BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID Reserved)
{
    switch(dwReason){
	case DLL_PROCESS_ATTACH:
		memset(&masMraSettings, 0, sizeof(masMraSettings));
		masMraSettings.hInstance=hInstance;
		masMraSettings.hHeap=HeapCreate(0,0,0);//GetProcessHeap();
		masMraSettings.bLoggedIn=FALSE;
		masMraSettings.dwStatusMode=ID_STATUS_OFFLINE;
		masMraSettings.dwDesiredStatusMode=ID_STATUS_OFFLINE;
		DisableThreadLibraryCalls((HMODULE)hInstance);
		break;
	case DLL_PROCESS_DETACH:
		HeapDestroy(masMraSettings.hHeap);
		masMraSettings.hHeap=NULL;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
    }

	/*BYTE szBuff[1024]={0};
	LPSTR lpszString="\r\n\t  GET \t  \r\n  \r\n  \r\n    http://mail.ru/  \tHTTP/1.1  \t  ";
	LPSTR lpszFindString="123456  ";
	LPSTR lpszWhatFindString="123456";
	SIZE_T dwStringLen=lstrlenA(lpszString),dwBuffSize,dwBuffLen;

//#define LWSHT "\r\n\t"
//#define LWSSP "\r\n "


	//lpszString=(LPSTR)MemoryFind(0,lpszFindString,(lstrlenA(lpszFindString)),lpszWhatFindString,lstrlenA(lpszWhatFindString));
	//lpszString=(LPSTR)MemoryFindByte(0,lpszFindString,(lstrlenA(lpszFindString)-1),'6');


	//WSP2SP(lpszString,dwStringLen,(LPSTR)szBuff,&dwBuffSize);
	//HT2SP(lpszString,dwStringLen,(LPSTR)szBuff,&dwBuffSize);
	CleanUnneededSP(lpszString,dwStringLen,(LPSTR)szBuff,&dwBuffSize);

	dwBuffLen=lstrlenA((LPSTR)szBuff);
	if (dwBuffSize!=dwBuffLen) DebugBreak();
	//*/


	/*LPCSTR lpszHeader,lpszHeaderLow,lpszValueName;
	LPSTR lpszValueNameRet;
	SIZE_T dwHeaderSize,dwValueNameSize,dwValueNameRetSize;
	CHAR szHeader[4096]={0},szHeaderLow[4096]={0};

	//lstrcpynA(szHeader,"Server: squid/3.1.0.15\r\nMime-Version: 1.0\r\nContent-Length: 0\r\nX-Cache: MISS from Firewall\r\nProxy-Connection: keep-alive",SIZEOF(szHeader));
	lstrcpynA(szHeader,"http/1.0 200 ok\r\nserver: apache-coyote/1.1\r\ndate: fri, 30 apr 2010 08:36:10 gmt\r\nlast-modified: wed, 28 apr 2010 03:22:49 gmt\r\ncache-control: max-age=360000\r\ncontent-type: \r\ncontent-length: 55946",SIZEOF(szHeader));
	lpszHeader=(LPCSTR)szHeader;
	lpszHeaderLow=(LPCSTR)szHeaderLow;
	dwHeaderSize=lstrlenA(lpszHeader);
	
	//lpszValueName="proxy-connection";
	//lpszValueName="content-length";
	lpszValueName="content-type";
	dwValueNameSize=lstrlenA(lpszValueName);


	BuffToLowerCase(lpszHeaderLow,lpszHeader,dwHeaderSize);

	HeaderValueGet(lpszHeader,lpszHeaderLow,dwHeaderSize,lpszValueName,dwValueNameSize,&lpszValueNameRet,&dwValueNameRetSize);
	//HeaderValueRemove(lpszHeader,lpszHeaderLow,dwHeaderSize,&dwBuffLen,lpszValueName,dwValueNameSize);
	szHeader[dwBuffLen]=0;
	szHeaderLow[dwBuffLen]=0;//*/


return(TRUE);
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion<MIN_MIR_VER_VERSION_DWORD) 
	{
		MessageBox(NULL, TranslateT("Pleace, update your Miranda IM, MRA will not load with this version."), NULL, (MB_OK|MB_ICONERROR));
		return(NULL);
	}
	return(&pluginInfoEx);
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces()
{
	return(interfaces);
}


extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	SIZE_T dwBuffLen;
	WCHAR szBuff[MAX_FILEPATH];
	LPSTR lpszFullFileName=(LPSTR)szBuff;
	LPWSTR lpwszFileName;
	PROTOCOLDESCRIPTOR pd={0};

	pluginLink=link;
	mir_getLP(&pluginInfoEx);
	mir_getMMI(&mmi);


	// Get module name from DLL file name
	if (GetModuleFileName(masMraSettings.hInstance,szBuff,MAX_FILEPATH))
	{
		WCHAR sztmBuff[MAX_FILEPATH];
		if ((dwBuffLen=GetFullPathName(szBuff,MAX_FILEPATH,sztmBuff,&lpwszFileName)))
		{
			dwBuffLen=(lstrlenW(lpwszFileName)-4);
			//lpwszFileName=L"MRA.dll";
			//dwBuffLen=3;
			masMraSettings.dwModuleNameLen=(dwBuffLen<MAXMODULELABELLENGTH)? dwBuffLen:(MAXMODULELABELLENGTH-1);
			memmove(masMraSettings.wszModuleName,lpwszFileName,(masMraSettings.dwModuleNameLen*sizeof(WCHAR)));
			CharUpperBuff(masMraSettings.wszModuleName,masMraSettings.dwModuleNameLen);
			masMraSettings.wszModuleName[masMraSettings.dwModuleNameLen]=0;
			WideCharToMultiByte(MRA_CODE_PAGE,0,masMraSettings.wszModuleName,(masMraSettings.dwModuleNameLen+1),masMraSettings.szModuleName,MAXMODULELABELLENGTH,NULL,NULL);

			if (DB_Mra_GetByte(NULL,"UseDisplayModuleName",TRUE))
			{
				masMraSettings.dwDisplayModuleNameLen=mir_sntprintf(masMraSettings.wszDisplayModuleName,SIZEOF(masMraSettings.wszDisplayModuleName),L"%s: %S",PROTOCOL_NAMEW,PROTOCOL_DISPLAY_NAME_ORIGA);
			}else{
				memmove(masMraSettings.wszDisplayModuleName,masMraSettings.wszModuleName,(masMraSettings.dwModuleNameLen*sizeof(WCHAR)));
				masMraSettings.dwDisplayModuleNameLen=masMraSettings.dwModuleNameLen;
				masMraSettings.szDisplayModuleName[masMraSettings.dwDisplayModuleNameLen]=0;
			}
			WideCharToMultiByte(MRA_CODE_PAGE,0,masMraSettings.wszDisplayModuleName,(masMraSettings.dwDisplayModuleNameLen+1),masMraSettings.szDisplayModuleName,MAX_PATH,NULL,NULL);
		}
	}

	// load libs
	if (GetModuleFileName(NULL,szBuff,MAX_FILEPATH))
	{// 
		DWORD dwErrorCode;

		masMraSettings.dwMirWorkDirPathLen=GetFullPathName(szBuff,MAX_FILEPATH,masMraSettings.szMirWorkDirPath,&lpwszFileName);
		if (masMraSettings.dwMirWorkDirPathLen)
		{
			masMraSettings.dwMirWorkDirPathLen-=lstrlenW(lpwszFileName);
			masMraSettings.szMirWorkDirPath[masMraSettings.dwMirWorkDirPathLen]=0;

			// load xstatus icons lib
			dwErrorCode=FindFile(masMraSettings.szMirWorkDirPath,masMraSettings.dwMirWorkDirPathLen,L"xstatus_MRA.dll",-1,szBuff,SIZEOF(szBuff),(DWORD*)&dwBuffLen);
			if (dwErrorCode==NO_ERROR)
			{
				masMraSettings.hDLLXStatusIcons=LoadLibraryEx(szBuff,NULL,/*LOAD_LIBRARY_AS_DATAFILE*/0);
				if (masMraSettings.hDLLXStatusIcons)
				{
					if ((dwBuffLen=LoadStringW(masMraSettings.hDLLXStatusIcons,IDS_IDENTIFY,szBuff,MAX_FILEPATH))==0 || CompareStringW(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,L"# Custom Status Icons #",23,szBuff,dwBuffLen)!=CSTR_EQUAL)
					{
						FreeLibrary(masMraSettings.hDLLXStatusIcons);
						masMraSettings.hDLLXStatusIcons=NULL;
					}
				}
			}

			// load zlib
			dwErrorCode=FindFile(masMraSettings.szMirWorkDirPath,masMraSettings.dwMirWorkDirPathLen,L"zlib.dll",-1,szBuff,SIZEOF(szBuff),(DWORD*)&dwBuffLen);
			if (dwErrorCode!=NO_ERROR) dwErrorCode=FindFile(masMraSettings.szMirWorkDirPath,masMraSettings.dwMirWorkDirPathLen,L"zlib1.dll",-1,szBuff,SIZEOF(szBuff),(DWORD*)&dwBuffLen);
			if (dwErrorCode==NO_ERROR)
			{// load xstatus icons lib
				masMraSettings.hDLLZLib=LoadLibraryEx(szBuff,NULL,0);
				if (masMraSettings.hDLLZLib)
				{
					masMraSettings.lpfnCompress2=(HANDLE)GetProcAddress(masMraSettings.hDLLZLib,"compress2");
					masMraSettings.lpfnUncompress=(HANDLE)GetProcAddress(masMraSettings.hDLLZLib,"uncompress");
				}
			}
		}
	}


	/*HMODULE hDLLCrypt32=LoadLibraryW(L"Crypt32.dll");
	if (hDLLCrypt32)
	{
		HANDLE hFunc=(HANDLE)GetProcAddress(hDLLCrypt32,"CryptProtectMemory");
		FreeLibrary(hDLLCrypt32);
	}

	HMODULE hDLLAdvapi32=LoadLibraryW(L"Advapi32.dll");
	if (hDLLAdvapi32)
	{
		HANDLE hFunc=(HANDLE)GetProcAddress(hDLLAdvapi32,"SystemFunction040");
		FreeLibrary(hDLLAdvapi32);
	}

	for(DWORD i=0;i<MRA_XSTATUS_COUNT;i++)
	{
		if (lstrlenA(lpcszXStatusNameDef[i])>STATUS_TITLE_MAX) 
		{
			DebugPrintCRLF(lpcszXStatusNameDef[i]);
		}
		if (lstrlenA(TranslateW(lpcszXStatusNameDef[i]))>STATUS_TITLE_MAX) 
		{
			DebugPrintCRLF(TranslateW(lpcszXStatusNameDef[i]));
		}
	}*/



	InitializeCriticalSectionAndSpinCount(&masMraSettings.csCriticalSectionSend,0);
	MraSendQueueInitialize(0,&masMraSettings.hSendQueueHandle);
	MraFilesQueueInitialize(0,&masMraSettings.hFilesQueueHandle);
	MraMPopSessionQueueInitialize(&masMraSettings.hMPopSessionQueue);

	FifoMTInitialize(&masMraSettings.ffmtAPCQueue,0);
	masMraSettings.hWaitEventThreadAPCHandle=CreateEvent(NULL,FALSE,FALSE,NULL);
	InterlockedExchange((volatile LONG*)&masMraSettings.dwAPCThreadRunning,TRUE);
	masMraSettings.hThreadAPC=(HANDLE)mir_forkthread((pThreadFunc)MraUserAPCThreadProc,NULL);
	if (masMraSettings.hThreadAPC==NULL)
	{
		MraAPCQueueDestroy(&masMraSettings.ffmtAPCQueue);
		CloseHandle(masMraSettings.hWaitEventThreadAPCHandle);
		masMraSettings.hWaitEventThreadAPCHandle=NULL;
		DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&masMraSettings.hThreadAPC,THREAD_SET_CONTEXT,FALSE,0);
		DebugPrintCRLFW(L"Fail on create event APC thread, using miranda main thread");
		MessageBoxW(NULL,L"Fail on create event APC thread, using miranda main thread",PROTOCOL_DISPLAY_NAMEW,(MB_OK|MB_ICONINFORMATION));
	}//*/

	masMraSettings.hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);
	masMraSettings.hHookPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN,OnPreShutdown);

	LoadServices();

	//pd.cbSize=sizeof(pd);
	pd.cbSize=PROTOCOLDESCRIPTOR_V3_SIZE;
	pd.szName=PROTOCOL_NAMEA;
	pd.type=PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	DebugPrintCRLFW(L"Load - DONE");
return(0);
}


extern "C" __declspec(dllexport) int Unload(void)
{
	UnloadServices();
	//if (hHookOnUserInfoInit)				UnhookEvent(hHookOnUserInfoInit);
	if (masMraSettings.hHookPreShutdown)	{UnhookEvent(masMraSettings.hHookPreShutdown);		masMraSettings.hHookPreShutdown=NULL;}
	if (masMraSettings.hHookModulesLoaded)	{UnhookEvent(masMraSettings.hHookModulesLoaded);	masMraSettings.hHookModulesLoaded=NULL;}

	Netlib_CloseHandle(masMraSettings.hNetlibUser);
	masMraSettings.hNetlibUser=NULL;

	MraMPopSessionQueueDestroy(masMraSettings.hMPopSessionQueue);
	masMraSettings.hMPopSessionQueue=NULL;

	MraFilesQueueDestroy(masMraSettings.hFilesQueueHandle);
	masMraSettings.hFilesQueueHandle=NULL;

	MraSendQueueDestroy(masMraSettings.hSendQueueHandle);
	masMraSettings.hSendQueueHandle=NULL;

	DeleteCriticalSection(&masMraSettings.csCriticalSectionSend);
	SecureZeroMemory(&masMraSettings.csCriticalSectionSend,sizeof(CRITICAL_SECTION));

	if (masMraSettings.hDLLXStatusIcons)
	{
		FreeLibrary(masMraSettings.hDLLXStatusIcons);
		masMraSettings.hDLLXStatusIcons=NULL;
	}

	if (masMraSettings.hDLLZLib)
	{
		FreeLibrary(masMraSettings.hDLLZLib);
		masMraSettings.hDLLZLib=NULL;
		masMraSettings.lpfnCompress2=NULL;
		masMraSettings.lpfnUncompress=NULL;
	}

	SecureZeroMemory(pluginLink,sizeof(pluginLink));
	SecureZeroMemory(&mmi,sizeof(pluginLink));

	DebugPrintCRLFW(L"Unload - DONE");
return(0);
}


static int OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	WCHAR szBuffer[MAX_PATH];
	NETLIBUSER nlu={0};

	mir_sntprintf(szBuffer,SIZEOF(szBuffer),L"%s %s",PROTOCOL_NAMEW,TranslateW(L"plugin connections"));
	nlu.cbSize=sizeof(nlu);
	nlu.flags=(NUF_INCOMING|NUF_OUTGOING|NUF_HTTPCONNS|NUF_UNICODE);
	nlu.szSettingsModule=PROTOCOL_NAMEA;
	nlu.ptszDescriptiveName=szBuffer;
	masMraSettings.hNetlibUser=(HANDLE)CallService(MS_NETLIB_REGISTERUSER,0,(LPARAM)&nlu);

#ifdef _DEBUG
	// всех в offline // тк unsaved values сохран€ютс€ их нужно инициализировать
	for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{// функци€ сама провер€ет принадлежность контакта к MRA
		SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS),-1,-1,0,0,ID_STATUS_OFFLINE,NULL,0,NULL,0,NULL,0);
	}
#else
	// unsaved values
	DB_Mra_CreateResidentSetting("Status");// NOTE: XStatus cannot be temporary
	DB_Mra_CreateResidentSetting("LogonTS");
	DB_Mra_CreateResidentSetting("ContactID");
	DB_Mra_CreateResidentSetting("GroupID");
	DB_Mra_CreateResidentSetting("ContactFlags");
	DB_Mra_CreateResidentSetting("ContactSeverFlags");
	DB_Mra_CreateResidentSetting("HooksLocked");
	DB_Mra_CreateResidentSetting(DBSETTING_CAPABILITIES);
	DB_Mra_CreateResidentSetting(DBSETTING_XSTATUSNAME);
	DB_Mra_CreateResidentSetting(DBSETTING_XSTATUSMSG);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUSTIME);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUSID);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUS);
	DB_Mra_CreateResidentSetting(DBSETTING_BLOGSTATUSMUSIC);
	
	// всех в offline // тк unsaved values сохран€ютс€ их нужно инициализировать
	for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
	{// функци€ сама провер€ет принадлежность контакта к MRA
		SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS),-1,-1,0,0,ID_STATUS_OFFLINE,NULL,0,NULL,0,NULL,0);
	}
#endif

	MraAvatarsQueueInitialize(&masMraSettings.hAvatarsQueueHandle);
	/*{
		HANDLE hContact;
		for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{// функци€ сама провер€ет принадлежность контакта к MRA
			if (IsContactMra(hContact)) break;
		}
		DB_Mra_DeleteValue(hContact,"AvatarLastCheckTime");
		MraAvatarsQueueGetAvatar(masMraSettings.hAvatarsQueueHandle,GAIF_FORCE,hContact,NULL,NULL,NULL);
	}//*/
	
	VersionConversions();

	MraAntiSpamLoadBadWordsW();

	LoadModules();

	CallService(MS_UPDATE_REGISTERFL,(WPARAM)MRA_PLUGIN_UPDATER_ID,(LPARAM)&pluginInfoEx);


	//hHookOnUserInfoInit=HookEvent(ME_USERINFO_INITIALISE,MsnOnDetailsInit);


	/*//BYTE btRND[MAX_PATH]={0};
	char szPass[MAX_PATH]={0};
	//LPSTR lpszPass="1234567890";
	SIZE_T dwBuffLen;

	//dwBuffLen=lstrlenA(lpszPass);
	//SetPassDB(lpszPass,dwBuffLen);
	GetPassDB(szPass,sizeof(szPass),&dwBuffLen);

	MessageBoxA(NULL,szPass,"Password",(MB_OK));

	//*/


	InterlockedExchange((volatile LONG*)&masMraSettings.dwGlobalPluginRunning,TRUE);

	//MraSetStatus(ID_STATUS_OFFLINE,0);

	DebugPrintCRLFW(L"OnModulesLoaded - DONE");
return(0);
}


int OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	InterlockedExchange((volatile LONG*)&masMraSettings.dwGlobalPluginRunning,FALSE);

	MraSetStatus(ID_STATUS_OFFLINE,0);
	
	MraAvatarsQueueDestroy(masMraSettings.hAvatarsQueueHandle);
	masMraSettings.hAvatarsQueueHandle=NULL;

	if (masMraSettings.hThreadWorker)
	{
		if (IsThreadAlive(masMraSettings.hThreadWorker)) WaitForSingleObjectEx(masMraSettings.hThreadWorker,(WAIT_FOR_THREAD_TIMEOUT*1000),FALSE);
		masMraSettings.hThreadWorker=NULL;
	}

	InterlockedExchange((volatile LONG*)&masMraSettings.dwAPCThreadRunning,FALSE);
	if (masMraSettings.hWaitEventThreadAPCHandle)
	if (IsThreadAlive(masMraSettings.hThreadAPC))
	{
		SetEvent(masMraSettings.hWaitEventThreadAPCHandle);
	}else{
		CloseHandle(masMraSettings.hWaitEventThreadAPCHandle);
		//WaitForSingleObjectEx(masMraSettings.hThreadAPC,(WAIT_FOR_THREAD_TIMEOUT*1000),FALSE); // no wait - dead lock.
		masMraSettings.hWaitEventThreadAPCHandle=NULL;
		masMraSettings.hThreadAPC=NULL;
	}

	UnloadModules();

	MraAntiSpamFreeBadWords();

	DebugPrintCRLFW(L"OnPreShutdown - DONE");
return(0);
}



void VersionConversions()
{
	/*switch(DB_Mra_GetDword(NULL,"LastPluginVersion",PLUGIN_MAKE_VERSION(0,0,0,0))){
	default:
	case PLUGIN_MAKE_VERSION(1,14,0,0):
	case PLUGIN_MAKE_VERSION(1,14,0,1):
	case PLUGIN_MAKE_VERSION(1,14,0,2):
		DB_Mra_DeleteValue(NULL,"MirVer");
		DB_Mra_DeleteValue(NULL,"MessagesCountSend");
		DB_Mra_DeleteValue(NULL,"MRAClient");
		DB_Mra_DeleteValue(NULL,"MRAClientVer");
		DB_Mra_DeleteValue(NULL,"MRAClientCapabilities");
		DB_Mra_DeleteValue(NULL,"Status");
		DB_Mra_DeleteValue(NULL,"ContactID");
		DB_Mra_DeleteValue(NULL,"GroupID");
		DB_Mra_DeleteValue(NULL,"ContactSeverFlags");
		DB_Mra_DeleteValue(NULL,"HooksLocked");
		for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			DB_Mra_DeleteValue(hContact,"MessagesCountSend");
			DB_Mra_DeleteValue(hContact,"MRAClient");
			DB_Mra_DeleteValue(hContact,"MRAClientVer");
			DB_Mra_DeleteValue(hContact,"MRAClientCapabilities");
			DB_Mra_DeleteValue(hContact,"Status");
			DB_Mra_DeleteValue(hContact,"ContactID");
			DB_Mra_DeleteValue(hContact,"GroupID");
			DB_Mra_DeleteValue(hContact,"ContactSeverFlags");
			DB_Mra_DeleteValue(hContact,"HooksLocked");
		}
	case PLUGIN_MAKE_VERSION(1,14,0,3):
	case PLUGIN_MAKE_VERSION(1,14,0,4):
	case PLUGIN_MAKE_VERSION(1,14,0,5):
	case PLUGIN_MAKE_VERSION(1,14,0,6):
	case PLUGIN_MAKE_VERSION(1,14,0,7):
	//case PLUGIN_MAKE_VERSION(1,14,1,0):
		DB_Mra_SetByte(NULL,DBSETTING_XSTATUSID,(BYTE)DB_Mra_GetDword(NULL,DBSETTING_XSTATUSID,MRA_MIR_XSTATUS_NONE));
		for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactMra(hContact)) DB_Mra_SetByte(hContact,DBSETTING_XSTATUSID,(BYTE)DB_Mra_GetDword(hContact,DBSETTING_XSTATUSID,MRA_MIR_XSTATUS_NONE));
		}
	case PLUGIN_MAKE_VERSION(1,16,0,2):
		DB_Mra_DeleteValue(NULL,"CvtSmilesToTags");
	case PLUGIN_MAKE_VERSION(1,16,0,3):
		for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactMra(hContact)==FALSE) DB_Mra_DeleteValue(hContact,DBSETTING_XSTATUSID);
		}
	case PLUGIN_MAKE_VERSION(0,0,0,0):// no previos version
		MraAntiSpamResetBadWordsList();
	case PLUGIN_VERSION_DWORD:// current version
		break;
	}*/

	DB_Mra_SetDword(NULL,"LastPluginVersion",PLUGIN_VERSION_DWORD);



	{// pass conv
		char szPass[MAX_PATH];
		SIZE_T dwPassSize;

		if(DB_Mra_GetStaticStringA(NULL,"Password",(LPSTR)szPass,SIZEOF(szPass),&dwPassSize))
		{//bit of a security hole here, since it's easy to extract a password from an edit box
			CallService(MS_DB_CRYPT_DECODESTRING,(dwPassSize+1),(LPARAM)szPass);

			SetPassDB(szPass,dwPassSize);

			SecureZeroMemory(szPass,sizeof(szPass));
			DB_Mra_DeleteValue(NULL,"Password");
		}
	}

//*/
}


