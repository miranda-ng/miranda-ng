#include "../commonheaders.h"
#include "gpgw.h"

BOOL isWindowsNT(void)
{
  BOOL result;
  OSVERSIONINFO ovi;

  memset(&ovi, 0, sizeof(ovi));
  ovi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
  GetVersionEx(&ovi);

  if(ovi.dwPlatformId==VER_PLATFORM_WIN32_NT) result=TRUE;
  else result=FALSE;

  return result;
}


void storeOutput(HANDLE ahandle, char **aoutput)
{
  DWORD success;
  char readbuffer[10];
  DWORD transfered;
  DWORD available;

  do
  {
    PeekNamedPipe(ahandle, NULL, 0, NULL, &available, NULL);

    if(available==0) continue;

    success=ReadFile(ahandle, readbuffer, sizeof(readbuffer), &transfered, NULL);

    if ((success)&&(transfered!=0))
      appendText(aoutput, readbuffer, transfered);
  }
  while(available>0);
}


pxResult pxExecute(char *acommandline, char *ainput, char **aoutput, LPDWORD aexitcode)
{
  BOOL success;
  STARTUPINFO startupinfo;
  SECURITY_ATTRIBUTES securityattributes;
  SECURITY_DESCRIPTOR securitydescriptor;
  PROCESS_INFORMATION processinformation;
  HANDLE  newstdin, newstdout, readstdout, writestdin;
  char *inputpos;
  DWORD transfered;
  int size;

  LogMessage("commandline:\n", acommandline, "\n");

  memset(&securityattributes, 0, sizeof(securityattributes));
  securityattributes.nLength=sizeof(SECURITY_ATTRIBUTES);
  securityattributes.bInheritHandle=TRUE;

  if(isWindowsNT())
  {
    InitializeSecurityDescriptor(&securitydescriptor, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&securitydescriptor, TRUE, NULL, FALSE);
    securityattributes.lpSecurityDescriptor=&securitydescriptor;
  }
  else securityattributes.lpSecurityDescriptor=NULL;

  success=CreatePipe(&newstdin, &writestdin ,&securityattributes ,0);
  if (! success)
  {
    LogMessage("--- ", "create pipe failed", "\n");
    return pxCreatePipeFailed;
  }

  success=CreatePipe(&readstdout, &newstdout, &securityattributes, 0);
  if (! success)
  {
    LogMessage("--- ", "create pipe failed", "\n");
    CloseHandle(newstdin);
    CloseHandle(writestdin);
    return pxCreatePipeFailed;
  }

  GetStartupInfo(&startupinfo);
  startupinfo.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  startupinfo.wShowWindow=SW_HIDE;
  startupinfo.hStdOutput=newstdout;
  startupinfo.hStdError=newstdout;
  startupinfo.hStdInput=newstdin;

  success=CreateProcess(NULL, acommandline, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupinfo, &processinformation);

  if (! success)
  {
    LogMessage("--- ", "create process failed", "\n");
    CloseHandle(newstdin);
    CloseHandle(writestdin);
    CloseHandle(newstdout);
    CloseHandle(readstdout);
    return pxCreateProcessFailed;
  }

  inputpos=ainput;

  while(TRUE)
  {
    success=GetExitCodeProcess(processinformation.hProcess, aexitcode);
    if ((success)&&(*aexitcode!=STILL_ACTIVE)) break;

    storeOutput(readstdout, aoutput);

    if (*inputpos!='\0') size=1;
    else size=0;

    success=WriteFile(writestdin, inputpos, size, &transfered, NULL);

    inputpos+=transfered;
  }

  storeOutput(readstdout, aoutput);
  WaitForSingleObject(processinformation.hProcess, INFINITE);

  LogMessage("output:\n", *aoutput, "");

  CloseHandle(processinformation.hThread);
  CloseHandle(processinformation.hProcess);
  CloseHandle(newstdin);
  CloseHandle(newstdout);
  CloseHandle(readstdout);
  CloseHandle(writestdin);

  return pxSuccess;
}

