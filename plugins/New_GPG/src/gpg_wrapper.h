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


#ifndef GPG_WRAPPER_H
#define GPG_WRAPPER_H
enum pxResult
{
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
};

struct gpg_execution_params
{
	gpg_execution_params();
	~gpg_execution_params();

	std::vector<std::wstring> aargv;
	CMStringA out;
	uint32_t code = 0;
	int bNoOutput = false;
	pxResult result = pxSuccess;
	boost::process::child *child = nullptr;

	__forceinline void addParam(const std::wstring &param)
	{	aargv.push_back(param);
	}
};

struct gpg_execution_params_pass : public gpg_execution_params
{
	string &old_pass, &new_pass;

	gpg_execution_params_pass(std::string &o, std::string &n):
		old_pass(o),
		new_pass(n)
	{
	}
};


bool gpg_launcher(gpg_execution_params &params, boost::posix_time::time_duration t = boost::posix_time::seconds(10));
void __cdecl pxEexcute_passwd_change_thread(gpg_execution_params_pass *param);

#endif