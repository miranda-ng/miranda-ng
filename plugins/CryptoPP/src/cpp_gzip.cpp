#include "commonheaders.h"

// gzip data
uint8_t *cpp_gzip(uint8_t *pData, size_t nLen, size_t& nCompressedLen)
{
	string zipped;
	Gzip gzip(new StringSink(zipped), 5);    // 1 is fast, 9 is slow
	gzip.Put(pData, nLen);
	gzip.MessageEnd();

	nCompressedLen = (int)zipped.length();
	uint8_t *pCompressed = (uint8_t*)malloc(nCompressedLen + 1);
	memcpy(pCompressed, zipped.data(), nCompressedLen);

	return pCompressed;
}

// gunzip data
uint8_t *cpp_gunzip(uint8_t *pCompressedData, size_t nCompressedLen, size_t& nLen)
{
	string unzipped;
	Gunzip gunzip(new StringSink(unzipped));
	gunzip.Put((uint8_t*)pCompressedData, nCompressedLen);
	gunzip.MessageEnd();

	nLen = (int)unzipped.length();
	uint8_t *pData = (uint8_t*)malloc(nLen + 1);
	memcpy(pData, unzipped.data(), nLen);

	return pData;
}

// zlibc data
string cpp_zlibc(string& pData)
{
	string zipped;
	ZlibCompressor zlib(new StringSink(zipped), 5);    // 1 is fast, 9 is slow
	zlib.Put((uint8_t*)pData.data(), pData.length());
	zlib.MessageEnd();

	return zipped;
}

// zlibd data
string cpp_zlibd(string& pData)
{
	string unzipped;
	ZlibDecompressor zlib(new StringSink(unzipped));
	zlib.Put((uint8_t*)pData.data(), pData.length());
	zlib.MessageEnd();

	return unzipped;
}
