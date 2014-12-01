/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//---------------------------------------------------------------------------
#include "global.h"

//---------------------------------------------------------------------------
CSendHost_ImageShack::CSendHost_ImageShack(HWND Owner, MCONTACT hContact, bool bAsync)
: CSend(Owner, hContact, bAsync) {
	m_EnableItem		= SS_DLG_DESCRIPTION | SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp		= LPGENT("Image upload");
}

CSendHost_ImageShack::~CSendHost_ImageShack(){
};

//---------------------------------------------------------------------------
int CSendHost_ImageShack::Send() {
	if(!hNetlibUser){ /// check Netlib
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}
	memset(&m_nlhr, 0, sizeof(m_nlhr));
	char* tmp; tmp=mir_t2a(m_pszFile);
	HTTPFormData frm[]={
//		{"Referer",HTTPFORM_HEADER("http://www.imageshack.us/upload_api.php")},
		{"fileupload",HTTPFORM_FILE(tmp)},
		//{"rembar","yes"},// no info bar on thumb
		{"public","no"},
		{"key",HTTPFORM_8BIT(DEVKEY_IMAGESHACK)},
	};
	int error=HTTPFormCreate(&m_nlhr,REQUEST_POST,"http://imageshack.us/upload_api.php",frm,sizeof(frm)/sizeof(HTTPFormData));
	mir_free(tmp);
	if(error)
		return !m_bAsync;
	/// start upload thread
	if(m_bAsync){
		mir_forkthread(&CSendHost_ImageShack::SendThreadWrapper, this);
		return 0;
	}
	SendThread();
	return 1;
}

void CSendHost_ImageShack::SendThread() {
	/// send DATA and wait for m_nlreply
	NETLIBHTTPREQUEST* reply=(NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&m_nlhr);
	HTTPFormDestroy(&m_nlhr);
	if(reply){
		if(reply->resultCode>=200 && reply->resultCode<300 && reply->dataLength){
			reply->pData[reply->dataLength-1]='\0';/// make sure its null terminated
			const char* url=NULL;
			url=GetHTMLContent(reply->pData,"<image_link>","</image_link>");
			if(url && *url){
				mir_free(m_URL), m_URL=mir_strdup(url);
				mir_free(m_URLthumb), m_URLthumb=mir_strdup(m_URL);
				size_t extlen;
				char* pos=strrchr(m_URLthumb,'.');
				if(pos && (extlen=mir_strlen(pos))>2){
					char* tmp=mir_strdup(pos);
					memcpy(pos,".th",3);
					memcpy(pos+3,tmp,extlen-3);
					mir_stradd(m_URLthumb,tmp+extlen-3);
					mir_free(tmp);
				}else{
					mir_freeAndNil(m_URLthumb);
				}
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)reply);
				svcSendMsgExit(url); return;
			}else{/// check error mess from server
				url=GetHTMLContent(reply->pData,"<error ","</error>");
				TCHAR* err=NULL;
				if(url) err=mir_a2t(url);
				if (!err || !*err){/// fallback to server response mess
					mir_free(err);
					err=mir_a2t(reply->pData);
				}
				Error(_T("%s"),err);
				mir_free(err);
			}
		}else{
			Error(SS_ERR_RESPONSE,m_pszSendTyp,reply->resultCode);
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)reply);
	}else{
		Error(SS_ERR_NORESPONSE,m_pszSendTyp,m_nlhr.resultCode);
	}
	Exit(ACKRESULT_FAILED);
}

void CSendHost_ImageShack::SendThreadWrapper(void * Obj) {
	reinterpret_cast<CSendHost_ImageShack*>(Obj)->SendThread();
}
