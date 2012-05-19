/*

'Language Pack Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (LangMan-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "common.h"

/************************* Error Output ***************************/

static void MessageBoxIndirectFree(MSGBOXPARAMSA *mbp)
{
	MessageBoxIndirectA(mbp);
	mir_free((char*)mbp->lpszCaption); /* does NULL check */
	mir_free((char*)mbp->lpszText);    /* does NULL check */
	mir_free(mbp);
}

void ShowInfoMessage(BYTE flags,const char *pszTitle,const char *pszTextFmt,...)
{
	char szText[256]; /* max for systray */
	MSGBOXPARAMSA *mbp;

	va_list va;
	va_start(va,pszTextFmt);
	mir_vsnprintf(szText,SIZEOF(szText),pszTextFmt,va);
	va_end(va);

	if(ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
		MIRANDASYSTRAYNOTIFY msn;
		msn.cbSize=sizeof(msn);
		msn.szProto=NULL;
		msn.szInfoTitle=(char*)pszTitle;
		msn.szInfo=(char*)szText;
		msn.uTimeout=30000; /* max timeout */
		msn.dwInfoFlags=flags;
		if(!CallServiceSync(MS_CLIST_SYSTRAY_NOTIFY,0,(LPARAM)&msn))
			return; /* success */
	}

	mbp=(MSGBOXPARAMSA*)mir_calloc(sizeof(*mbp));
	if(mbp==NULL) return;
	mbp->cbSize=sizeof(*mbp);
	mbp->lpszCaption=mir_strdup(pszTitle);
	mbp->lpszText=mir_strdup(szText);
	mbp->dwStyle=MB_OK|MB_SETFOREGROUND|MB_TASKMODAL;
	mbp->dwLanguageId=LANGIDFROMLCID((LCID)CallService(MS_LANGPACK_GETLOCALE,0,0));
	switch(flags&NIIF_ICON_MASK) {
		case NIIF_INFO:    mbp->dwStyle|=MB_ICONINFORMATION; break;
		case NIIF_WARNING: mbp->dwStyle|=MB_ICONWARNING; break;
		case NIIF_ERROR:   mbp->dwStyle|=MB_ICONERROR;
	}
	mir_forkthread(MessageBoxIndirectFree,mbp);
}
