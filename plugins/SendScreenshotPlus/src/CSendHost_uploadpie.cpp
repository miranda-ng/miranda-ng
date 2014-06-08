/*
            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2014 Miranda NG project (http://miranda-ng.org)

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/
#include "global.h"

CSendHost_UploadPie::CSendHost_UploadPie(HWND Owner, MCONTACT hContact, bool bAsync, int expire)
	: m_expire(expire), CSend(Owner,hContact,bAsync)
{
	m_EnableItem=SS_DLG_DESCRIPTION|SS_DLG_AUTOSEND|SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp=LPGENT("Image upload");
}

CSendHost_UploadPie::~CSendHost_UploadPie()
{}

//---------------------------------------------------------------------------
int CSendHost_UploadPie::Send()
{
	if(!hNetlibUser){ /// check Netlib
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}
	ZeroMemory(&m_nlhr, sizeof(m_nlhr));
	char* tmp; tmp=mir_t2a(m_pszFile);
	HTTPFormData frm[]={
		{"MAX_FILE_SIZE",HTTPFORM_INT(3145728)},// ??
		{"upload",HTTPFORM_INT(1)},// ??
		{"uploadedfile",HTTPFORM_FILE(tmp)},
		{"expire",HTTPFORM_INT(m_expire)},// 30m
		//{"expire",HTTPFORM_INT(2,},// 1h
		//{"expire",HTTPFORM_INT(3)},// 6h
		//{"expire",HTTPFORM_INT(4)},// 1d
		//{"expire",HTTPFORM_INT(5)},// 1w
		//{"x",HTTPFORM_INT(130)},// ??
		//{"y",HTTPFORM_INT(17)},// ??
	};
	int error=HTTPFormCreate(&m_nlhr,REQUEST_POST,"http://uploadpie.com/",frm,sizeof(frm)/sizeof(HTTPFormData));
	mir_free(tmp);
	if(error)
		return !m_bAsync;
	/// start upload thread
	if(m_bAsync){
		mir_forkthread(&CSendHost_UploadPie::SendThread, this);
		return 0;
	}
	SendThread(this);
	return 1;
}

void CSendHost_UploadPie::SendThread(void* obj)
{
	CSendHost_UploadPie* self=(CSendHost_UploadPie*)obj;
	//send DATA and wait for m_nlreply
	NETLIBHTTPREQUEST* reply=(NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,(WPARAM)hNetlibUser,(LPARAM)&self->m_nlhr);
	self->HTTPFormDestroy(&self->m_nlhr);
	if(reply){
		if(reply->resultCode>=200 && reply->resultCode<300 && reply->dataLength){
			reply->pData[reply->dataLength-1]='\0';/// make sure its null terminated
			char* url=reply->pData;
			do{
				char* pos;
				if((url=strstr(url,"http://uploadpie.com/"))){
					for(pos=url+21; (*pos>='0'&&*pos<='9') || (*pos>='a'&&*pos<='z') || (*pos>='A'&&*pos<='Z') || *pos=='_' || *pos=='-' || *pos=='"' || *pos=='\''; ++pos){
						if(*pos=='"' || *pos=='\'') break;
					}
					if(url+21!=pos && (*pos=='"' || *pos=='\'')){
						*pos='\0';
						break;
					}
					++url;
				}
			}while(url);
			if(url){
				mir_free(self->m_URL), self->m_URL=mir_strdup(url);
				CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)reply);
				self->svcSendMsgExit(url); return;
			}else{/// check error mess from server
				const char* err=GetHTMLContent(reply->pData,"<p id=\"error\"","</p>");
				TCHAR* werr;
				if(err) werr=mir_a2t(err);
				else werr=mir_a2t(reply->pData);
				self->Error(_T("%s"),werr);
				mir_free(werr);
			}
		}else{
			self->Error(LPGENT("Upload server did not respond timely."));
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)reply);
	}else{
		self->Error(SS_ERR_INIT, self->m_pszSendTyp);
	}
	self->Exit(ACKRESULT_FAILED);
}
