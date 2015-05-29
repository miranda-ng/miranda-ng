#ifndef _TOX_ADDRESS_H_
#define _TOX_ADDRESS_H_

class ToxHexAddress;
class ToxBinAddress;

class ToxHexAddress
{
private:
	std::string hexData;
	void Init(const char *hex, size_t size)
	{
		hexData = std::string(hex, size);
	}
	void Init(const uint8_t *bin, size_t size)
	{
		char *hex = (char*)mir_alloc(size * 2 + 1);
		hexData = bin2hex(bin, size, hex);
		std::transform(hexData.begin(), hexData.end(), hexData.begin(), ::toupper);
		mir_free(hex);
	}
public:
	ToxHexAddress(const ToxHexAddress &address) : hexData(address.hexData) { }
	ToxHexAddress(const char *hex, size_t size = TOX_ADDRESS_SIZE * 2) : hexData(hex, hex + size) { }
	ToxHexAddress(const std::string &hex)
	{
		Init(hex.c_str(), hex.size());
	}
	ToxHexAddress(const uint8_t *bin, size_t size = TOX_ADDRESS_SIZE)
	{
		Init(bin, size);
	}
	ToxHexAddress(const std::vector<uint8_t> &bin)
	{
		Init(bin.data(), bin.size());
	}
	ToxHexAddress& operator=(const char *hex)
	{
		Init(hex, mir_strlen(hex));
		return *this;
	}
	const size_t GetLength() const
	{
		return hexData.length();
	}
	const bool IsEmpty() const
	{
		return hexData.empty();
	}
	const ToxHexAddress GetPubKey() const
	{
		ToxHexAddress pubKey = hexData.substr(0, TOX_PUBLIC_KEY_SIZE * 2).c_str();
		return pubKey;
	}
	operator const char*() const
	{
		return hexData.c_str();
	}
	static ToxHexAddress Empty()
	{
		return ToxHexAddress("", 0);
	}
	ToxBinAddress ToBin() const;
};

class ToxBinAddress
{
private:
	std::vector<uint8_t> binData;
	void Init(const char *hex, size_t size)
	{
		char *endptr;
		const char *pos = hex;
		size /= 2; binData.resize(size);
		for (size_t i = 0; i < size; i++)
		{
			char buf[5] = { '0', 'x', pos[0], pos[1], 0 };
			binData[i] = (uint8_t)strtol(buf, &endptr, 0);
			pos += 2;
		}
	}
public:
	ToxBinAddress(const ToxBinAddress &address) : binData(address.binData) { }
	ToxBinAddress(const uint8_t *bin, size_t size = TOX_ADDRESS_SIZE) : binData(bin, bin + size) { }
	ToxBinAddress(const std::vector<uint8_t> &bin, size_t size = TOX_ADDRESS_SIZE) : binData(bin.begin(), bin.begin() + size) { }
	ToxBinAddress(const char *hex, size_t size = TOX_ADDRESS_SIZE * 2)
	{
		Init(hex, size);
	}
	ToxBinAddress(const std::string &hex)
	{
		Init(hex.c_str(), hex.size());
	}
	ToxBinAddress& operator=(const char *hex)
	{
		Init(hex, mir_strlen(hex));
		return *this;
	}
	const ToxBinAddress GetPubKey() const
	{
		ToxBinAddress pubKey(binData.data(), TOX_PUBLIC_KEY_SIZE);
		return pubKey;
	}
	operator const uint8_t*() const
	{
		return binData.data();
	}
	ToxHexAddress ToHex() const
	{
		ToxHexAddress hex(binData.data(), binData.size());
		return hex;
	}
};

ToxBinAddress ToxHexAddress::ToBin() const
{
	ToxBinAddress bin(hexData.c_str());
	return bin;
}

#endif //_TOX_ADDRESS_H_