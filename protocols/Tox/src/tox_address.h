#ifndef _TOX_ADDRESS_H_
#define _TOX_ADDRESS_H_

class ToxHexAddress;
class ToxBinAddress;

class ToxHexAddress
{
private:
	size_t hexSize;
	char *hexData;
	void Init(const char *hex, size_t size)
	{
		hexSize = size;
		hexData = (char*)mir_calloc(hexSize + 1);
		memcpy(hexData, hex, hexSize);
	}
	void Init(const uint8_t *bin, size_t size)
	{
		hexSize = size * 2;
		hexData = (char*)mir_calloc(hexSize + 1);
		char *p = bin2hex(bin, size, hexData);
		while (*p++ = toupper(*p));
	}
public:
	ToxHexAddress(const char *hex, size_t size = TOX_ADDRESS_SIZE * 2) { Init(hex, size); }
	ToxHexAddress(const std::string &hex) { Init(hex.c_str(), hex.size()); }
	ToxHexAddress(const uint8_t *bin, size_t size = TOX_ADDRESS_SIZE) { Init(bin, size); }
	ToxHexAddress(const ToxHexAddress &address) : hexData(address.hexData) { Init(address.hexData, address.hexSize); }
	~ToxHexAddress() { if (hexData) mir_free(hexData); }
	ToxHexAddress& operator=(const char *hex)
	{
		Init(hex, mir_strlen(hex));
		return *this;
	}
	const size_t GetLength() const { return hexSize; }
	const bool IsEmpty() const { return hexData[0] == 0; }
	const ToxHexAddress GetPubKey() const { return ToxHexAddress(hexData, TOX_PUBLIC_KEY_SIZE * 2); }
	operator const char*() const { return hexData; }
	static ToxHexAddress Empty() { return ToxHexAddress("", 0); }
	ToxBinAddress ToBin() const;
};

class ToxBinAddress
{
private:
	size_t binSize;
	uint8_t *binData;
	void Init(const char *hex, size_t size)
	{
		binSize = size / 2;
		binData = (uint8_t*)mir_alloc(binSize);
		hex2bin(hex, binData, binSize);
	}
	void Init(const uint8_t *bin, size_t size)
	{
		binSize = size;
		binData = (uint8_t*)mir_alloc(binSize);
		memcpy(binData, bin, binSize);
	}
public:
	ToxBinAddress(const uint8_t *bin, size_t size = TOX_ADDRESS_SIZE) { Init(bin, size); }
	ToxBinAddress(const char *hex, size_t size = TOX_ADDRESS_SIZE * 2) { Init(hex, size); }
	ToxBinAddress(const std::string &hex) { Init(hex.c_str(), hex.size()); }
	ToxBinAddress(const ToxBinAddress &address) { Init(address.binData, address.binSize); }
	~ToxBinAddress() { if (binData) mir_free(binData); }
	ToxBinAddress& operator=(const char *hex)
	{
		Init(hex, mir_strlen(hex));
		return *this;
	}
	const ToxBinAddress GetPubKey() const { return ToxBinAddress(binData, TOX_PUBLIC_KEY_SIZE); }
	operator const uint8_t*() const { return binData; }
	ToxHexAddress ToHex() const { return ToxHexAddress(binData, binSize); }
};

ToxBinAddress ToxHexAddress::ToBin() const { return ToxBinAddress(hexData, hexSize); }

#endif //_TOX_ADDRESS_H_