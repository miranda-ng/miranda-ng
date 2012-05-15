#include "commonheaders.h"


string base64encode(const string buf) {
	string out;
	char *base64 = base64encode(buf.data(), buf.length());
	out.assign(base64);
	free(base64);
	return out;
}


char *base64encode(const char *inBuffer, const int count) {

	int srcIndex = 0, destIndex = 0, remainder = count % 3;
	char *outBuffer = (char *) malloc(count*2+1);
	BYTE *inBufferPtr = (BYTE *) inBuffer;

	while(srcIndex < count) {
		outBuffer[destIndex++] = encode64(inBufferPtr[srcIndex] >> 2);
		outBuffer[destIndex++] = encode64(((inBufferPtr[srcIndex] << 4) & 0x30) | ((inBufferPtr[srcIndex + 1] >> 4) & 0x0F));
		srcIndex++;
		outBuffer[destIndex++] = encode64(((inBufferPtr[srcIndex] << 2) & 0x3C) | ((inBufferPtr[srcIndex + 1] >> 6) & 0x03));
		srcIndex++;
		outBuffer[destIndex++] = encode64(inBufferPtr[srcIndex++] & 0x3F);
	}

	if(remainder == 2) {
		outBuffer[destIndex - 1] = BPAD;
		outBuffer[destIndex - 2] = encode64((inBufferPtr[srcIndex - 2] << 2) & 0x3C);
	}
	else if(remainder == 1) {
		outBuffer[destIndex - 2] = outBuffer[destIndex - 1] = BPAD;
		outBuffer[destIndex - 3] = encode64((inBufferPtr[srcIndex - 3] << 4) & 0x30);
	}
	destIndex -= (3 - remainder) % 3;
	outBuffer[destIndex] = '\0';

	return outBuffer;
}


string base64decode(const string buf) {
	string out;
	int len = buf.length();
	char *plain = base64decode(buf.data(), &len);
	out.assign(plain,len);
	free(plain);
	return out;
}


string base64decode(const char *buf) {
	string out;
	int len = strlen(buf);
	char *plain = base64decode(buf, &len);
	out.assign(plain,len);
	free(plain);
	return out;
}


char *base64decode(const char *inBuffer, int *count) {

	int srcIndex = 0, destIndex = 0;
	char *outBuffer = (char *) malloc(*count);

	while(srcIndex < *count) {
		BYTE c0, c1, c2 = 0, c3 = 0;
		const int delta = *count - srcIndex;
		c0 = decode64(inBuffer[srcIndex++]);
		c1 = decode64(inBuffer[srcIndex++]);
		if(delta > 2) {
			c2 = decode64(inBuffer[srcIndex++]);
			if(delta > 3)
				c3 = decode64(inBuffer[srcIndex++]);
		}
		if((c0 | c1 | c2 | c3) == BERR) {
			free(outBuffer);
			return(NULL);
		}
		outBuffer[destIndex++] = (c0 << 2) | (c1 >> 4);
		if(delta > 2) {
			outBuffer[destIndex++] = (c1 << 4) | (c2 >> 2);
			if(delta > 3 )
				outBuffer[destIndex++] = (c2 << 6) | (c3);
		}
	}
	outBuffer[destIndex] = '\0';
	*count = destIndex;

	return outBuffer;
}


// EOF
