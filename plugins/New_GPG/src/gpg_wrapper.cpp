// Copyright © 2010-2012 sss
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "commonheaders.h"

//thx gpg module from Harald Treder, Zakhar V. Bardymov

//boost::mutex gpg_mutex;


pxResult pxExecute(wstring *acommandline, char *ainput, string *aoutput, LPDWORD aexitcode, pxResult *result, HANDLE hProcess, PROCESS_INFORMATION *pr)
{
//	gpg_mutex.lock();
	if(!gpg_valid)
		return pxNotConfigured;
	extern logtofile debuglog;
	BOOL success;
	STARTUPINFO sinfo = {0};
	SECURITY_ATTRIBUTES sattrs = {0};
	SECURITY_DESCRIPTOR sdesc = {0};
	PROCESS_INFORMATION pri = {0};
	HANDLE  newstdin, newstdout, readstdout, writestdin;
	char *inputpos;
	unsigned long transfered;
	int size;

	wstring commandline;

	sattrs.nLength=sizeof(SECURITY_ATTRIBUTES);
	sattrs.bInheritHandle=TRUE;
	InitializeSecurityDescriptor(&sdesc,SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sdesc,TRUE,NULL,FALSE);
	sattrs.lpSecurityDescriptor=&sdesc;

	success=CreatePipe(&newstdin,&writestdin,&sattrs,0);
	if (!success)
	{
		*result = pxCreatePipeFailed;
		return pxCreatePipeFailed;
	}

	success=CreatePipe(&readstdout,&newstdout,&sattrs,0);
	if (!success)
	{
		CloseHandle(newstdin);
		CloseHandle(writestdin);
		*result = pxCreatePipeFailed;
		return pxCreatePipeFailed;
	}

	GetStartupInfo(&sinfo);
	sinfo.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
	sinfo.wShowWindow=SW_HIDE;
	sinfo.hStdOutput=newstdout;
	sinfo.hStdError=newstdout;
	sinfo.hStdInput=newstdin;

	char *mir_path = new char [MAX_PATH];
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)"\\", (LPARAM)mir_path);
	SetCurrentDirectoryA(mir_path);
	delete [] mir_path;

	TCHAR *bin_path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", _T(""));
	{
		if(_waccess(bin_path, 0) == -1)
		{
			if(errno == ENOENT)
			{
				mir_free(bin_path);
				debuglog<<time_str()<<": GPG executable not found\n";
				*result = pxNotFound;
				return pxNotFound;
			}
		}
	}
	TCHAR *home_dir = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
	{ //form initial command
		commandline += _T("\"");
		commandline += bin_path;
		commandline += _T("\" --homedir \"");
		commandline += home_dir;
		commandline += _T("\" ");
		commandline += _T("--display-charset utf-8 ");
		commandline += _T("-z 9 ");
		commandline += *acommandline;
		mir_free(bin_path);
		mir_free(home_dir);
	}

	debuglog<<time_str()<<": gpg in: "<<commandline<<"\n";

	success = CreateProcess(NULL, (TCHAR*)commandline.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT, (void*)_T("LANGUAGE=en@quot\0LC_ALL=English\0"), NULL, &sinfo, &pri);

	if (!success)
	{
		CloseHandle(newstdin);
		CloseHandle(writestdin);
		CloseHandle(newstdout);
		CloseHandle(readstdout);
		debuglog<<time_str()<<": Failed to create process\n";
//		gpg_mutex.unlock();
		*result = pxCreateProcessFailed;
		return pxCreateProcessFailed;
	}

	hProcess = pri.hProcess;

	inputpos=ainput;

	while (true)
	{
		if(!pri.hProcess)
			break;
		success=GetExitCodeProcess(pri.hProcess,aexitcode);
		if (success && *aexitcode!=STILL_ACTIVE)
			break;

		storeOutput(readstdout,aoutput);

		if (*inputpos!='\0') size=1;
		else size=0;

		success=WriteFile(writestdin,inputpos,size,&transfered,NULL);
		inputpos+=transfered;
		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
	}

	storeOutput(readstdout,aoutput);

	fix_line_term(*aoutput);

	debuglog<<time_str()<<": gpg out: "<<aoutput->c_str();

	WaitForSingleObject(pri.hProcess,INFINITE);

	CloseHandle(pri.hThread);
	CloseHandle(pri.hProcess);
	CloseHandle(newstdin);
	CloseHandle(newstdout);
	CloseHandle(readstdout);
	CloseHandle(writestdin);

	*result = pxSuccess;
//	gpg_mutex.unlock();
	return pxSuccess;
}

void pxEexcute_thread(void *param)
{
	gpg_execution_params *params = (gpg_execution_params*)param;
	pxResult result = pxExecute(params->cmd, params->useless, params->out, params->code, params->result, params->hProcess, params->proc);
}
