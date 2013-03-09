/* eventhooker.h - Helper for hooking events in Miranda. 
* Copyright (C) Miklashevsky Roman, sss, elzor
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef EVENTHOOKER_H_C8EAA58A_7C4D_45f7_A88E_0E41FE93754D
#define EVENTHOOKER_H_C8EAA58A_7C4D_45f7_A88E_0E41FE93754D

namespace miranda
{

#define MIRANDA_HOOK_EVENT(NAME, WPARAMNAME, LPARAMNAME) \
	int NAME##_Handler(WPARAM,LPARAM);\
	miranda::EventHooker NAME##_Hooker(NAME, NAME##_Handler);\
	int NAME##_Handler(WPARAM WPARAMNAME, LPARAM LPARAMNAME)

	struct EventHooker
	{
		EventHooker(std::string name, MIRANDAHOOK fun);
		~EventHooker();
		void Hook();
		void Unhook();
		static void HookAll();
		static void UnhookAll();
	private:
		std::string name_;
		MIRANDAHOOK fun_;
		HANDLE handle_;
	};

}

#endif
