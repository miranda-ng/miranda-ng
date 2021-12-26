#include "commonheaders.h"

char* base16encode(const char *inBuffer, size_t count)
{
	char *outBuffer = (char *)malloc(count * 2 + 1);
	char *outBufferPtr = outBuffer;
	uint8_t *inBufferPtr = (uint8_t *)inBuffer;

	while (count) {
		*outBufferPtr++ = encode16(((*inBufferPtr) >> 4) & 0x0F);
		*outBufferPtr++ = encode16((*inBufferPtr++) & 0x0F);
		count--;
	}
	*outBufferPtr = '\0';

	return outBuffer;
}

char* base16decode(const char *inBuffer, size_t *count)
{
	char *outBuffer = (char *)mir_alloc(*count);
	uint8_t *outBufferPtr = (uint8_t *)outBuffer;
	bool big_endian = false;

	if (*inBuffer == '0' && *(inBuffer + 1) == 'x') {
		inBuffer += *count;
		big_endian = true;
		*count -= 2;
	}
	while (*count > 1) {
		uint8_t c0, c1;
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
			return(nullptr);
		}
		*outBufferPtr++ = (c0 << 4) | c1;
		*count -= 2;
	}
	*outBufferPtr = '\0';
	*count = (int)(outBufferPtr - (uint8_t *)outBuffer);

	return outBuffer;
}

char *base16decode(const char *inBuffer)
{
	size_t count = strlen(inBuffer);
	return base16decode(inBuffer, &count);
}
