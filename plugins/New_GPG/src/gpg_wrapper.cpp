// Copyright © 2010-22 sss
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

#include "stdafx.h"

namespace bp = boost::process;

gpg_execution_params::gpg_execution_params()
{ }

gpg_execution_params::~gpg_execution_params()
{ }

void pxEexcute_thread(gpg_execution_params *params)
{
	if (!globals.gpg_valid)
		return;

	CMStringW bin_path(g_plugin.getMStringW("szGpgBinPath"));
	if (_waccess(bin_path, 0)) {
		if (globals.debuglog)
			globals.debuglog << "GPG executable not found";
		params->result = pxNotFound;
		return;
	}

	bp::environment env = boost::this_process::environment();
	env.set("LANGUAGE", "en@quot");
	env.set("LC_ALL", "English");
	env.set("LANG", "C");

	std::vector<std::wstring> argv;
	CMStringW home_dir(g_plugin.getMStringW("szHomePath"));
	if (!home_dir.IsEmpty()) { // this check are required for first run gpg binary validation
		argv.push_back(L"--homedir");
		argv.push_back(home_dir.c_str());
	}

	argv.push_back(L"--display-charset");
	argv.push_back(L"utf-8");
	argv.push_back(L"-z9");
	argv.insert(argv.end(), params->aargv.begin(), params->aargv.end());

	if (globals.debuglog) {
		std::wstring args;
		for (unsigned int i = 0; i < argv.size(); ++i) {
			args += argv[i];
			args += L" ";
		}
		args.erase(args.size() - 1, 1);

		globals.debuglog << "gpg in: " << toUTF8(args);
	}

	params->out.Empty();

	wchar_t mir_path[MAX_PATH];
	PathToAbsoluteW(L"\\", mir_path);

	bp::child *c;
	std::future<std::string> pout, perr;
	boost::asio::io_context ios;
	if (params->bNoOutput)
		c = new bp::child(bin_path.c_str(), argv, bp::windows::hide, bp::std_in.close(), ios);
	else 
		c = new bp::child(bin_path.c_str(), argv, bp::windows::hide, bp::std_in.close(), bp::std_out > pout, bp::std_err > perr, ios);

	params->child = c;

	ios.run();
	c->wait();

	if (!params->bNoOutput) {
		std::string s = pout.get();
		if (!s.empty())
			params->out.Append(s.c_str());

		s = perr.get();
		if (!s.empty())
			params->out.Append(s.c_str());

		params->out.Replace("\r\n", "\n");
		params->out.Replace("\r\r", "");

		if (globals.debuglog)
			globals.debuglog << "gpg out: " << params->out.c_str();
	}

	params->child = nullptr;
	params->code = c->exit_code();
	delete c;

	if (params->code) {
		if (globals.debuglog)
			globals.debuglog << ": warning: wrong gpg exit status, gpg output: " << params->out.c_str();
		params->result = pxSuccessExitCodeInvalid;
	}
	else params->result = pxSuccess;
}

bool gpg_launcher(gpg_execution_params &params, boost::posix_time::time_duration t)
{
	bool ret = true;
	HANDLE hThread = mir_forkThread<gpg_execution_params>(pxEexcute_thread, &params);
	if (WaitForSingleObject(hThread, t.total_milliseconds()) == WAIT_TIMEOUT) {
		ret = false;
		if (params.child)
			params.child->terminate();
		if (globals.debuglog)
			globals.debuglog << "GPG execution timed out, aborted";
	}
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

void pxEexcute_passwd_change_thread(gpg_execution_params_pass *params)
{
	if (!globals.gpg_valid) {
		params->result = pxNotConfigured;
		return;
	}

	CMStringW bin_path(g_plugin.getMStringW("szGpgBinPath"));
	if (_waccess(bin_path, 0)) {
		if (globals.debuglog)
			globals.debuglog << "GPG executable not found";
		params->result = pxNotFound;
		return;
	}

	bp::environment env = boost::this_process::environment();
	env.set("LANGUAGE", "en@quot");
	env.set("LC_ALL", "English");

	std::vector<std::wstring> argv;

	argv.push_back(bin_path.c_str());
	argv.push_back(L"--homedir");
	argv.push_back(g_plugin.getMStringW("szHomePath").c_str());
	argv.push_back(L"--display-charset");
	argv.push_back(L"utf-8");
	argv.push_back(L"-z9");
	argv.insert(argv.end(), params->aargv.begin(), params->aargv.end());

	wchar_t mir_path[MAX_PATH];
	PathToAbsoluteW(L"\\", mir_path);

	bp::child c(bin_path.c_str(), argv, env, boost::process::windows::hide);
	params->child = &c;

	c.wait();

	params->child = nullptr;
}
