#if !defined(HISTORYSTATS_GUARD__FORMAT_H)
#define HISTORYSTATS_GUARD__FORMAT_H

/*
 * ext::basic_format and helper routines
 */

namespace ext
{
	template<typename T_>
	class basic_format
	{
	public:
		typedef typename T_ char_type;
		typedef typename std::basic_string<T_> str_type;
		typedef typename std::basic_string<T_>::size_type size_type;
		typedef typename ext::basic_strfunc<T_> _Func;

	private:
		str_type m_Str;
		char_type m_Sep;
		size_type m_NextPos;

	public:
		explicit basic_format(const str_type& str, char_type sep = '|')
			: m_Str(str), m_Sep(sep), m_NextPos(0)
		{
		}

		const str_type& str() const
		{
			return m_Str;
		}

		basic_format& operator %(const char_type* szValue)
		{
			str_type::size_type pos = m_Str.find(m_Sep, m_NextPos);

			if (pos != str_type::npos)
			{
				size_type len = _Func::len(szValue);

				m_Str.replace(pos, 1, szValue, len);
				m_NextPos = pos + len;
			}

			return *this;
		}

		basic_format& operator %(const str_type& strValue)
		{
			return (*this % strValue.c_str());
		}

		basic_format& operator %(int nValue)
		{
			std::basic_ostringstream<char_type> oss;

			oss << nValue;

			return (*this % oss.str().c_str());
		}
	};

	template<typename T_>
	inline const std::basic_string<T_>& str(const basic_format<T_>& format)
	{
		return format.str();
	}
};

template<typename T_>
inline std::basic_ostream<T_>& operator <<(std::basic_ostream<T_>& os, const ext::basic_format<T_>& format)
{
	os << format.str();

	return os;
}

/*
 * ext::basic_kformat and helper routines
 */

namespace ext
{
	template<typename T_>
	class basic_kformat
	{
	public:
		typedef typename T_ char_type;
		typedef typename std::basic_string<T_> str_type;
		typedef typename std::basic_string<T_>::size_type size_type;
		typedef typename ext::basic_strfunc<T_> _Func;

	private:
		str_type m_Str;
		str_type m_Mask;
		str_type m_CurKey;

	public:
		explicit basic_kformat(const str_type& str)
			: m_Str(str), m_Mask(str.length(), '_')
		{
		}

		const str_type& str() const
		{
			return m_Str;
		}

		basic_kformat& operator %(const char_type* szKey)
		{
			m_CurKey = szKey;

			return *this;
		}

		basic_kformat& operator *(const char_type* szValue)
		{
			if (!m_CurKey.empty())
			{
				ext::string::size_type pos = 0;
				ext::string::size_type key_len = m_CurKey.length();
				ext::string::size_type value_len = _Func::len(szValue);

				while ((pos = m_Str.find(m_CurKey, pos)) != ext::string::npos)
				{
					if (m_Mask.substr(pos, key_len).find('X') == ext::string::npos)
					{
						// replace since we didn't replace here before
						m_Str.replace(pos, key_len, szValue, value_len);
						m_Mask.replace(pos, key_len, value_len, 'X');
						pos += value_len;
					}
					else
					{
						// skip since we already replaced in this area
						pos += key_len;
					}
				}
			}

			return *this;
		}

		basic_kformat& operator *(const str_type& strValue)
		{
			return (*this * strValue.c_str());
		}

		basic_kformat& operator *(int nValue)
		{
			std::basic_ostringstream<char_type> oss;

			oss << nValue;

			return (*this * oss.str().c_str());
		}
	};

	template<typename T_>
	inline const std::basic_string<T_>& str(const basic_kformat<T_>& format)
	{
		return format.str();
	}
}

template<typename T_>
inline std::basic_ostream<T_>& operator <<(std::basic_ostream<T_>& os, const ext::basic_kformat<T_>& format)
{
	os << format.str();

	return os;
}

#endif // HISTORYSTATS_GUARD__FORMAT_H
