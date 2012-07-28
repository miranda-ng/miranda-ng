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

pxResult pxExecute(wstring *acommandline, char *ainput, string *aoutput, LPDWORD aexitcode, pxResult *result);

struct gpg_execution_params
{
	wstring *cmd;
	char *useless;
	string *out;
	LPDWORD code;
	pxResult *result;
	HANDLE hProcess;
	PROCESS_INFORMATION *proc;
};

void pxEexcute_thread(void *param);

#endif