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
		if(!boost::filesystem::exists(bin_path))
		{
			mir_free(bin_path);
			if(bDebugLog)
				debuglog<<std::string(time_str()+": GPG executable not found");
			*result = pxNotFound;
			return pxNotFound;
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

	if(bDebugLog)
		debuglog<<std::string(time_str()+": gpg in: "+toUTF8(commandline));

	success = CreateProcess(NULL, (TCHAR*)commandline.c_str(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT, (void*)_T("LANGUAGE=en@quot\0LC_ALL=English\0"), NULL, &sinfo, &pri);

	if (!success)
	{
		CloseHandle(newstdin);
		CloseHandle(writestdin);
		CloseHandle(newstdout);
		CloseHandle(readstdout);
		if(bDebugLog)
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

	if(bDebugLog)
		debuglog<<std::string(time_str()+": gpg out: "+*aoutput);

	WaitForSingleObject(pri.hProcess,INFINITE);

	CloseHandle(pri.hThread);
	CloseHandle(pri.hProcess);
	CloseHandle(newstdin);
	CloseHandle(newstdout);
	CloseHandle(readstdout);
	CloseHandle(writestdin);

	*result = pxSuccess;
//	gpg_mutex.unlock();
	if(*aexitcode)
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": warning: wrong gpg exit status, gpg output: "+*aoutput);
		return pxSuccessExitCodeInvalid;
	}
	return pxSuccess;
}

void pxEexcute_thread(void *param)
{
	gpg_execution_params *params = (gpg_execution_params*)param;
	pxResult result = pxExecute(params->cmd, params->useless, params->out, params->code, params->result, params->hProcess, params->proc);
}



pxResult pxExecute_passwd_change(std::vector<std::wstring> &aargv, char *ainput, string *aoutput, LPDWORD aexitcode, pxResult *result, HANDLE hProcess, PROCESS_INFORMATION *pr, string &old_pass, string &new_pass)
{
//	gpg_mutex.lock();
	if(!gpg_valid)
		return pxNotConfigured;
	extern logtofile debuglog;

	TCHAR *bin_path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", _T(""));
	{
		if(!boost::filesystem::exists(bin_path))
		{
			mir_free(bin_path);
			if(bDebugLog)
				debuglog<<std::string(time_str()+": GPG executable not found");
			*result = pxNotFound;
			return pxNotFound;
		}
	}

	using namespace boost::process;
	using namespace boost::process::initializers;
	using namespace boost::iostreams;


	std::vector<std::wstring> argv;
	std::vector<std::wstring> env;
	env.push_back(L"LANGUAGE=en@quot");
	env.push_back(L"LC_ALL=English");
	argv.push_back(bin_path);
	argv.push_back(L"--homedir");
	TCHAR *home_dir = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
	argv.push_back(home_dir);
	mir_free(home_dir);
	argv.push_back(L"--display-charset");
	argv.push_back(L"utf-8");
	argv.push_back(L"-z9");
	argv.insert(argv.end(), aargv.begin(), aargv.end());

//	pipe pout = create_pipe();
	pipe pin = create_pipe();
//	file_descriptor_sink sout(pout.sink, close_handle);
	file_descriptor_source sin(pin.source, close_handle);

	char *mir_path = new char [MAX_PATH];
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)"\\", (LPARAM)mir_path);

	//execute(set_args(argv), bind_stdout(sout), bind_stdin(sin), show_window(SW_HIDE), hide_console(), inherit_env(), set_env(env), start_in_dir(toUTF16(mir_path)));
	child c = execute(set_args(argv), bind_stdin(sin), inherit_env(), set_env(env), start_in_dir(toUTF16(mir_path)));
	//child c = execute(run_exe("c:\\windows\\system32\\cmd.exe"), bind_stdin(sin), inherit_env(), set_env(env), start_in_dir(toUTF16(mir_path)));

	delete [] mir_path;
	
//	file_descriptor_source source(pout.source, close_handle);
/*	file_descriptor_sink _sin(pin.sink, close_handle);

	stream<file_descriptor_sink> out(_sin);


	std::wstring cmd;
	for(int i = 0; i < argv.size(); i++)
	{
		cmd += argv[i];
		cmd += L" ";
	}
	*/
//	out<<toUTF8(cmd)<<std::endl;

	//fucked gpg does not want to give us stdin/stdout
	wait_for_exit(c);

/*	out<<old_pass<<std::endl;
	out<<new_pass<<std::endl;
	out<<new_pass<<std::endl;
	out<<"save"<<std::endl; */

//	stream<file_descriptor_source> is(source);
/*	std::string s;

	is>>s;
	
	MessageBoxA(NULL, s.c_str(), "", MB_OK); */


/*	ctx.environment = boost::process::self::get_environment();
	ctx.environment.insert(boost::process::environment::value_type("LANGUAGE", "en@quot"));
	ctx.environment["LC_ALL"] = "English";

	char *mir_path = new char [MAX_PATH];
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)"\\", (LPARAM)mir_path);
	ctx.work_directory = mir_path;
	delete [] mir_path;

	boost::process::win32_child proc = boost::process::win32_launch(toUTF8(bin_path), argv, ctx);

	mir_free(bin_path);

//	boost::process::pistream &_stdout = proc.get_stdout();
	boost::process::postream &_stdin = proc.get_stdin();

	boost::this_thread::sleep(boost::posix_time::milliseconds(50));

	std::string buf; */


/*	std::getline(_stdout, buf);
	while(_stdout.good())
	{
		*aoutput += buf;
		if(aoutput->find("Enter passphrase") != std::string::npos)
			break;
		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
		std::getline(_stdout, buf);
	}
	*aoutput += buf; */

	//_stdin<<old_pass<<std::endl;

/*	std::getline(_stdout, buf);
	while(_stdout.good())
	{
		*aoutput += buf;
		if(aoutput->find("Enter the new passphrase for this secret key.") != std::string::npos)
			break;
		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
		std::getline(_stdout, buf);
	}

	*aoutput += buf;

	if(aoutput->find("Enter passphrase") != std::string::npos)*/
		//_stdin<<new_pass<<std::endl;

/*	std::getline(_stdout, buf);
	while(_stdout.good())
	{
		*aoutput += buf;
		if(aoutput->find("Repeat passphrase") != std::string::npos)
			break;
		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
		std::getline(_stdout, buf);
	}
	*aoutput += buf; */

	//_stdin<<new_pass<<std::endl;

/*	std::getline(_stdout, buf);
	while(_stdout.good())
	{
		*aoutput += buf;
		if(aoutput->find("Command") != std::string::npos)
			break;
		boost::this_thread::sleep(boost::posix_time::milliseconds(50));
		std::getline(_stdout, buf);
	}
	*aoutput += buf; */

	//_stdin<<"save"<<std::endl; 

	//proc.wait();

	//MessageBoxA(NULL, aoutput->c_str(), "info", MB_OK);

	return pxSuccess;
}


void pxEexcute_passwd_change_thread(void *param)
{
	gpg_execution_params_pass *params = (gpg_execution_params_pass*)param;
	pxResult result = pxExecute_passwd_change(params->args, params->useless, params->out, params->code, params->result, params->hProcess, params->proc, params->old_pass, params->new_pass);
}
