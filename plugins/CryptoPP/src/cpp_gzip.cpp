#include "commonheaders.h"

// gzip data
BYTE *cpp_gzip(BYTE *pData, size_t nLen, size_t& nCompressedLen)
{
	string zipped;
	Gzip gzip(new StringSink(zipped), 5);    // 1 is fast, 9 is slow
	gzip.Put(pData, nLen);
	gzip.MessageEnd();

	nCompressedLen = (int)zipped.length();
	PBYTE pCompressed = (PBYTE)malloc(nCompressedLen + 1);
	memcpy(pCompressed, zipped.data(), nCompressedLen);

	return pCompressed;
}

// gunzip data
BYTE *cpp_gunzip(BYTE *pCompressedData, size_t nCompressedLen, size_t& nLen)
{
	string unzipped;
	Gunzip gunzip(new StringSink(unzipped));
	gunzip.Put((PBYTE)pCompressedData, nCompressedLen);
	gunzip.MessageEnd();

	nLen = (int)unzipped.length();
	PBYTE pData = (PBYTE)malloc(nLen + 1);
	memcpy(pData, unzipped.data(), nLen);

	return pData;
}

// zlibc data
string cpp_zlibc(string& pData)
{
	string zipped;
	ZlibCompressor zlib(new StringSink(zipped), 5);    // 1 is fast, 9 is slow
	zlib.Put((PBYTE)pData.data(), pData.length());
	zlib.MessageEnd();

	return zipped;
}

// zlibd data
string cpp_zlibd(string& pData)
{
	string unzipped;
	ZlibDecompressor zlib(new StringSink(unzipped));
	zlib.Put((PBYTE)pData.data(), pData.length());
	zlib.MessageEnd();

	return unzipped;
}
