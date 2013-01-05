/*
Miranda-IM SMS Plugin
Copyright (C) 2001-2  Richard Hughes
Copyright (C) 2007-8  Rozhuk Ivan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
---------------------------------------------------------------------------

This was the original words.
This plugin was modified by Ariel Shulman (NuKe007).
For any comments, problems, etc. contact me at Miranda-IM forums or E-Mail or ICQ.
All the information needed you can find at www.nuke007.tk
Enjoy the code and use it smartly!
*/

#include "common.h"




/*
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



*/

int hLangpack;
HINSTANCE hInst;

SMS_SETTINGS ssSMSSettings;


PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {70AC2AC9-85C6-4624-9B05-24733FEBB052}
	SMS_GUID
};


extern "C" __declspec(dllexport) const MUUID interfaces[] = {SMS_GUID,MIID_LAST};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

int		OnModulesLoaded		(WPARAM wParam,LPARAM lParam);
int		OnPreShutdown		(WPARAM wParam,LPARAM lParam);
void	VersionConversions();

BOOL WINAPI DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID lpvReserved)
{
    hInst = hInstance;

	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		ZeroMemory(&ssSMSSettings,sizeof(ssSMSSettings));
		ssSMSSettings.hInstance=hInstance;
		ssSMSSettings.hHeap=HeapCreate(0,0,0);//GetProcessHeap();
		DisableThreadLibraryCalls((HMODULE)hInstance);
		break;
	case DLL_PROCESS_DETACH:
		HeapDestroy(ssSMSSettings.hHeap);
		ssSMSSettings.hHeap=NULL;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
    }

	return TRUE;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	ssSMSSettings.hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,OnModulesLoaded);
	ssSMSSettings.hHookPreShutdown=HookEvent(ME_SYSTEM_PRESHUTDOWN,OnPreShutdown);

	SendSMSWindowInitialize();
	RecvSMSWindowInitialize();

	LoadServices();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnloadServices();

	if (ssSMSSettings.hHookPreShutdown)		{UnhookEvent(ssSMSSettings.hHookPreShutdown);	ssSMSSettings.hHookPreShutdown=NULL;}
	if (ssSMSSettings.hHookModulesLoaded)	{UnhookEvent(ssSMSSettings.hHookModulesLoaded);	ssSMSSettings.hHookModulesLoaded=NULL;}

	return 0;
}



int OnModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	VersionConversions();

	LoadModules();

	return 0;
}


int OnPreShutdown(WPARAM wParam,LPARAM lParam)
{
	UnloadModules();

	RecvSMSWindowDestroy();
	SendSMSWindowDestroy();
	FreeAccountList();

	return 0;
}



void VersionConversions()
{
	WCHAR wsztm[MAX_PATH];

	if (DB_SMS_GetStaticStringW(NULL,"UseSignature",wsztm,SIZEOF(wsztm),NULL))
	{
		DB_SMS_SetByte(NULL,"UseSignature",(wsztm[0]=='0'));
	}else{
		DB_SMS_SetByte(NULL,"UseSignature",SMS_DEFAULT_USESIGNATURE);
	}

	if (DB_SMS_GetStaticStringW(NULL,"SignaturePos",wsztm,SIZEOF(wsztm),NULL))
	{
		DB_SMS_SetByte(NULL,"SignaturePos",(wsztm[0]=='0'));
	}else{
		DB_SMS_SetByte(NULL,"SignaturePos",SMS_DEFAULT_SIGNATUREPOS);
	}

	if (DB_SMS_GetStaticStringW(NULL,"ShowACK",wsztm,SIZEOF(wsztm),NULL))
	{
		DB_SMS_SetByte(NULL,"ShowACK",(wsztm[0]=='0'));
	}else{
		DB_SMS_SetByte(NULL,"ShowACK",SMS_DEFAULT_SHOWACK);
	}

}