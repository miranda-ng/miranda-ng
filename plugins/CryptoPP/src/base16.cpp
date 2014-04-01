#include "commonheaders.h"

char *base16encode(const char *inBuffer, int count)
{
	char *outBuffer = (char *)malloc(count * 2 + 1);
	char *outBufferPtr = outBuffer;
	BYTE *inBufferPtr = (BYTE *)inBuffer;

	while (count) {
		*outBufferPtr++ = encode16(((*inBufferPtr) >> 4) & 0x0F);
		*outBufferPtr++ = encode16((*inBufferPtr++) & 0x0F);
		count--;
	}
	*outBufferPtr = '\0';

	return outBuffer;
}

char *base16decode(const char *inBuffer, unsigned int *count)
{
	char *outBuffer = (char *)mir_alloc(*count);
	BYTE *outBufferPtr = (BYTE *)outBuffer;
	bool big_endian = false;

	if (*inBuffer == '0' && *(inBuffer + 1) == 'x') {
		inBuffer += *count;
		big_endian = true;
		*count -= 2;
	}
	while (*count > 1) {
		BYTE c0, c1;
		if (big_endian) {
			c1 = decode16(*--inBuffer);
			c0 = decode16(*--inBuffer);
		}
		else {
			c0 = decode16(*inBuffer++);
			c1 = decode16(*inBuffer++);
		}
		if ((c0 | c1) == BERR) {
			free(outBuffer);
			*count = 0;
			return(NULL);
		}
		*outBufferPtr++ = (c0 << 4) | c1;
		*count -= 2;
	}
	*outBufferPtr = '\0';
	*count = (int)(outBufferPtr - (BYTE *)outBuffer);

	return outBuffer;
}

char *base16decode(const char *inBuffer)
{
	unsigned count = (unsigned)strlen(inBuffer);
	return base16decode(inBuffer, &count);
}
