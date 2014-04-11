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
CSendImageShack::CSendImageShack(HWND Owner, MCONTACT hContact, bool bAsync)
: CSend(Owner, hContact, bAsync) {
	m_EnableItem		= SS_DLG_DESCRIPTION | SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND;
	m_pszSendTyp		= LPGENT("Image upload");
	m_pszFileName		= NULL;
	m_pszContentType	= NULL;
	m_MFDRboundary		= NULL;
	m_nlreply			= NULL;
	m_Url				= NULL;
}

CSendImageShack::~CSendImageShack(){
	mir_free(m_pszFileName);
	mir_free(m_MFDRboundary);
	// FREEHTTPREQUESTSTRUCT*
	if (m_nlreply) CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) m_nlreply);
	mir_free(m_Url);
};

//---------------------------------------------------------------------------
int CSendImageShack::Send() {
	if(!hNetlibUser){ /// check Netlib
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}
	if (!m_pszFileName) {
		m_pszFileName = GetFileNameA(m_pszFile);
	}
	if (!m_pszContentType) GetContentType();

	// create new boundary
	MFDR_Reset();

	// initialize the netlib request
	ZeroMemory(&m_nlhr, sizeof(m_nlhr));
	m_nlhr.cbSize					= sizeof(m_nlhr);
	m_nlhr.requestType				= REQUEST_POST;
	m_nlhr.flags					= NLHRF_HTTP11;			//NLHRF_DUMPASTEXT;
	m_nlhr.szUrl					= "http://www.imageshack.us/upload_api.php";
	m_nlhr.headersCount				= 6;
	{	//NETLIBHTTPHEADER start
		m_nlhr.headers=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*m_nlhr.headersCount);
		m_nlhr.headers[0].szName		= "Referer";
		m_nlhr.headers[0].szValue		= "http://www.imageshack.us/upload_api.php";
		m_nlhr.headers[1].szName		= "Connection";
		m_nlhr.headers[1].szValue		= "Keep-alive";
		m_nlhr.headers[2].szName		= "AcceptLanguage";
		m_nlhr.headers[2].szValue		= "en-us, pt-br";
		m_nlhr.headers[3].szName		= "Host";
		m_nlhr.headers[3].szValue		= "imageshack.us";
		m_nlhr.headers[4].szName		= "User-Agent";
		m_nlhr.headers[4].szValue		= __USER_AGENT_STRING;	//szAgent;	/;
		//nlhr.headers[x].szName		= "Authorization";
		//nlhr.headers[x].szValue		= auth;		//Basic base-64-authorization

		//$header .= "Content-type: multipart/form-data; boundary=" . part::getBoundary() . "\r\n";
		mir_snprintf(m_nlheader_ContentType, SIZEOF(m_nlheader_ContentType), "multipart/form-data; boundary=%s", m_MFDRboundary);
		m_nlhr.headers[m_nlhr.headersCount-1].szName		= "Content-Type";
		m_nlhr.headers[m_nlhr.headersCount-1].szValue		= m_nlheader_ContentType;
	}	//NETLIBHTTPHEADER end

//POST DATA file-header, init DATA with MultipartFormDataRequest
	//$params[] = new filepart('fileupload', $file, basename($file), $contentType, 'iso-8859-1');
	//($this->sendStart($h);)
	AppendToData("--");
	AppendToData(m_MFDRboundary);
	AppendToData("\r\n");
	//($this->sendDispositionHeader($h);)
	AppendToData("Content-Disposition: form-data; name=\"");
	AppendToData("fileupload");
	AppendToData("\"; filename=\"");
	AppendToData(m_pszFileName);
	AppendToData("\"");
	AppendToData("\r\n");
	//($this->sendContentTypeHeader($h);)
	AppendToData("Content-Type: ");
	AppendToData(m_pszContentType);
	AppendToData("; charset=");
	AppendToData("iso-8859-1");
	//($this->sendEndOfHeader($h);)
	AppendToData("\r\n");
	AppendToData("\r\n");
	//Now we add the file binary ($this->sendData($h))
	FILE * fileId = _tfsopen(m_pszFile, _T("rb"), _SH_DENYWR );
	if( !fileId) {
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
		return !m_bAsync;
	}
	fseek(fileId, NULL, SEEK_END);
	size_t lenFile  = ftell(fileId);
	size_t sizeDest = sizeof(char)*(m_nlhr.dataLength + lenFile + 1);
	m_nlhr.pData    = (char *) mir_realloc(m_nlhr.pData, sizeDest);
	fseek(fileId, NULL, SEEK_SET );
	int i;
	int ch = fgetc( fileId );
	for( i=0; (i < (int)lenFile ) && ( feof( fileId ) == 0 ); i++ ) {
		m_nlhr.pData[m_nlhr.dataLength+i] = (char)ch;
		ch = fgetc( fileId );
	}
	m_nlhr.pData[sizeDest-1] = 0;						//NULL Termination for binary data
	m_nlhr.dataLength = (int)sizeDest - 1;
	fclose(fileId);
	//($this->sendEnd($h);)
	AppendToData("\r\n");

//POST DATA footer (for "optimage", 1)
//POST DATA footer (for "optsize", optsize)

//POST DATA footer (for "tags", tags)
//POST DATA footer (for "rembar", "yes" : "no")
//POST DATA footer (for "public", "yes" : "no")
//POST DATA footer (for "cookie", cookie)

//POST DATA footer (for "key", DEVKEY_IMAGESHACK)
	//($this->sendStart($h);)
	AppendToData("--");
	AppendToData(m_MFDRboundary);
	AppendToData("\r\n");
	//($this->sendDispositionHeader($h);)
	AppendToData("Content-Disposition: form-data; name=\"");
	AppendToData("key");
	AppendToData("\"");
	//($this->sendTransferEncodingHeader($h); )
	AppendToData("\r\n");
	AppendToData("Content-Transfer-Encoding: ");
	AppendToData("8bit");				//??"binary"
	//($this->sendEndOfHeader($h);)
	AppendToData("\r\n");
	AppendToData("\r\n");
	//($this->sendData($h);)
	AppendToData(DEVKEY_IMAGESHACK);
	//($this->sendEnd($h);)
	AppendToData("\r\n");

//POST DATA Exit
	//$postdata = "--" . part::getBoundary() . "--\r\n";
	AppendToData("--");
	AppendToData(m_MFDRboundary);
	AppendToData("--\r\n");

//start upload thread
	if(m_bAsync){
		mir_forkthread(&CSendImageShack::SendThreadWrapper, this);
		return 0;
	}
	SendThread();
	return 1;
}

void CSendImageShack::SendThread() {
	//send DATA and wait for m_nlreply
	m_nlreply = (NETLIBHTTPREQUEST *) CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM) hNetlibUser, (LPARAM) &m_nlhr);
	mir_freeAndNil(m_nlhr.pData);
	mir_freeAndNil(m_nlhr.headers);
	if(m_nlreply){
		if( m_nlreply->resultCode >= 200 && m_nlreply->resultCode < 300 ){
			m_nlreply->pData[m_nlreply->dataLength] = 0;// make sure its null terminated
			const char* URL = NULL;
			URL = GetTagContent(m_nlreply->pData, "<image_link>", "</image_link>");
			if (URL && URL[0]!= NULL) {
				m_Url = mir_strdup(URL);
				if(m_bSilent)
					return;
				svcSendMsgExit(URL); return;
			}else{//check error mess from server
				TCHAR* err = mir_a2t(GetTagContent(m_nlreply->pData, "<error ", "</error>"));
				if (!err || !*err){//fallback to server response mess
					mir_freeAndNil(err);
					err = mir_a2t(m_nlreply->pData);
				}
				Error(_T("%s"),err);
				mir_free(err);
			}
		}else{
			Error(LPGENT("Upload server did not respond timely."));
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM) m_nlreply);
		m_nlreply = NULL;
	}else{
		Error(SS_ERR_INIT, m_pszSendTyp);
	}
	Exit(ACKRESULT_FAILED);
}

void CSendImageShack::SendThreadWrapper(void * Obj) {
	reinterpret_cast<CSendImageShack*>(Obj)->SendThread();
}

//---------------------------------------------------------------------------
void CSendImageShack::MFDR_Reset() {
	char	Temp[64];
	DWORD	dwBoundaryRand1 = GetTickCount();
	DWORD	dwBoundaryRand2 = rand();

	mir_freeAndNil(m_MFDRboundary);
	mir_snprintf(Temp, SIZEOF(Temp), "B-O-U-N-D-A-R-Y%u%u", dwBoundaryRand1, dwBoundaryRand2);
	mir_stradd(m_MFDRboundary,Temp);
}

void CSendImageShack::GetContentType() {
	if (m_pszContentType) mir_freeAndNil(m_pszContentType);
	char* FileExtension = GetFileExtA(m_pszFile);

	if ((strcmp(FileExtension, ".jpeg")==0) || (strcmp(FileExtension, ".jpe")==0) || (strcmp(FileExtension ,".jpg")==0))
		m_pszContentType="image/jpeg";
	else if (strcmp(FileExtension, ".bmp")==0)
		m_pszContentType="image/bmp";
	else if (strcmp(FileExtension, ".png")==0)
		m_pszContentType="image/png";
	else if (strcmp(FileExtension, ".gif")==0)
		m_pszContentType="image/gif";
	else if ((strcmp(FileExtension, ".tif")==0) || (strcmp(FileExtension, ".tiff")==0))
		m_pszContentType="image/tiff";
	else
		m_pszContentType="application/octet-stream";

	mir_free(FileExtension);
	return;
}

void CSendImageShack::AppendToData(const char *pszVal) {
	if (!m_nlhr.pData) {
		m_nlhr.pData = mir_strdup(pszVal);
		m_nlhr.dataLength = (int)strlen(pszVal);
	}
	else {
		size_t lenVal   = strlen(pszVal);
		size_t sizeNew  = sizeof(char)*(m_nlhr.dataLength + lenVal + 1);
		m_nlhr.pData    = (char*) mir_realloc(m_nlhr.pData, sizeNew);

		strcpy(m_nlhr.pData + sizeof(char)*m_nlhr.dataLength, pszVal);
		m_nlhr.pData[sizeNew-1] = 0;
		m_nlhr.dataLength = (int)sizeNew -1;
	}
}

//---------------------------------------------------------------------------
const char * CSendImageShack::GetTagContent(char * pszSource, const char * pszTagStart, const char * pszTagEnd) {
	char * b = strstr(pszSource, pszTagStart);
	if (!b) return NULL;
	b += strlen(pszTagStart);
	char * e = strstr(b, pszTagEnd);
	if (e) *e = 0;
	return b;
}
