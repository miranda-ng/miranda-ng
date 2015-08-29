#ifndef _STRING_WRAPPER_H_
#define _STRING_WRAPPER_H_

class StringReferenceWrapper
{
public:
	StringReferenceWrapper(_In_reads_(length) PCWSTR stringRef, _In_ UINT32 length) throw()
	{
		HRESULT hr = WindowsCreateStringReference(stringRef, length, &_header, &_hstring);

		if (FAILED(hr))
			RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
	}

	template <size_t N>
	StringReferenceWrapper(_In_reads_(N) wchar_t const (&stringRef)[N]) throw()
	{
		UINT32 length = N - 1;
		HRESULT hr = WindowsCreateStringReference(stringRef, length, &_header, &_hstring);

		if (FAILED(hr))
			RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
	}

	template <size_t _>
	StringReferenceWrapper(_In_reads_(_) wchar_t(&stringRef)[_]) throw()
	{
		size_t length = wcslen(stringRef);
		WindowsCreateStringReference(stringRef, length, &_header, &_hstring);
	}

	~StringReferenceWrapper()
	{
		WindowsDeleteString(_hstring);
	}

	HSTRING Get() const throw()
	{
		return _hstring;
	}

private:
	HSTRING _hstring;
	HSTRING_HEADER _header;
};

#endif //_STRING_WRAPPER_H_