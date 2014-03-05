#if !defined(HISTORYSTATS_GUARD_UTF8BUFFER_H)
#define HISTORYSTATS_GUARD_UTF8BUFFER_H

#include <streambuf>
#include <string>

class UTF8Buffer
	: public std::basic_streambuf<TCHAR, std::char_traits<TCHAR> >
	, private pattern::NotCopyable<UTF8Buffer>
{
private:
	typedef std::char_traits<TCHAR> _Tr;

private:
	int m_BufSize;
	TCHAR* m_pBuf;
	ext::a::ostream& m_Stream;
	char* m_pUTF8Buf;
	WCHAR* m_pBufW;

#if !defined(_UNICODE)
	WCHAR m_CharMap[256];
#endif // _UNICODE

private:
	void dump(const TCHAR* pBegin, const TCHAR* pEnd)
	{
		size_t len = pEnd - pBegin;

#if !defined(_UNICODE)
		WCHAR* pW = m_pBufW;
		char* pA = m_pBuf;

		while (pA < pEnd)
		{
			*pW++ = m_CharMap[static_cast<unsigned char>(*pA++)];
		}
#endif // _UNICODE
		size_t utf8_len = utils::rawUTF8Encode(m_pBufW, len, m_pUTF8Buf);

		m_Stream.write(m_pUTF8Buf, utf8_len);
	}

protected:
	virtual int_type overflow(int_type _C = UTF8Buffer::_Tr::eof())
	{
		if (_Tr::eq_int_type(_Tr::eof(), _C))
		{
			return _Tr::not_eof(_C);
		}
		else if (pptr() != 0 && pptr() < epptr())
		{
			*pptr() = _Tr::to_char_type(_C);
			pbump(1);

			return _C;
		}
		else
		{
			dump(pbase(), pptr());

			setp(m_pBuf, m_pBuf + m_BufSize);

			*pptr() = _Tr::to_char_type(_C);
			pbump(1);

			return _C;
		}
	}

	virtual int sync()
	{
		dump(pbase(), pptr());

		setp(m_pBuf, m_pBuf + m_BufSize);

		return 0;
	}

public:
	explicit UTF8Buffer(ext::a::ostream& outStream, int bufferSize = 4096)
		: m_BufSize(bufferSize), m_Stream(outStream)
	{
		m_pBuf = new TCHAR[m_BufSize];
		m_pUTF8Buf = new char[3 * m_BufSize];

#if defined(_UNICODE)
		m_pBufW = m_pBuf;
#else // _UNICODE
		m_pBufW = new WCHAR[m_BufSize];
#endif // _UNICODE

#if !defined(_UNICODE)
		char ANSIChars[256];

		array_each_(i, ANSIChars)
		{
			ANSIChars[i] = static_cast<char>(i);
		}

		m_CharMap[0] = ' ';
		MultiByteToWideChar(CP_ACP, 0, ANSIChars + 1, 255, m_CharMap + 1, 255);
#endif // _UNICODE

		setp(m_pBuf, m_pBuf + m_BufSize);
	}

	virtual ~UTF8Buffer()
	{
		delete[] m_pBuf;
		delete[] m_pUTF8Buf;

#if !defined(_UNICODE)
		delete[] m_pBufW;
#endif // _UNICODE
	}
};

#endif // HISTORYSTATS_GUARD_UTF8BUFFER_H
