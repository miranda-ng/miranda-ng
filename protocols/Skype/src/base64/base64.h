#include <windows.h>

class Base64
{
private:
	static char CharBase64[];
	static int	IndexBase64[];

public:	
	static ULONG Encode(const char *inputString, char *outputBuffer, SIZE_T nMaxLength);
	static ULONG Decode(const char *inputString, char *outputBuffer, SIZE_T nMaxLength);
};