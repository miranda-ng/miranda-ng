/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

extern int CreateTimerForConnectingIcon(WPARAM,LPARAM);

void FreeProtocolData( void )
{
	//free protocol data
	int nPanel;
	int nParts = SendMessage(pcli->hwndStatus,SB_GETPARTS,0,0);
	for (nPanel = 0;nPanel<nParts;nPanel++)
	{
		ProtocolData *PD;
		PD = (ProtocolData *)SendMessage(pcli->hwndStatus,SB_GETTEXT,(WPARAM)nPanel,0);
		if (PD != NULL && !IsBadCodePtr(( FARPROC )PD)) {
			SendMessage(pcli->hwndStatus,SB_SETTEXT,(WPARAM)nPanel|SBT_OWNERDRAW,0);
			if (PD->RealName) mir_free(PD->RealName);
			if (PD) mir_free(PD);
}	}	}

void CluiProtocolStatusChanged(int parStatus, const char* szProto)
{
	int protoCount,i;
	PROTOACCOUNT **accs;
	int *partWidths,partCount;
	int borders[3];
	int flags;
	int FirstIconOffset;

	if ( pcli->hwndStatus == 0 )
		return;

	FirstIconOffset = db_get_dw(NULL,"CLUI","FirstIconOffset",0);

	ProtoEnumAccounts( &protoCount, &accs );
	if ( protoCount == 0 )
		return;

	OutputDebugStringA("CluiProtocolStatusChanged");
	OutputDebugStringA("\r\n");
	FreeProtocolData();

	SendMessage(pcli->hwndStatus,SB_GETBORDERS,0,(LPARAM)&borders);

	SendMessage(pcli->hwndStatus,SB_SETBKCOLOR,0,db_get_dw(0,"CLUI","SBarBKColor",CLR_DEFAULT));
	partWidths = (int*)alloca((protoCount+1)*sizeof(int));
	//partWidths[0] = FirstIconOffset;
	if ( db_get_b(NULL,"CLUI","UseOwnerDrawStatusBar",0)||db_get_b(NULL,"CLUI","EqualSections",1))
    {
		RECT rc;
		int toshow, part;

		SendMessage(pcli->hwndStatus,WM_SIZE,0,0);
		GetClientRect(pcli->hwndStatus,&rc);
		rc.right -= borders[0]*2;
		toshow = 0;
		for (i = 0;i<protoCount;i++)
		{
			if ( !pcli->pfnGetProtocolVisibility( accs[i]->szModuleName ))
				continue;

			toshow++;
		}

		if (toshow>0)
		{
			for (part = 0,i = 0;i<protoCount;i++)
			{
			    if ( !pcli->pfnGetProtocolVisibility( accs[i]->szModuleName ))
				    continue;

				partWidths[part] = ((part+1)*(rc.right/toshow))-(borders[2]>>1);
				//partWidths[part] = 40*part+40;
				part++;
			}
		//partCount = part;
		}
		partCount = toshow;

	}
	else {
		HDC hdc;
		SIZE textSize;
		BYTE showOpts = db_get_b(NULL,"CLUI","SBarShow",5);
		DWORD extraspace = db_get_dw(NULL,"StatusBar","BkExtraSpace",0);
		DWORD startoffset = db_get_dw(NULL,"StatusBar","FirstIconOffset",0);
		int x;

		hdc = GetDC(pcli->hwndStatus);

		for (partCount = 0,i = 0;i<protoCount;i++) {      //count down since built in ones tend to go at the end
			int ind = pcli->pfnGetAccountIndexByPos(i);
			if (ind < 0)
				continue;

			if ( !pcli->pfnGetProtocolVisibility(accs[ind]->szModuleName))
				continue;

			x = 2;
			if (showOpts & 1) x += GetSystemMetrics(SM_CXSMICON);
			if (showOpts & 2) {
				TCHAR szName[64];
				mir_sntprintf(szName, SIZEOF(szName), _T("%s%s"), accs[ind]->tszAccountName, showOpts&4 ? _T(" ") : _T(""));
				GetTextExtentPoint32(hdc, szName, (int)_tcslen(szName), &textSize);
				x += textSize.cx;
			}
			if (showOpts & 4) {
				int status = CallProtoService(accs[ind]->szModuleName, PS_GETSTATUS, 0, 0);
				TCHAR *szStatus = pcli->pfnGetStatusModeDescription(status, 0);
				if ( !szStatus)
					szStatus = _T("");
				GetTextExtentPoint32(hdc, szStatus, (int)_tcslen(szStatus), &textSize);
				x += textSize.cx;
			}
			if (showOpts&6) x += 2;
			partWidths[partCount] = (partCount?partWidths[partCount-1]:startoffset)+x+extraspace;
			partCount++;
		}
		ReleaseDC(pcli->hwndStatus,hdc);
	}
	if (partCount == 0) {
		SendMessage(pcli->hwndStatus,SB_SIMPLE,TRUE,0);
		return;
	}
	SendMessage(pcli->hwndStatus,SB_SIMPLE,FALSE,0);

	partWidths[partCount-1] = -1;

	SendMessage(pcli->hwndStatus,SB_SETMINHEIGHT,GetSystemMetrics(SM_CYSMICON)+2,0);
	SendMessage(pcli->hwndStatus,SB_SETPARTS,partCount,(LPARAM)partWidths);

	flags = SBT_OWNERDRAW;
	if ( !db_get_b(NULL, "CLUI", "SBarBevel", 1))
		flags |= SBT_NOBORDERS;

	for (partCount = 0, i = 0; i<protoCount; i++) {    //count down since built in ones tend to go at the end
		int ind = pcli->pfnGetAccountIndexByPos(i);
		if (ind < 0) continue;

		if ( !pcli->pfnGetProtocolVisibility(accs[ind]->szModuleName))
			continue;

		ProtocolData *PD = (ProtocolData*)mir_alloc(sizeof(ProtocolData));
		PD->RealName = mir_strdup(accs[ind]->szModuleName);
		PD->protopos = accs[ind]->iOrder;

		SendMessage(pcli->hwndStatus, SB_SETTEXT, partCount++ | flags, (LPARAM)PD);
	}

	CreateTimerForConnectingIcon(parStatus, (LPARAM)szProto);
	InvalidateRect(pcli->hwndStatus, NULL, FALSE);
	return;
}
