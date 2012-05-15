/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-11  George Hazan
Copyright ( C ) 2007     Maxim Mluhov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Revision       : $Revision: 13444 $
Last change on : $Date: 2011-03-13 23:44:13 +0300 (Вс, 13 мар 2011) $
Last change by : $Author: borkra $

*/

#include "jabber.h"

struct CAPTCHA_FORM_PARAMS
{
	LPCTSTR from;
	LPCTSTR challenge;
	LPCTSTR fromjid;
	LPCTSTR sid;
	LPCTSTR to;
	LPCTSTR hint;
	HBITMAP bmp;
	int w,h;
	TCHAR Result[MAX_PATH];
};

INT_PTR CALLBACK JabberCaptchaFormDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	CAPTCHA_FORM_PARAMS *params = (CAPTCHA_FORM_PARAMS*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	switch (msg) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );
		SendMessage( hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIconBig(IDI_KEYS));
		SendMessage( hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedIcon(IDI_KEYS));
		params = (CAPTCHA_FORM_PARAMS*)lParam;

		LPCTSTR hint = params->hint;
		if( hint == NULL )
			hint = TranslateT("Enter the text you see");
		SetDlgItemText( hwndDlg, IDC_INSTRUCTION, TranslateTS( hint ) );
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, ( LONG )params );

		return TRUE;
	}
	case WM_CTLCOLORSTATIC:
		switch( GetWindowLongPtr((HWND)lParam, GWL_ID)) {
		case IDC_WHITERECT:
		case IDC_INSTRUCTION:
		case IDC_TITLE:
			return (BOOL)GetStockObject(WHITE_BRUSH);
		}
		return NULL;

	case WM_PAINT: 
		if ( params ) {
			PAINTSTRUCT ps;
			HDC hdc, hdcMem;
			RECT rc;

			GetClientRect( hwndDlg, &rc );
			hdc = BeginPaint( hwndDlg, &ps );
			hdcMem = CreateCompatibleDC( hdc );
			HGDIOBJ hOld = SelectObject( hdcMem, params->bmp );

			int y = ( rc.bottom + rc.top - params->h ) / 2;
			int x = ( rc.right + rc.left - params->w ) / 2;
			BitBlt( hdc, x, y, params->w, params->h, hdcMem, 0,0, SRCCOPY );
			SelectObject( hdcMem, hOld );
			DeleteDC( hdcMem );

			EndPaint( hwndDlg, &ps );
		}
		break;

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDCANCEL:
			EndDialog( hwndDlg, 0 );
			return TRUE;

		case IDC_SUBMIT:
			GetDlgItemText( hwndDlg, IDC_VALUE, params->Result, SIZEOF(params->Result) );
			EndDialog( hwndDlg, 1 );
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog( hwndDlg, 0 );
		break;

	case WM_DESTROY:
		WindowFreeIcon( hwndDlg );
		break;
	}
	return FALSE;
}

bool CJabberProto::ProcessCaptcha (HXML node, HXML parentNode, ThreadData* info ){
	CAPTCHA_FORM_PARAMS param;
	char *ImageBuf = 0;
	const TCHAR *PicType = 0;
	TCHAR *CaptchaPath = 0;

	HXML x = xmlGetChildByTag( node, "x", "xmlns", _T(JABBER_FEAT_DATA_FORMS));
	if ( x == NULL )
		return false;

	HXML y = xmlGetChildByTag(x, _T("field"), _T("var"), _T("from"));
	if ( y == NULL )
		return false;
	if (( y = xmlGetChild( y, "value" )) == NULL )
		return false;
	param.fromjid = xmlGetText( y );

	if (( y = xmlGetChildByTag(x, _T("field"), _T("var"), _T("sid"))) == NULL )
		return false;
	if (( y = xmlGetChild( y, "value" )) == NULL )
		return false;
	param.sid = xmlGetText( y );

	if (( y = xmlGetChildByTag(x, _T("field"), _T("var"), _T("ocr"))) == NULL )
		return false;
	param.hint = xmlGetAttrValue (y, _T("label"));

	param.from = xmlGetAttrValue( parentNode, _T("from"));
	param.to = xmlGetAttrValue( parentNode, _T("to"));
	param.challenge = xmlGetAttrValue( parentNode, _T("id"));
	HXML o = xmlGetChild( parentNode, "data" );
	if ( o == NULL || xmlGetText( o ) == NULL )
		return false;

	GetCaptchaImage(parentNode, ImageBuf, PicType, CaptchaPath);
	char* p = mir_t2a( CaptchaPath );
	param.bmp = ( HBITMAP ) JCallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )p );
	DeleteFile(CaptchaPath);
	mir_free(CaptchaPath);
	mir_free(p);

	BITMAP bmp = {0};
	GetObject( param.bmp, sizeof(bmp), &bmp );
	param.w = bmp.bmWidth;
	param.h = bmp.bmHeight;
	int res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CAPTCHAFORM), NULL, JabberCaptchaFormDlgProc, (LPARAM)&param);
	if (lstrcmp(param.Result, _T("")) == 0 || !res)
		sendCaptchaError(info, param.from, param.to, param.challenge);
	else
		sendCaptchaResult (param.Result, info, param.from, param.challenge, param.fromjid, param.sid);
	return true;
}

void CJabberProto::GetCaptchaImage ( HXML node, char *ImageBuf, const TCHAR *PicType, TCHAR*& CaptchaPath ){
	HXML o = xmlGetChild( node , "data" );
	int bufferLen;
	char* buffer = JabberBase64DecodeT(xmlGetText( o ), &bufferLen );
	if ( buffer == NULL )
		return;

	const TCHAR* szPicType;
	HXML m = xmlGetChild( node , "TYPE" );
	if ( m == NULL || xmlGetText( m ) == NULL ) {
	LBL_NoTypeSpecified:
		switch( JabberGetPictureType( buffer )) {
		case PA_FORMAT_GIF:	szPicType = _T("image/gif");	break;
		case PA_FORMAT_BMP:  szPicType = _T("image/bmp");	break;
		case PA_FORMAT_PNG:  szPicType = _T("image/png");	break;
		case PA_FORMAT_JPEG: szPicType = _T("image/jpeg");	break;
		default:
			goto LBL_Ret;
		}
	}
	else {
		const TCHAR* tszType = xmlGetText( m );
		if ( !_tcscmp( tszType, _T("image/jpeg")) ||
			 !_tcscmp( tszType, _T("image/png"))  ||
			 !_tcscmp( tszType, _T("image/gif"))  ||
			 !_tcscmp( tszType, _T("image/bmp")))
			szPicType = tszType;
		else
			goto LBL_NoTypeSpecified;
	}

	DWORD nWritten;

LBL_Ret:
	TCHAR* ext = _tcsstr((TCHAR*)szPicType, _T("/"))+1;
	TCHAR filename[MAX_PATH];
	mir_sntprintf(filename, SIZEOF(filename), _T("%%TEMP%%\\captcha.%s"), ext);
	CaptchaPath = Utils_ReplaceVarsT(filename);
	HANDLE hFile = CreateFile( CaptchaPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		goto LBL_Ret;

	if ( !WriteFile( hFile, buffer, bufferLen, &nWritten, NULL ))
		goto LBL_Ret;

	CloseHandle( hFile );

	ImageBuf = buffer;
	PicType = szPicType;
}

void CJabberProto::sendCaptchaResult(TCHAR* buf, ThreadData* info, LPCTSTR from, LPCTSTR challenge, LPCTSTR fromjid,  LPCTSTR sid){
	XmlNodeIq iq( _T("set"), SerialNext());
	HXML query= iq <<XATTR(_T("to"), from) << XCHILD(_T("captcha")) << XATTR( _T("xmlns"), _T("urn:xmpp:captcha")) << XCHILD (_T("x")) << XATTR(_T("xmlns"), _T(JABBER_FEAT_DATA_FORMS)) << XATTR(_T("type"), _T("submit"));
		query << XCHILD(_T("field")) << XATTR (_T("var"), _T("FORM_TYPE")) << XCHILD(_T("value"), _T("urn:xmpp:captcha"));
		query << XCHILD(_T("field")) << XATTR (_T("var"), _T("from")) << XCHILD(_T("value"), fromjid);
		query << XCHILD(_T("field")) << XATTR (_T("var"), _T("challenge")) << XCHILD(_T("value"), challenge);
		query << XCHILD(_T("field")) << XATTR (_T("var"), _T("sid")) << XCHILD(_T("value"), sid);
		query << XCHILD(_T("field")) << XATTR (_T("var"), _T("ocr")) << XCHILD(_T("value"), buf);
	info -> send (iq);
}

void CJabberProto::sendCaptchaError(ThreadData* info, LPCTSTR from, LPCTSTR to, LPCTSTR challenge ){
	XmlNode message( _T("message"));
	HXML query= message << XATTR(_T("type"), _T("error")) << XATTR(_T("to"), from) << XATTR(_T("id"), challenge) << XATTR(_T("from"), to)
		  << XCHILD(_T("error")) << XATTR(_T("type"), _T("modify"))
	      << XCHILD(_T("not-acceptable")) << XATTR(_T("xmlns"), _T("urn:ietf:params:xml:ns:xmpp-stanzas"));
	info -> send (message);
}
