#ifndef __FBB8ABCA_315E_4ace_B2EB_51E03AD6F8D1_Base64_h__
#define __FBB8ABCA_315E_4ace_B2EB_51E03AD6F8D1_Base64_h__

#pragma once

namespace base64
{
	bool encode(const BYTE* in, size_t inlen,std::vector<char>& out);
	bool decode(const char* in, size_t inlen,std::vector<BYTE>& out);
}

#endif //__FBB8ABCA_315E_4ace_B2EB_51E03AD6F8D1_Base64_h__
