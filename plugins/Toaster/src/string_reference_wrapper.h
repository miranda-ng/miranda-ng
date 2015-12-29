#pragma once

class StringReferenceWrapper
{
public:
	StringReferenceWrapper(_In_reads_(length) PCWSTR stringRef, _In_ UINT32 length) throw()
	{	WindowsCreateStringReference(stringRef, length, &_header, &_hstring);
	}

	template <size_t N>
	StringReferenceWrapper(_In_reads_(N) wchar_t const (&stringRef)[N]) throw()
	{	WindowsCreateStringReference(stringRef, (UINT32)(N - 1), &_header, &_hstring);
	}

	template <size_t _>
	StringReferenceWrapper(_In_reads_(_) wchar_t(&stringRef)[_]) throw()
	{	WindowsCreateStringReference(stringRef, mir_wstrlen(stringRef), &_header, &_hstring);
	}

	~StringReferenceWrapper()
	{	WindowsDeleteString(_hstring);
	}

	HSTRING Get() const throw()
	{	return _hstring;
	}

private:
	HSTRING _hstring;
	HSTRING_HEADER _header;
};