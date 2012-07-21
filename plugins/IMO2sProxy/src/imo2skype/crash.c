#include <imagehlp.h>
#include <malloc.h>
#pragma comment (lib, "imagehlp.lib")

#define MAXNAMELEN 1024
#define HEXDUMP_BREAK	16

void HexDump(char *pszOutput, PBYTE pData, int cbLength, BOOL bShow)
{
	char *pszLastLine;
	int i;

	*pszOutput=0;
	pszLastLine = pszOutput;
	for (i=0; i<cbLength; i++)
	{
		if (i%HEXDUMP_BREAK == 0)
		{
			if (i>=HEXDUMP_BREAK)
			{
				int j;

				strcat (pszOutput, "    ");
				for (j=i-HEXDUMP_BREAK; j<i; j++) sprintf (pszOutput+strlen(pszOutput), "%c", pData[j]);
			}
			sprintf (pszOutput+strlen(pszOutput), "\r\n");
			if (bShow) LOG (("\t%s", pszLastLine));
			if (i==0) *pszOutput=0;
			pszLastLine = pszOutput+strlen(pszOutput);
		}
		sprintf (pszOutput+strlen(pszOutput), "%02X ", pData[i]);
		if (i == cbLength-1)
		{
			int j;

			for (j=0; j<HEXDUMP_BREAK - i%HEXDUMP_BREAK - 1; j++) sprintf (pszOutput+strlen(pszOutput), "   ");
			sprintf (pszOutput+strlen(pszOutput), "    ");
			for (j=i-i%HEXDUMP_BREAK; j<i; j++) sprintf (pszOutput+strlen(pszOutput), "%c", 
				(isprint(pData[j]) && pData[j]!=0x0D && pData[j]!=0x0A && pData[j]!=0x09)?pData[j]:'.');
			if (bShow) LOG (("\t%s", pszLastLine));
		}
	}
}


void CrashDumpMem (DWORD dwBase, char *pszName, DWORD dwDwords)
{
	int cbLength = sizeof(DWORD) * dwDwords;
	PBYTE mem = (PBYTE)alloca (cbLength*2);
	SYSTEM_INFO sysInfo;
	SIZE_T read=0;

	dwBase &= ~3;
	
	dwBase-=cbLength; cbLength*=2;
	GetSystemInfo(&sysInfo);
	if (dwBase < (DWORD)sysInfo.lpMinimumApplicationAddress || dwBase > (DWORD)sysInfo.lpMaximumApplicationAddress)
		return;

	if (ReadProcessMemory (GetCurrentProcess(), (PVOID)dwBase, mem, cbLength, &read))
	{
		char *pszHexdump = (char *)alloca (((cbLength/HEXDUMP_BREAK)+1)*81);

		LOG (("Memory dump for %s:\n", pszName));
		HexDump (pszHexdump, mem, read, TRUE);
		LOG (("\n"));
	}
}


BOOL ShowThreadStack (HANDLE hThread, CONTEXT *c)
{
	STACKFRAME stFrame={0};
	DWORD dwSymOptions, dwFrameNum=0, dwMachine, dwOffsetFromSym=0;
	IMAGEHLP_LINE Line={0};
	IMAGEHLP_MODULE Module={0};
	HANDLE hProcess = GetCurrentProcess();
	BYTE pbSym[sizeof(IMAGEHLP_SYMBOL) + MAXNAMELEN];
	IMAGEHLP_SYMBOL *pSym = (IMAGEHLP_SYMBOL *)&pbSym;
	char szUndecName[MAXNAMELEN], szUndecFullName[MAXNAMELEN];

	/*
	if ( ! GetThreadContext( hThread, &c ) )
	{
		SetError (NULL, NULL, 0);
		LOG (("Cannot get thread context%d\n", GetLastError()));
		return FALSE;
	}
	*/
	LOG (("Please wait...\n"));
	if (!SymInitialize (hProcess, NULL, TRUE))
	{
		LOG (("Cannot initialize symbol engine (%08X)\n", GetLastError()));
		return FALSE;
	}

	dwSymOptions = SymGetOptions();
	dwSymOptions |= SYMOPT_LOAD_LINES;
	dwSymOptions &= ~SYMOPT_UNDNAME;
	SymSetOptions (dwSymOptions);

	stFrame.AddrPC.Mode		 = AddrModeFlat;
	dwMachine		             = IMAGE_FILE_MACHINE_I386;
	stFrame.AddrPC.Offset    = c->Eip;
	stFrame.AddrStack.Offset = c->Esp;
	stFrame.AddrStack.Mode   = AddrModeFlat;
	stFrame.AddrFrame.Offset = c->Ebp;
	stFrame.AddrFrame.Mode   = AddrModeFlat;

	Module.SizeOfStruct = sizeof(Module);
	Line.SizeOfStruct = sizeof(Module);

	LOG (("\n--# FV EIP----- RetAddr- FramePtr StackPtr Symbol\n"));
	do
	{
		if (!StackWalk (dwMachine, hProcess, hThread, &stFrame, &c, NULL, &SymFunctionTableAccess, &SymGetModuleBase, NULL))
			break;

		LOG (( "\n%3d %c%c %08lx %08lx %08lx %08lx ",
			dwFrameNum, stFrame.Far? 'F': '.', stFrame.Virtual? 'V': '.',
			stFrame.AddrPC.Offset, stFrame.AddrReturn.Offset,
			stFrame.AddrFrame.Offset, stFrame.AddrStack.Offset ));

		if (stFrame.AddrPC.Offset == 0)
		{
			LOG (( "(-nosymbols-)\n" ));
		}
		else
		{ // we seem to have a valid PC
			if (!SymGetSymFromAddr (hProcess, stFrame.AddrPC.Offset, &dwOffsetFromSym, pSym))
			{
				if (GetLastError() != 487)
				{
					LOG (("Unable to get symbol from addr (%08X)", GetLastError()));
				}
			}
			else
			{
				UnDecorateSymbolName (pSym->Name, szUndecName, MAXNAMELEN, UNDNAME_NAME_ONLY );
				UnDecorateSymbolName (pSym->Name, szUndecFullName, MAXNAMELEN, UNDNAME_COMPLETE );
				LOG (("%s", szUndecName));
				if (dwOffsetFromSym) LOG ((" %+ld bytes", (long)dwOffsetFromSym));
				LOG (("\n    Sig:  %s\n    Decl: %s\n", pSym->Name, szUndecFullName));
			}

			if (!SymGetLineFromAddr (hProcess, stFrame.AddrPC.Offset, &dwOffsetFromSym, &Line))
			{
				if (GetLastError() != 487)
				{
					LOG (("Unable to get line from addr (%08X)", GetLastError()));
				}
			}
			else
			{
				LOG (("    Line: %s(%lu) %+ld bytes\n", Line.FileName, Line.LineNumber, dwOffsetFromSym ));
			}

			if (!SymGetModuleInfo (hProcess, stFrame.AddrPC.Offset, &Module))
			{
				if (GetLastError() != 487)
				{
					LOG (("Unable to get module info (%08X)", GetLastError()));
				}
			}
			else
			{
				char ty[80];

				switch ( Module.SymType )
				{
				case SymNone:
					strcpy( ty, "-nosymbols-" );
					break;
				case SymCoff:
					strcpy( ty, "COFF" );
					break;
				case SymCv:
					strcpy( ty, "CV" );
					break;
				case SymPdb:
					strcpy( ty, "PDB" );
					break;
				case SymExport:
					strcpy( ty, "-exported-" );
					break;
				case SymDeferred:
					strcpy( ty, "-deferred-" );
					break;
				case SymSym:
					strcpy( ty, "SYM" );
					break;
				default:
					_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
					break;
				
				}
				LOG (( "    Mod:  %s[%s], base: %08lxh\n", Module.ModuleName, Module.ImageName, Module.BaseOfImage ));
				LOG (( "    Sym:  type: %s, file: %s\n", ty, Module.LoadedImageName));
			}
		}
		dwFrameNum++;
	} while (stFrame.AddrReturn.Offset);
	SymCleanup (hProcess);
	return TRUE;
}


LONG Win32FaultHandler(struct _EXCEPTION_POINTERS *  ExInfo)
{
	char  *FaultTx;
    int    wsFault    = ExInfo->ExceptionRecord->ExceptionCode;
    PVOID  CodeAdress = ExInfo->ExceptionRecord->ExceptionAddress;
#ifndef USE_GUI
	char choice[10];
#endif

	if (//IsDebuggerPresent() || 
		ExInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
		return EXCEPTION_CONTINUE_SEARCH;

    switch(ExInfo->ExceptionRecord->ExceptionCode)
    {
      case EXCEPTION_ACCESS_VIOLATION      : FaultTx = "ACCESS VIOLATION";		break;
      case EXCEPTION_DATATYPE_MISALIGNMENT : FaultTx = "DATATYPE MISALIGNMENT"; break;
      case EXCEPTION_FLT_DIVIDE_BY_ZERO    : FaultTx = "FLT DIVIDE BY ZERO";	break;
	  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED : FaultTx = "ARRAY BOUNDS EXCEEDED";	break;
	  case EXCEPTION_FLT_DENORMAL_OPERAND  : FaultTx = "FLT DENORMAL OPERAND";	break;
	  case EXCEPTION_FLT_INEXACT_RESULT    : FaultTx = "FLT INEXACT RESULT";	break;
	  case EXCEPTION_FLT_INVALID_OPERATION : FaultTx = "FLT INVALID OPERATION";	break;
	  case EXCEPTION_FLT_OVERFLOW          : FaultTx = "FLT OVERFLOW";			break;
	  case EXCEPTION_FLT_STACK_CHECK	   : FaultTx = "FLT STACK CHECK";		break;
	  case EXCEPTION_FLT_UNDERFLOW         : FaultTx = "FLT UNDERFLOW";			break;
	  case EXCEPTION_ILLEGAL_INSTRUCTION   : FaultTx = "ILLEGAL INSTRUCTION";	break;
	  case EXCEPTION_IN_PAGE_ERROR         : FaultTx = "IN PAGE ERROR";			break;
	  case EXCEPTION_INT_DIVIDE_BY_ZERO    : FaultTx = "INT DEVIDE BY ZERO";	break;
	  case EXCEPTION_INT_OVERFLOW          : FaultTx = "INT OVERFLOW";			break;
	  case EXCEPTION_INVALID_DISPOSITION   : FaultTx = "INVALID DISPOSITION";	break;
	  case EXCEPTION_NONCONTINUABLE_EXCEPTION:FaultTx= "NONCONTINUABLE EXCEPTION"; break;
	  case EXCEPTION_PRIV_INSTRUCTION      : FaultTx = "PRIVILEGED INSTRUCTION";break;
	  case EXCEPTION_SINGLE_STEP           : FaultTx = "SINGLE STEP";			break;
	  case EXCEPTION_STACK_OVERFLOW        : FaultTx = "STACK OVERFLOW";		break;
      case DBG_CONTROL_C                   : return EXCEPTION_EXECUTE_HANDLER;
      default                              : FaultTx = "(unknown)";				break;
    }

    LOG (("****************************************************\n"));
    LOG (("*** A Program Fault occurred:                    ***\n"));
    LOG (("*** Error code %08X: %-24s***\n", wsFault, FaultTx));
    LOG (("****************************************************\n"));
    LOG (("***   Address: %08X       Flags: %08X    ***\n", (int)CodeAdress, ExInfo->ExceptionRecord->ExceptionFlags));
	LOG (("***   EAX=%08X  EBX=%08X  ECX=%08X   ***\n", ExInfo->ContextRecord->Eax, ExInfo->ContextRecord->Ebx, ExInfo->ContextRecord->Ecx));
	LOG (("***   EDX=%08X  EBP=%08X  ESI=%08X   ***\n", ExInfo->ContextRecord->Edx, ExInfo->ContextRecord->Ebp, ExInfo->ContextRecord->Esi));
	LOG (("***   EDI=%08X  ESP=%08X  EIP=%08X   ***\n", ExInfo->ContextRecord->Edi, ExInfo->ContextRecord->Esp, ExInfo->ContextRecord->Eip));
	LOG (("****************************************************\n"));
	CrashDumpMem (ExInfo->ContextRecord->Eax, "EAX", 8);
	CrashDumpMem (ExInfo->ContextRecord->Ebx, "EBX", 8);
	CrashDumpMem (ExInfo->ContextRecord->Ecx, "ECX", 8);
	CrashDumpMem (ExInfo->ContextRecord->Edx, "EDX", 8);
	CrashDumpMem (ExInfo->ContextRecord->Esi, "ESI", 8);
	CrashDumpMem (ExInfo->ContextRecord->Edi, "EDI", 8);
	CrashDumpMem (ExInfo->ContextRecord->Esp, "ESP", 32);
	CrashDumpMem (ExInfo->ContextRecord->Ebp, "EBP", 32);
	{
		OSVERSIONINFO ov={0};

		ov.dwOSVersionInfoSize=sizeof(ov);
		GetVersionEx (&ov);
		LOG (("Target platform: Microsoft Windows NT %d.%d Build %d %s\n", ov.dwMajorVersion, ov.dwMinorVersion,
			ov.dwBuildNumber, ov.szCSDVersion));
	}

    //LogStackFrames(CodeAddress, (char *)ExInfo->ContextRecord->Ebp);
	ShowThreadStack (GetCurrentThread(), ExInfo->ContextRecord);

#ifdef USE_GUI
	if (MessageBox(NULL, "A Program Fault occurred. Do you want to continue execution (not recommended)?", "CRASH!", MB_ICONERROR | MB_YESNO) == IDYES)
#else
	printf ("Do you want to continue program execution (not recommended) [y/N]? ");
	fgets(choice, sizeof(choice) - 1, stdin);

	if (*choice=='y' || *choice=='Y')
#endif
    {
       ExInfo->ContextRecord->Eip++;
       return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

void Crash_Init(void)
{
//#ifndef _DEBUG
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) Win32FaultHandler);
//#endif
}
