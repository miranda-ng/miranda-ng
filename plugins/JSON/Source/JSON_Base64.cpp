#include "JSON_Base64.h"
#include "JSONDefs.h"

#ifdef JSON_BINARY  //if this is not needed, don't waste space compiling it

static const json_char * chars64 = JSON_TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
#ifdef JSON_UNICODE
    static std::string BASE64_EMPTY_STRING;
#else
    #define BASE64_EMPTY_STRING EMPTY_STRING
#endif

json_string JSONBase64::json_encode64(const unsigned char * binary, size_t bytes){
    size_t misaligned = bytes % 3;
    size_t aligned = (bytes - misaligned) / 3;  //divide by three once and inc is faster than add 3 each time
    json_string result;
    result.reserve((size_t)(((float)bytes) * 1.37f) + 4);
    
    //do all of the ones that are 3 byte aligned
    for (size_t i = 0; i < aligned; ++i){
	   result += chars64[(binary[0] & 0xFC) >> 2];
	   result += chars64[((binary[0] & 0x03) << 4) + ((binary[1] & 0xF0) >> 4)];
	   result += chars64[((binary[1] & 0x0F) << 2) + ((binary[2] & 0xC0) >> 6)];
	   result += chars64[binary[2] & 0x3F];
	   binary += 3;
    }
    
    if (misaligned){
	   //copy the rest into a temporary buffer
	   unsigned char temp[3];
	   for (unsigned int i = 0; i < misaligned; ++i){
		  temp[i] = *binary++;
	   }
	   for (unsigned int i = (unsigned int)misaligned; i < 3; ++i){
		  temp[i] = '\0';
	   }
	   
	   //now do the final three bytes
	   result += chars64[(temp[0] & 0xFC) >> 2];
	   result += chars64[((temp[0] & 0x03) << 4) + ((temp[1] & 0xF0) >> 4)];
	   if (misaligned == 2){
		  result += chars64[((temp[1] & 0x0F) << 2) + ((temp[2] & 0xC0) >> 6)];
		  result += JSON_TEXT('=');
	   } else {
		  result += JSON_TEXT("==");
	   }
    }
    JSON_ASSERT((size_t)(((float)bytes) * 1.37f) + 4 >= result.length(), JSON_TEXT("Didn't reserve enough space for a one-time go"));
    return result;
}

inline json_char toBinary(json_char c){
    if (c == JSON_TEXT('+')){
	   return JSON_TEXT('>');
    } else if (c == JSON_TEXT('/')){
	   return JSON_TEXT('?');
    } else if (c < JSON_TEXT(':')){
	   return c + JSON_TEXT('\x04');
    } else if (c < JSON_TEXT('[')){
	   return c - JSON_TEXT('\x41');
    }
    return c - 71;
}

/*
 Must be a std::string because it's binary, and chars must be 1 byte
 */
std::string JSONBase64::json_decode64(const json_string & encoded){
    const size_t length = encoded.length();
    #if defined JSON_DEBUG || defined JSON_SAFE
	   size_t pos = encoded.find_first_not_of(chars64);
	   if (pos != json_string::npos){
		  JSON_ASSERT_SAFE(encoded[pos] == JSON_TEXT('='), JSON_TEXT("Not Base64"), return BASE64_EMPTY_STRING;);
		  if (pos != length - 1){
			 JSON_ASSERT_SAFE(pos == length - 2, JSON_TEXT("Not Base64"), return BASE64_EMPTY_STRING;);
			 JSON_ASSERT_SAFE(encoded[pos + 1] == JSON_TEXT('='), JSON_TEXT("Not Base64"), return BASE64_EMPTY_STRING;);
		  }
	   }
    #endif
    const json_char * runner = encoded.c_str();
    size_t aligned = length / 4; //don't do the last ones as they might be = padding
    std::string result;
    if (aligned){
	   --aligned;
	   result.reserve((size_t)((float)length / 1.37) + 1);
	   
	   //first do the ones that can not have any padding
	   for (unsigned int i = 0; i < aligned; ++i){
		  const json_char second = toBinary(runner[1]);
		  const json_char third = toBinary(runner[2]);
		  result += (toBinary(runner[0]) << 2) + ((second & 0x30) >> 4);
		  result += ((second & 0xf) << 4) + ((third & 0x3c) >> 2);
		  result += ((third & 0x3) << 6) + toBinary(runner[3]);
		  runner += 4;
	   }
	   
	   //now do the ones that might have padding, the first two characters can not be padding, so do them quickly
	   const char second = toBinary(runner[1]);
	   result += (toBinary(runner[0]) << 2) + ((second & 0x30) >> 4);
	   if (runner[2] != '='){  //not two = pads
		  const char third = toBinary(runner[2]);
		  result += ((second & 0xf) << 4) + ((third & 0x3c) >> 2);
		  if (runner[3] != '='){  //no padding
			 result += ((third & 0x3) << 6) + toBinary(runner[3]);
		  }
	   }
    }
    return result;
}

#endif
