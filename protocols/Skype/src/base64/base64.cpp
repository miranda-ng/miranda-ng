#include "base64.h"

char Base64::CharBase64[] = 
{
	'A','B','C','D','E','F','G','H','I','J','K','L','M  ','N','O','P',
	'Q','R','S','T','U','V','W','X','Y','Z','a','b','c  ','d','e','f',
	'g','h','i','j','k','l','m','n','o','p','q','r','s  ','t','u','v',
	'w','x','y','z','0','1','2','3','4','5','6','7','8  ','9','+','/'
};

ULONG Base64::Encode(const char *inputString, char *outputBuffer, SIZE_T nMaxLength)
{
	int outpos = 0;
	char chr[3], enc[4];

	for (unsigned int i = 0; i < ::strlen(inputString); i += 3)
	{
		if (outpos + 4 >= nMaxLength)break;

		chr[0] = inputString[i];
		chr[1] = inputString[i+1];
		chr[2] = inputString[i+2];

		enc[0] = chr[0] >> 2;
		enc[1] = ((chr[0] & 0x03) << 4) | (chr[1] >> 4);
		enc[2] = ((chr[1] & 0x0F) << 2) | (chr[2] >> 6);
		enc[3] = chr[2] & 0x3F;

		outputBuffer[outpos++] = Base64::CharBase64[enc[0]];
		outputBuffer[outpos++] = Base64::CharBase64[enc[1]];

		if (i + 1 >= ::strlen(inputString))
		{
			outputBuffer[outpos++] = '=';
			outputBuffer[outpos++] = '=';
		}
		else if (i + 2 >= ::strlen(inputString))
		{
			outputBuffer[outpos++] = Base64::CharBase64[enc[2]];
			outputBuffer[outpos++] = '=';
		}
		else
		{
			outputBuffer[outpos++] = Base64::CharBase64[enc[2]];
			outputBuffer[outpos++] = Base64::CharBase64[enc[3]];
		}
	}

	outputBuffer[outpos] = 0;
	return outpos;
}

int Base64::IndexBase64[] = 
{
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
	52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
	-1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	-1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

ULONG Base64::Decode(const char *inputString, char *outputBuffer, SIZE_T nMaxLength)
{
	int outpos = 0;
	char chr[3], enc[4];	

	for (int i = 0; i < ::strlen(inputString); i += 4)
	{
		if (outpos+3 >= nMaxLength) break;

		enc[0] = Base64::IndexBase64[inputString[i]];
		enc[1] = Base64::IndexBase64[inputString[i+1]];
		enc[2] = Base64::IndexBase64[inputString[i+2]];
		enc[3] = Base64::IndexBase64[inputString[i+3]];

		if (enc[0] == -1 || enc[1] == -1) break;

		chr[0] = (enc[0] << 2) | (enc[1] >> 4);
		chr[1] = ((enc[1] & 15) << 4) | (enc[2] >> 2);
		chr[2] = ((enc[2] & 3) << 6) | enc[3];

		outputBuffer[outpos++] = chr[0];

		if (enc[2] != -1) outputBuffer[outpos++] = chr[1];
		if (enc[3] != -1) outputBuffer[outpos++] = chr[2];
	}

	outputBuffer[outpos] = 0;
	return outpos;
}