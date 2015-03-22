/*
Copyright (c) 2014 Robert Pösel

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

struct CDummyProto;

struct CDummyProto : public PROTO<CDummyProto>
{
				CDummyProto(const char*, const TCHAR*);
				~CDummyProto();

	//====================================================================================
	// PROTO_INTERFACE
	//====================================================================================

	virtual	DWORD_PTR __cdecl GetCaps(int type, MCONTACT hContact = NULL);

	virtual	int       __cdecl SendMsg(MCONTACT hContact, int flags, const char* msg);

	virtual	int       __cdecl SetStatus(int iNewStatus);

	//==== Events ========================================================================

	int __cdecl OnModulesLoaded(WPARAM, LPARAM);
	int __cdecl OnOptionsInit(WPARAM, LPARAM);
	int __cdecl OnPreShutdown(WPARAM, LPARAM);

	//==== Services ======================================================================

	INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
	
	void __cdecl SendMsgAck(void *param);
	
	char uniqueIdText[100];
	char uniqueIdSetting[100];
};
