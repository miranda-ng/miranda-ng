#ifndef _TOX_ADDRESS_H_
#define _TOX_ADDRESS_H_

class ToxHexAddress;
class ToxBinAddress;

class ToxHexAddress
{
private:
	std::string hexData;
public:
	ToxHexAddress(const char *hex) : hexData(hex) { }
	ToxHexAddress(const std::string &hex) : hexData(hex) { }
	ToxHexAddress(const ToxHexAddress &address) : hexData(address.hexData) { }
	ToxHexAddress(const std::vector<uint8_t> &bin)
	{
		this->ToxHexAddress::ToxHexAddress(bin.data(), bin.size());
	}
	ToxHexAddress(const uint8_t *bin, size_t size = TOX_FRIEND_ADDRESS_SIZE)
	{
		char *hex = (char*)mir_alloc(size * 2 + 1);
		hexData = bin2hex(bin, size, hex);
		std::transform(hexData.begin(), hexData.end(), hexData.begin(), ::toupper);
		mir_free(hex);
	}
	const size_t GetLength() const
	{
		return hexData.length();
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
	ToxBinAddress ToBin() const;
};

class ToxBinAddress
{
private:
	std::vector<uint8_t> binData;
public:
	ToxBinAddress(const ToxBinAddress &address) : binData(address.binData) { }
	ToxBinAddress(const std::vector<uint8_t> &bin) : binData(bin) { }
	ToxBinAddress(const uint8_t *bin, size_t size = TOX_FRIEND_ADDRESS_SIZE) : binData(bin, bin + size) { }
	ToxBinAddress(const std::string &hex)
	{
		this->ToxBinAddress::ToxBinAddress(hex.c_str());
	}
	ToxBinAddress(const char *hex)
	{
		char *endptr;
		const char *pos = hex;
		size_t size = mir_strlen(hex) / 2;
		for (size_t i = 0; i < size; i++)
		{
			char buf[5] = { '0', 'x', pos[0], pos[1], 0 };
			binData.push_back((uint8_t)strtol(buf, &endptr, 0));
			pos += 2;
		}
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