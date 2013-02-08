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


#ifndef GPG_WRAPPER_H
#define GPG_WRAPPER_H
typedef enum {
	pxSuccess,
	pxSuccessExitCodeInvalid,
	pxCreatePipeFailed,
	pxDuplicateHandleFailed,
	pxCloseHandleFailed,
	pxCreateProcessFailed,
	pxThreadWaitFailed,
	pxReadFileFailed,
	pxBufferOverflow,
	pxNotFound,
	pxNotConfigured
}
pxResult;

pxResult pxExecute(std::vector<std::string> &aargv, string *aoutput, LPDWORD aexitcode, pxResult *result);
pxResult pxExecute_passwd_change(std::vector<std::string> &aargv, string *aoutput, LPDWORD aexitcode, pxResult *result, string &old_pass, string &new_pass);

struct gpg_execution_params
{
	std::vector<std::wstring> &aargv;
//	char *useless;
	string *out;
	LPDWORD code;
	pxResult *result;
	boost::process::child *child;
//	HANDLE hProcess;
//	PROCESS_INFORMATION *proc;
	gpg_execution_params(std::vector<std::wstring> &a): aargv(a)
	{
		child = nullptr;
	}
};

struct gpg_execution_params_pass
{
	std::vector<std::wstring> &args;
	string &old_pass, &new_pass;
	string *out;
	LPDWORD code;
	pxResult *result;
	boost::process::child *child;
//	HANDLE hProcess;
//	PROCESS_INFORMATION *proc;
	gpg_execution_params_pass(std::vector<std::wstring> &a, std::string &o, std::string &n): args(a), old_pass(o), new_pass(n)
	{
		child = nullptr;
	}
};


void pxEexcute_thread(gpg_execution_params &params);
bool gpg_launcher(gpg_execution_params &params, boost::posix_time::time_duration t = boost::posix_time::seconds(10));
void pxEexcute_passwd_change_thread(void *param);

#endif