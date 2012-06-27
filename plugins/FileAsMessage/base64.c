#define XNetlib_GetBase64DecodedBufferSize(cchEncoded)  (((cchEncoded)>>2)*3)
#define XNetlib_GetBase64EncodedBufferSize(cbDecoded)  (((cbDecoded)*4+11)/12*4+1)

static char base64chars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int XNetlibBase64Encode(WPARAM wParam,LPARAM lParam)
{
	NETLIBBASE64 *nlb64=(NETLIBBASE64*)lParam;
	int iIn;
	char *pszOut;
	PBYTE pbIn;

	if(nlb64==NULL || nlb64->pszEncoded==NULL || nlb64->pbDecoded==NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if(nlb64->cchEncoded<XNetlib_GetBase64EncodedBufferSize(nlb64->cbDecoded)) {
		SetLastError(ERROR_BUFFER_OVERFLOW);
		return 0;
	}
	nlb64->cchEncoded=XNetlib_GetBase64EncodedBufferSize(nlb64->cbDecoded);
	for(iIn=0,pbIn=nlb64->pbDecoded,pszOut=nlb64->pszEncoded;iIn<nlb64->cbDecoded;iIn+=3,pbIn+=3,pszOut+=4) {
		pszOut[0]=base64chars[pbIn[0]>>2];
		if(nlb64->cbDecoded-iIn==1) {
			pszOut[1]=base64chars[(pbIn[0]&3)<<4];
			pszOut[2]='=';
			pszOut[3]='=';
			pszOut+=4;
			break;
		}
		pszOut[1]=base64chars[((pbIn[0]&3)<<4)|(pbIn[1]>>4)];
		if(nlb64->cbDecoded-iIn==2) {
			pszOut[2]=base64chars[(pbIn[1]&0xF)<<2];
			pszOut[3]='=';
			pszOut+=4;
			break;
		}
		pszOut[2]=base64chars[((pbIn[1]&0xF)<<2)|(pbIn[2]>>6)];
		pszOut[3]=base64chars[pbIn[2]&0x3F];
	}
	pszOut[0]='\0';
	return 1;
}

BYTE Base64CharToInt(char c)
{
	if(c>='A' && c<='Z') return c-'A';
	if(c>='a' && c<='z') return c-'a'+26;
	if(c>='0' && c<='9') return c-'0'+52;
	if(c=='+') return 62;
	if(c=='/') return 63;
	if(c=='=') return 64;
	return 255;
}

int XNetlibBase64Decode(WPARAM wParam,LPARAM lParam)
{
	NETLIBBASE64 *nlb64=(NETLIBBASE64*)lParam;
	char *pszIn;
	PBYTE pbOut;
	BYTE b1,b2,b3,b4;
	int iIn;

	if(nlb64==NULL || nlb64->pszEncoded==NULL || nlb64->pbDecoded==NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if(nlb64->cchEncoded&3) {
		SetLastError(ERROR_INVALID_DATA);
		return 0;
	}
	if(nlb64->cbDecoded<XNetlib_GetBase64DecodedBufferSize(nlb64->cchEncoded)) {
		SetLastError(ERROR_BUFFER_OVERFLOW);
		return 0;
	}
	nlb64->cbDecoded=XNetlib_GetBase64DecodedBufferSize(nlb64->cchEncoded);
	for(iIn=0,pszIn=nlb64->pszEncoded,pbOut=nlb64->pbDecoded;iIn<nlb64->cchEncoded;iIn+=4,pszIn+=4,pbOut+=3) {
		b1=Base64CharToInt(pszIn[0]);
		b2=Base64CharToInt(pszIn[1]);
		b3=Base64CharToInt(pszIn[2]);
		b4=Base64CharToInt(pszIn[3]);
		if(b1==255 || b1==64 || b2==255 || b2==64 || b3==255 || b4==255) {
			SetLastError(ERROR_INVALID_DATA);
			return 0;
		}
		pbOut[0]=(b1<<2)|(b2>>4);
		if(b3==64) {nlb64->cbDecoded-=2; break;}
		pbOut[1]=(b2<<4)|(b3>>2);
		if(b4==64) {nlb64->cbDecoded--; break;}
		pbOut[2]=b4|(b3<<6);
	}
	return 1;
}
