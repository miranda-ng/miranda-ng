#include "../commonheaders.h"
#include "gpgw.h"

void storeOutput(HANDLE ahandle, char **aoutput)
{
	DWORD available;
	do {
		PeekNamedPipe(ahandle, NULL, 0, NULL, &available, NULL);

		if (available == 0) continue;

		DWORD transfered;
		char readbuffer[10];
		DWORD success = ReadFile(ahandle, readbuffer, sizeof(readbuffer), &transfered, NULL);
		if ((success) && (transfered != 0))
			appendText(aoutput, readbuffer, transfered);
	} while (available > 0);
}

pxResult pxExecute(char *acommandline, char *ainput, char **aoutput, LPDWORD aexitcode)
{
	LogMessage("commandline:\n", acommandline, "\n");

	SECURITY_ATTRIBUTES securityattributes;
	memset(&securityattributes, 0, sizeof(securityattributes));
	securityattributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityattributes.bInheritHandle = TRUE;

	SECURITY_DESCRIPTOR securitydescriptor;
	InitializeSecurityDescriptor(&securitydescriptor, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&securitydescriptor, TRUE, NULL, FALSE);
	securityattributes.lpSecurityDescriptor = &securitydescriptor;

	HANDLE newstdin, newstdout, readstdout, writestdin;
	BOOL success = CreatePipe(&newstdin, &writestdin, &securityattributes, 0);
	if (!success) {
		LogMessage("--- ", "create pipe failed", "\n");
		return pxCreatePipeFailed;
	}

	success = CreatePipe(&readstdout, &newstdout, &securityattributes, 0);
	if (!success) {
		LogMessage("--- ", "create pipe failed", "\n");
		CloseHandle(newstdin);
		CloseHandle(writestdin);
		return pxCreatePipeFailed;
	}

	STARTUPINFO startupinfo;
	GetStartupInfo(&startupinfo);
	startupinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupinfo.wShowWindow = SW_HIDE;
	startupinfo.hStdOutput = newstdout;
	startupinfo.hStdError = newstdout;
	startupinfo.hStdInput = newstdin;

	PROCESS_INFORMATION processinformation;
	success = CreateProcess(NULL, acommandline, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &startupinfo, &processinformation);
	if (!success) {
		LogMessage("--- ", "create process failed", "\n");
		CloseHandle(newstdin);
		CloseHandle(writestdin);
		CloseHandle(newstdout);
		CloseHandle(readstdout);
		return pxCreateProcessFailed;
	}

	char *inputpos = ainput;

	while (TRUE) {
		success = GetExitCodeProcess(processinformation.hProcess, aexitcode);
		if ((success) && (*aexitcode != STILL_ACTIVE))
			break;

		storeOutput(readstdout, aoutput);

		int size;
		if (*inputpos != '\0')
			size = 1;
		else
			size = 0;

		DWORD transfered;
		success = WriteFile(writestdin, inputpos, size, &transfered, NULL);
		inputpos += transfered;
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
