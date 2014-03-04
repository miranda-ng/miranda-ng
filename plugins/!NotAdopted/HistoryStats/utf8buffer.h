#if !defined(HISTORYSTATS_GUARD_UTF8BUFFER_H)
#define HISTORYSTATS_GUARD_UTF8BUFFER_H

#include <streambuf>
#include <string>

class UTF8Buffer
	: public std::basic_streambuf<mu_text, std::char_traits<mu_text> >
	, private pattern::NotCopyable<UTF8Buffer>
{
private:
	typedef std::char_traits<mu_text> _Tr;

private:
	int m_BufSize;
	mu_text* m_pBuf;
	ext::a::ostream& m_Stream;
	mu_ansi* m_pUTF8Buf;
	mu_wide* m_pBufW;

#if !defined(MU_WIDE)
	mu_wide m_CharMap[256];
#endif // MU_WIDE

private:
	void dump(const mu_text* pBegin, const mu_text* pEnd)
	{
		size_t len = pEnd - pBegin;

#if !defined(MU_WIDE)
		mu_wide* pW = m_pBufW;
		mu_ansi* pA = m_pBuf;

		while (pA < pEnd)
		{
			*pW++ = m_CharMap[static_cast<unsigned char>(*pA++)];
		}
#endif // MU_WIDE
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
		m_pBuf = new mu_text[m_BufSize];
		m_pUTF8Buf = new mu_ansi[3 * m_BufSize];

#if defined(MU_WIDE)
		m_pBufW = m_pBuf;
#else // MU_WIDE
		m_pBufW = new mu_wide[m_BufSize];
#endif // MU_WIDE

#if !defined(MU_WIDE)
		mu_ansi ANSIChars[256];

		array_each_(i, ANSIChars)
		{
			ANSIChars[i] = static_cast<mu_ansi>(i);
		}

		m_CharMap[0] = muC(' ');
		MultiByteToWideChar(CP_ACP, 0, ANSIChars + 1, 255, m_CharMap + 1, 255);
#endif // MU_WIDE

		setp(m_pBuf, m_pBuf + m_BufSize);
	}

	virtual ~UTF8Buffer()
	{
		delete[] m_pBuf;
		delete[] m_pUTF8Buf;

#if !defined(MU_WIDE)
		delete[] m_pBufW;
#endif // MU_WIDE
	}
};

#endif // HISTORYSTATS_GUARD_UTF8BUFFER_H
