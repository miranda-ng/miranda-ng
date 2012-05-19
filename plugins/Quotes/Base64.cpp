#include "StdAfx.h"
#include "Base64.h"

bool base64::encode(const BYTE* in, size_t inlen,std::vector<char>& out)
{
	int nOutLength = Base64EncodeGetRequiredLength((int)inlen);
	out.resize(nOutLength);
	char* p = &*out.begin();
	bool bResult = (TRUE == Base64Encode(in,(int)inlen,p,&nOutLength));
	if(false == bResult)
	{
		out.resize(nOutLength);
		p = &*out.begin();
		bResult = (TRUE == Base64Encode(in,(int)inlen,p,&nOutLength));
	}
	if(bResult)
	{
		out.resize(nOutLength);
	}

	return bResult;
}


bool base64::decode(const char* in, size_t inlen,std::vector<BYTE>& out)
{
	int nOutLength = (int)inlen;
	out.resize(nOutLength);
	BYTE* p = &*out.begin();
	bool bResult = TRUE == Base64Decode(in,(int)inlen,p,&nOutLength);
	if(false == bResult)
	{
		out.resize(nOutLength);
		p = &*out.begin();
		bResult = TRUE == Base64Decode(in,(int)inlen,p,&nOutLength);
	}
	if(bResult)
	{
		out.resize(nOutLength);
	}

	return true;
}
