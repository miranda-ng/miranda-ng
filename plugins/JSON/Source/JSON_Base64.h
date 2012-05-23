#ifndef JSON_BASE64_H
#define JSON_BASE64_H

#include "JSONDebug.h"
#ifdef JSON_BINARY  //if this is not needed, don't waste space compiling it

class JSONBase64 {
public:
    static json_string json_encode64(const unsigned char * binary, size_t bytes);
    static std::string json_decode64(const json_string & encoded);
};

#endif
#endif
