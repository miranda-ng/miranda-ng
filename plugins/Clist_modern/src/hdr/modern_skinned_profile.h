#ifndef modern_skinned_profile_h__
#define modern_skinned_profile_h__

//#include "modern_commonheaders.h"
//#include "modern_commonprototypes.h"

#include <map>
#include <string>
//#include <atlstr.h>

class MString
{
private:
	TCHAR * _buffer;
public:
	MString() : _buffer(NULL) {};
	MString(const TCHAR * str) { _buffer = str ? _tcsdup(str) : NULL; }
	MString(const MString& str)      { _buffer = str._buffer ? _tcsdup(str._buffer) : NULL; }
	MString& operator=(const MString& str)
	{
		if (_buffer) free(_buffer);
		_buffer = str._buffer ? _tcsdup(str._buffer) : NULL;
	}
	TCHAR* operator()(const MString& str) { return _buffer; }
	~MString()
	{
		if (_buffer)
			free(_buffer);
		_buffer = NULL;
	}

#ifdef _UNICODE
	MString(const char * str)
	{
		if (!str)
			_buffer = NULL;
		else
		{
			int cbLen = MultiByteToWideChar(0, 0, str, -1, NULL, 0);
			wchar_t* _buffer = (wchar_t*)malloc(sizeof(wchar_t)*(cbLen + 1));
			if (_buffer == NULL) return;
			MultiByteToWideChar(0, 0, str, -1, _buffer, cbLen);
			_buffer[cbLen] = 0;
		}
	}
#endif

};

class ValueVariant
{
public:
	ValueVariant() : _type(VVT_EMPTY) {};
	ValueVariant(BYTE bValue) : _type(VVT_BYTE), _bValue(bValue)    {};
	ValueVariant(WORD wValue) : _type(VVT_WORD), _wValue(wValue)    {};
	ValueVariant(DWORD dwValue) : _type(VVT_DWORD), _dwValue(dwValue)  {};
	ValueVariant(const MString& strValue) : _type(VVT_STRING), _strValue(strValue) {};
	ValueVariant(const char * szValue) : _type(VVT_STRING), _strValue(szValue) {};
#ifdef _UNICODE
	ValueVariant(const wchar_t * szValue) : _type(VVT_STRING), _strValue(szValue) {};
#endif

	BYTE GetByte()
	{
		switch (_type)
		{
		case VVT_BYTE:
			return (BYTE)_bValue;
		case VVT_WORD:
		case VVT_DWORD:
			DebugBreak();
			return (BYTE)_bValue;
		default:
			DebugBreak();
		}
		return 0;
	}

	WORD GetWord()
	{
		switch (_type)
		{
		case VVT_WORD:
			return (WORD)_wValue;

		case VVT_BYTE:
		case VVT_DWORD:
			DebugBreak();
			return (WORD)_wValue;
		default:
			DebugBreak();
		}
		return 0;
	}

	DWORD GetDword()
	{
		switch (_type)
		{
		case VVT_DWORD:
			return (DWORD)_dwValue;

		case VVT_BYTE:
		case VVT_WORD:
			DebugBreak();
			return (DWORD)_dwValue;
		default:
			DebugBreak();
		}
		return 0;
	}
	MString GetString()
	{
		switch (_type)
		{
		case VVT_STRING:
			return _strValue;

		default:
			DebugBreak();
		}
		return "";
	}
	const MString& GetStringStatic()
	{
		switch (_type)
		{
		case VVT_STRING:
			return _strValue;

		default:
			DebugBreak();
		}
		return _strValue;
	}
	bool IsEmpty() { return _type == VVT_EMPTY; }

private:
	enum
	{
		VVT_EMPTY = 0,
		VVT_BYTE,
		VVT_WORD,
		VVT_DWORD,
		VVT_STRING
	};
	int _type;
	union
	{
		BYTE  _bValue;
		WORD  _wValue;
		DWORD _dwValue;
	};
	MString   _strValue;

};

// this is single tone class to represent some profile settings to be skinned 
class CSkinnedProfile
{
private:
	static CSkinnedProfile _me;
	CSkinnedProfile();
	~CSkinnedProfile();

private:
	// Skinned profile holded attributes
	MString _strSkinFilename;
	bool	_bLoadedFonts;
	bool	_bLoadedOthers;

	typedef std::map<HashStringKeyNoCase, ValueVariant> ValueList_t;
	typedef std::map<HashStringKeyNoCase, ValueList_t> KeyList_t;

	KeyList_t SkinnedProfile;

	ValueVariant* _GetValue(const char * szSection, const char * szKey);

	mir_cs _Lock; // critical section to matable skinned profile access

public:
	static CSkinnedProfile* SkinProfile() { return &_me; }

	HRESULT Init();
	HRESULT Clear();

	static BYTE  SpiGetSkinByte(MCONTACT hContact, const char * szSection, const char * szKey, const BYTE defValue);
	static WORD  SpiGetSkinWord(MCONTACT hContact, const char * szSection, const char * szKey, const WORD defValue);
	static DWORD SpiGetSkinDword(MCONTACT hContact, const char * szSection, const char * szKey, const DWORD defValue);
	static BOOL  SpiCheckSkinned(MCONTACT hContact, const char * szSection, const char * szKey);

};



#endif // modern_skinned_profile_h__