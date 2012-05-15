#ifndef __CPP_RSAU_H__
#define __CPP_RSAU_H__

class TLV {
private:
	map<u_char,string> data;

public:
	TLV();
	TLV(const string& b) { decode(b); };

	string encode() {
		map<u_char,string>::const_iterator i;
		string b;
		for( i=data.begin(); i!=data.end(); ++i ) {
			b += tlv(i->first,i->second);
		}
		return b;
	};
	void decode(const string& b) {
		u_char t;
		string v;
		string r = b;
		while( r.length() ) {
			t = un_tlv(r,v);
			if( t==-1 ) {
				break;
			}
			data[t] = v;
		}
	};

	u_int count() { return data.size(); };
	bool exist(u_char t) { return (data.find(t) != data.end()); };
	string get(u_char t) { return data[t]; };
	void put(u_char t, string& v) { data[t]=v; };

private:
	string tlv(u_int t, const string& v) {
		string b;
		t |= v.length()<<8;
		b.assign((const char*)&t,3);
		b += v;
		return b;
	};
	u_char un_tlv(string& b, string& v) {
		string r; v = r;
		u_int t = 0;
		if( b.length() > 3 ) {
			b.copy((char*)&t,3);
			u_int l = t>>8;
			t &= 0xFF;
			if( b.length() >= 3+l ) {
		  		v = b.substr(3,l);
		  		r = b.substr(3+l);
			}
		}
		if( !v.length() ) {
			return -1;
		}
		b = r;
		return t;
	};
};

string pad256(string&);

string& add_delim(string&,const string&,int);
string& del_delim(string&,const string&);

string tlv(int,const string&);
string tlv(int,const char*);
string tlv(int,int);
string& un_tlv(string&,int&,string&);
string& un_tlv(string&,int&,int&);

int str2int(string&);

string hash(string&);
string hash(PBYTE,int);

string hash128(string&);
string hash128(LPSTR);
string hash128(PBYTE,int);

string hash256(string&);
string hash256(LPSTR);
string hash256(PBYTE,int);

Integer BinaryToInteger(const string&);
string	IntegerToBinary(const Integer&);

AutoSeededRandomPool& GlobalRNG();

void	GenerateRSAKey(unsigned int,string&,string&);
string	RSAEncryptString(const RSA::PublicKey&,const string&);
string  RSADecryptString(const RSA::PrivateKey&,const string&);
string	RSASignString(const RSA::PrivateKey&,const string&);
BOOL	RSAVerifyString(const RSA::PublicKey&,const string&,const string&);

#endif
